// remote_view.cpp
// Build: link -lcivetweb -lturbojpeg -lwebp -lopus -lpthread
//
// Wire protocol (little endian), server -> browser:
//   [0][u16 w][u16 h]                                   init
//   [1][codec u8: 0=JPEG 1=WebP][u16 x][u16 y][u16 w][u16 h][payload]   tile
//   [2][u32 seq][opus packet]                           audio, 20 ms mono 48 kHz
// browser -> server:
//   [0x10][down u8][u16 x][u16 y]                       pointer
//   [0x11][1 = up, 255 = down]                          wheel
#include "remote_view.h"
#include "civetweb.h"
#include <turbojpeg.h>
#include <webp/encode.h>
#include <opus/opus.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace {
	using namespace std::chrono;

	RvConfig g_cfg;
	int W, H;
	std::atomic<bool> g_run{false}
	;
	mg_context* g_ctx = nullptr;
	std::thread g_sender, g_audio;

	// ---------- clients / broadcast ----------
	std::mutex g_cliMtx;
	std::vector<mg_connection*> g_clients;
	std::atomic<bool> g_full{true}
	;

	bool haveClients() { std::lock_guard<std::mutex> lk(g_cliMtx); return !g_clients.empty(); }

	void broadcast(const uint8_t* p, size_t n) {
		std::lock_guard<std::mutex> lk(g_cliMtx); // also serialises sender + audio writes
		for (auto* c : g_clients)
			mg_websocket_write(c, MG_WEBSOCKET_OPCODE_BINARY, (const char*)p, n);
	}

	// ---------- framebuffers ----------
	struct Rect {
		int x1 = INT_MAX, y1 = INT_MAX, x2 = -1, y2 = -1;
		bool empty() const { return x2 < x1 || y2 < y1; }
		void add(int ax1, int ay1, int ax2, int ay2) {
			x1 = std::min(x1, ax1); y1 = std::min(y1, ay1);
			x2 = std::max(x2, ax2); y2 = std::max(y2, ay2);
		}
		void clear() { *this = Rect { }; }
	};

	std::mutex g_fbMtx;
	std::vector<uint8_t> g_shadow, g_snap, g_sent; // XRGB8888, stride = W*4
	Rect g_dirty[3]; // 0 = above band, 1 = band (lossy), 2 = below band

	inline size_t off(int y, int x) { return ((size_t)y * W + x) * 4; }

	bool trim(Rect& r) {
		// shrink rect to rows that really changed
		size_t bytes = (size_t)(r.x2 - r.x1 + 1) * 4;
		auto changed = [&](int y) { return memcmp(&g_snap[off(y, r.x1)], &g_sent[off(y, r.x1)], bytes) != 0; };
		while (r.y1 <= r.y2 && !changed(r.y1)) r.y1++;
		while (r.y2 >= r.y1 && !changed(r.y2)) r.y2--;
		return r.y1 <= r.y2;
	}

	void put16(uint8_t* p, int v) { uint16_t u = (uint16_t)v; memcpy(p, &u, 2); }

	void sendTile(int codec, const Rect& r, const uint8_t* data, size_t n) {
		std::vector<uint8_t> m(10 + n);
		m[0] = 1; m[1] = (uint8_t)codec;
		put16(&m[2], r.x1); put16(&m[4], r.y1);
		put16(&m[6], r.x2 - r.x1 + 1); put16(&m[8], r.y2 - r.y1 + 1);
		memcpy(&m[10], data, n);
		broadcast(m.data(), m.size());
	}

	void encodeJpeg(tjhandle tj, const Rect& r) {
		unsigned char* out = nullptr; unsigned long sz = 0;
		int w = r.x2 - r.x1 + 1, h = r.y2 - r.y1 + 1;
		if (tjCompress2(tj,
			&g_snap[off(r.y1, r.x1)],
			w,
			W * 4,
			h,
			TJPF_BGRX,
			&out,
			&sz,
			TJSAMP_420,
			g_cfg.jpegQuality,
			TJFLAG_FASTDCT) == 0)
			sendTile(0, r, out, sz);
		tjFree(out);
	}

	void encodeWebpLossless(const Rect& r) {
		int w = r.x2 - r.x1 + 1, h = r.y2 - r.y1 + 1;
		WebPConfig cfg; WebPConfigInit(&cfg);
		cfg.lossless = 1; cfg.method = 1; cfg.quality = 20; // fast, still lossless
		WebPPicture pic; WebPPictureInit(&pic);
		pic.use_argb = 1; pic.width = w; pic.height = h;
		WebPMemoryWriter wr; WebPMemoryWriterInit(&wr);
		pic.writer = WebPMemoryWrite; pic.custom_ptr = &wr;
		if (WebPPictureImportBGRX(&pic, &g_snap[off(r.y1, r.x1)], W * 4) && WebPEncode(&cfg, &pic))
			sendTile(1, r, wr.mem, wr.size);
		WebPMemoryWriterClear(&wr);
		WebPPictureFree(&pic);
	}

	void senderLoop() {
		tjhandle tj = tjInitCompress();
		auto lastBand = steady_clock::now();
		const auto uiTick   = milliseconds(1000 / std::max(1, g_cfg.uiFps));
		const auto bandTick = milliseconds(1000 / std::max(1, g_cfg.bandFps));

		while (g_run) {
			std::this_thread::sleep_for(uiTick);
			if (!haveClients()) continue;

			bool bandDue = steady_clock::now() - lastBand >= bandTick;
			Rect r[3];
			{
				std::lock_guard<std::mutex> lk(g_fbMtx);
				if (g_full.exchange(false)) {
					// new client: resend everything
					std::fill(g_sent.begin(), g_sent.end(), 0);
					int zy[4] = { 0, g_cfg.bandY1, g_cfg.bandY2 + 1, H };
					for (int z = 0; z < 3; z++) { g_dirty[z].clear(); g_dirty[z].add(0, zy[z], W - 1, zy[z + 1] - 1); }
				}
				for (int z = 0; z < 3; z++) {
					if (z == 1 && !bandDue) continue;            // band is rate limited, dirt keeps accumulating
					r[z] = g_dirty[z]; g_dirty[z].clear();
					if (r[z].empty()) continue;
					for (int y = r[z].y1; y <= r[z].y2; y++)     // snapshot so LVGL can keep drawing
						memcpy(&g_snap[off(y, r[z].x1)], &g_shadow[off(y, r[z].x1)], (size_t)(r[z].x2 - r[z].x1 + 1) * 4);
				}
			}
			if (bandDue) lastBand = steady_clock::now();

			for (int z = 0; z < 3; z++) {
				if (r[z].empty() || !trim(r[z])) continue;
				if (z == 1) encodeJpeg(tj, r[z]);
				else encodeWebpLossless(r[z]);
				size_t bytes = (size_t)(r[z].x2 - r[z].x1 + 1) * 4;
				for (int y = r[z].y1; y <= r[z].y2; y++)
					memcpy(&g_sent[off(y, r[z].x1)], &g_snap[off(y, r[z].x1)], bytes);
			}
		}
		tjDestroy(tj);
	}

	// ---------- audio ----------
	struct Ring {
		// single producer / single consumer
		static constexpr size_t N = 1 << 16, M = N - 1;
		std::vector<float> b = std::vector<float>(N);
		std::atomic<size_t> w{0}, r{0}
		;
		size_t avail() const { return w.load(std::memory_order_acquire) - r.load(std::memory_order_relaxed); }
		void push(float v) {
			// drops when full
			size_t wi = w.load(std::memory_order_relaxed);
			if (wi - r.load(std::memory_order_acquire) >= N) return;
			b[wi & M] = v; w.store(wi + 1, std::memory_order_release);
		}
		void pop(float* p, size_t n) {
			size_t ri = r.load(std::memory_order_relaxed);
			for (size_t i = 0; i < n; i++) p[i] = b[(ri + i) & M];
			r.store(ri + n, std::memory_order_release);
		}
	} g_ring;

	void audioLoop() {
		constexpr int FRAME = 960; // 20 ms @ 48 kHz
		int err = 0;
		OpusEncoder* enc = opus_encoder_create(48000, 1, OPUS_APPLICATION_AUDIO, &err);
		opus_encoder_ctl(enc, OPUS_SET_BITRATE(32000));
		opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(0));
		float pcm[FRAME]; uint8_t pkt[5 + 400]; uint32_t seq = 0;

		while (g_run) {
			while (g_ring.avail() > 5 * FRAME) g_ring.pop(pcm, FRAME); // cap latency, drop stale audio
			if (g_ring.avail() < (size_t)FRAME) { std::this_thread::sleep_for(milliseconds(4)); continue; }
			g_ring.pop(pcm, FRAME);
			if (!haveClients()) continue;
			int n = opus_encode_float(enc, pcm, FRAME, pkt + 5, sizeof(pkt) - 5);
			if (n > 0) { pkt[0] = 2; memcpy(pkt + 1, &seq, 4); seq++; broadcast(pkt, 5 + n); }
		}
		opus_encoder_destroy(enc);
	}

	// ---------- input ----------
	struct Ev { bool down; int x, y; };
	std::mutex g_inMtx;
	std::deque<Ev> g_ev;
	std::atomic<int> g_wheel{0}
	;

	void pushInput(const uint8_t* d, size_t n) {
		if (d[0] == 0x10 && n >= 6) {
			uint16_t x, y; memcpy(&x, d + 2, 2); memcpy(&y, d + 4, 2);
			Ev e{d[1] != 0, std::min<int>(x, W - 1), std::min<int>(y, H - 1)}
			;
			std::lock_guard<std::mutex> lk(g_inMtx);
			if (!g_ev.empty() && g_ev.back().down == e.down) g_ev.back() = e; // coalesce moves, keep press/release edges
			else g_ev.push_back(e);
		}
		else if (d[0] == 0x11 && n >= 2) {
			g_wheel += (d[1] == 1) ? 1 : -1;
		}
	}

	void ptrRead(lv_indev_t*, lv_indev_data_t* d) {
		// runs on the LVGL thread
		static lv_point_t last{0, 0}
		; static bool down = false;
		std::lock_guard<std::mutex> lk(g_inMtx);
		if (!g_ev.empty()) { Ev e = g_ev.front(); g_ev.pop_front(); last = { e.x, e.y }; down = e.down; }
		d->point = last;
		d->state = down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
		d->continue_reading = !g_ev.empty();
	}

	// ---------- civetweb ----------
	void logf(const mg_connection* c, const char* what) {
		const mg_request_info* ri = mg_get_request_info(c);
		char ts[32]; time_t t = time(nullptr); struct tm tmv; localtime_r(&t, &tmv);
		strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tmv);
		const char* ua = mg_get_header(c, "User-Agent");
		fprintf(stderr, "[remote_view] %s %s from %s (%s)\n", ts, what, ri->remote_addr, ua ? ua : "unknown agent");
	}

	// Logs, then returns 0 so civetweb still serves the file itself.
	int indexHandler(mg_connection* c, void*) {
		logf(c, "index page requested");
		return 0;
	}

	int  wsConnect(const mg_connection*, void*) { return 0; }
	void wsReady(mg_connection* c, void*) {
		logf(c, "websocket opened");
		uint8_t m[5] = { 0 }; put16(m + 1, W); put16(m + 3, H);
		mg_websocket_write(c, MG_WEBSOCKET_OPCODE_BINARY, (const char*)m, 5);
		{ std::lock_guard<std::mutex> lk(g_cliMtx); g_clients.push_back(c); }
		g_full = true;
	}
	int wsData(mg_connection*, int bits, char* d, size_t n, void*) {
		if ((bits & 0xF) == MG_WEBSOCKET_OPCODE_BINARY && n > 0) pushInput((const uint8_t*)d, n);
		return 1;
	}
	void wsClose(const mg_connection* c, void*) {
		logf(c, "websocket closed");
		{
			std::lock_guard<std::mutex> lk(g_cliMtx);
			g_clients.erase(std::remove(g_clients.begin(), g_clients.end(), c), g_clients.end());
		}
		{ std::lock_guard<std::mutex> lk(g_inMtx); g_ev.push_back({ false, 0, 0 }); }   // no stuck button
		if (g_cfg.onDisconnect) g_cfg.onDisconnect(); // failsafe: unkey PTT
	}
} // namespace

// ---------- public API ----------
void rv_flush_tap(const lv_area_t* a, const uint8_t* px) {
	// DIRECT mode: px is the base of the full frame buffer, stride assumed W*4.
	std::lock_guard<std::mutex> lk(g_fbMtx);
	size_t bytes = (size_t)(a->x2 - a->x1 + 1) * 4;
	for (int y = a->y1; y <= a->y2; y++) memcpy(&g_shadow[off(y, a->x1)], px + off(y, a->x1), bytes);
	int zy[4] = { 0, g_cfg.bandY1, g_cfg.bandY2 + 1, H };
	for (int z = 0; z < 3; z++) {
		int y1 = std::max<int>(a->y1, zy[z]), y2 = std::min<int>(a->y2, zy[z + 1] - 1);
		if (y1 <= y2) g_dirty[z].add(a->x1, y1, a->x2, y2);
	}
}

void rv_audio_push(std::span<const float> in, unsigned ch) {
	if (ch == 0) return;
	const size_t frames = in.size() / ch; // ignores a trailing partial frame
	for (size_t i = 0; i < frames; i++) {
		float s = 0;
		for (unsigned c = 0; c < ch; c++) s += in[i * ch + c];
		g_ring.push(s / ch);
	}
}

void rv_audio_push(const float* in, unsigned frames, unsigned ch) {
	for (unsigned i = 0; i < frames; i++) {
		float s = 0; for (unsigned c = 0; c < ch; c++) s += in[i * ch + c];
		g_ring.push(s / ch);
	}
}

void rv_start(const RvConfig& cfg, lv_display_t*) {
	g_cfg = cfg; W = cfg.w; H = cfg.h;
	size_t sz = (size_t)W * H * 4;
	g_shadow.assign(sz, 0); g_snap.assign(sz, 0); g_sent.assign(sz, 0);

	lv_indev_t* in = lv_indev_create();
	lv_indev_set_type(in, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(in, ptrRead);

	lv_timer_create([](lv_timer_t*) {
			int s = g_wheel.exchange(0);
			if (s && g_cfg.onWheel) g_cfg.onWheel(s);
		},
		20,
		nullptr);

	const char* opts[] = { 
		"document_root",
		g_cfg.docroot,
		"listening_ports",
		g_cfg.ports,
		"ssl_certificate",
		g_cfg.cert,
		"num_threads",
		"8",
		nullptr 
	};
	mg_init_library(MG_FEATURES_DEFAULT); // loads OpenSSL when civetweb is built with it
	if (!mg_check_feature(MG_FEATURES_SSL))
		fprintf(stderr, "[remote_view] WARNING: civetweb was built without SSL support, \"%s\" will fail\n", g_cfg.ports);

	mg_callbacks cb {}
	;
	cb.log_message = [](const mg_connection*, const char* msg) {
		fprintf(stderr, "[civetweb] %s\n", msg); return 1;
	};
	g_ctx = mg_start(&cb, nullptr, opts);
	if (!g_ctx) {
		fprintf(stderr,
			"[remote_view] mg_start FAILED (ports=%s cert=%s docroot=%s). "
		                "See [civetweb] lines above for the reason.\n",
			g_cfg.ports,
			g_cfg.cert,
			g_cfg.docroot);
		return;
	}
	fprintf(stderr, "[remote_view] listening on %s\n", g_cfg.ports);
	mg_set_request_handler(g_ctx, "/$", indexHandler, nullptr); // "$" = exact match
	mg_set_request_handler(g_ctx, "/index.html$", indexHandler, nullptr);
	mg_set_websocket_handler(g_ctx, "/ws", wsConnect, wsReady, wsData, wsClose, nullptr);

	g_run = true;
	g_sender = std::thread(senderLoop);
	g_audio  = std::thread(audioLoop);
}

void rv_stop() {
	g_run = false;
	if (g_sender.joinable()) g_sender.join();
	if (g_audio.joinable()) g_audio.join();
	if (g_ctx) mg_stop(g_ctx);
}