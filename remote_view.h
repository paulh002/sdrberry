// remote_view.h - mirror an LVGL 9.4 display + RtAudio output to a browser via civetweb WebSocket
#pragma once
#include <functional>
#include <lvgl.h>
#include <span>

struct RvConfig {
    int w = 1280, h = 720;          // logical (landscape) LVGL resolution
    int bandY1 = 360, bandY2 = 670; // spectrum + waterfall rows (lossy JPEG zone), inclusive
    int jpegQuality = 60;           // 50-70 is fine for noisy spectrum/waterfall
    int bandFps = 15;               // update rate of the lossy band
    int uiFps = 30;                 // update rate of everything else
    const char* docroot = "/opt/sdr/www";   // index.html, client.js, worklet.js
    const char* cert    = "/etc/sdr/cert.pem";
    const char* ports   = "8443s";
    std::function<void(int)> onWheel;       // +N / -N steps, called on the LVGL thread (tune here)
    std::function<void()>    onDisconnect;  // called from a civetweb thread: release PTT, must be thread-safe
};

// Call after lv_init() and after the display exists. Creates the pointer indev,
// the wheel timer, the civetweb server and the sender/audio threads.
void rv_start(const RvConfig& cfg, lv_display_t* disp);
void rv_stop();

// Call at the start of your display flush_cb (before lv_display_flush_ready).
// Display must use LV_COLOR_FORMAT_XRGB8888 and LV_DISPLAY_RENDER_MODE_DIRECT.
void rv_flush_tap(const lv_area_t* area, const uint8_t* px_map);

// Call from the RtAudio callback. Interleaved float32 at 48000 Hz. Real-time safe (no alloc, no locks).
void rv_audio_push(const float* interleaved, unsigned frames, unsigned channels);
void rv_audio_push(std::span<const float> in, unsigned ch);

