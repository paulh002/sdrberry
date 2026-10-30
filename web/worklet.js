// worklet.js - mono PCM jitter buffer. Receives Float32Array via port.postMessage.
class PCM extends AudioWorkletProcessor {
  constructor() {
    super();
    this.buf = new Float32Array(48000);   // 1 s ring
    this.r = 0; this.w = 0; this.n = 0;
    this.playing = false;
    this.START  = 4800;    // start after 100 ms buffered
    this.TARGET = 6000;    // trim back to 125 ms when too full
    this.MAX    = 12000;   // 250 ms: beyond this drop old samples (clock drift / bursts)
    this.port.onmessage = (e) => this.push(e.data);
  }
  push(f) {
    const len = this.buf.length;
    for (let i = 0; i < f.length; i++) { this.buf[this.w] = f[i]; this.w = (this.w + 1) % len; }
    this.n += f.length;
    if (this.n > this.MAX) {
      const drop = this.n - this.TARGET;
      this.r = (this.r + drop) % len; this.n -= drop;
    }
  }
  process(_inputs, outputs) {
    const o = outputs[0][0], len = this.buf.length;
    if (!this.playing && this.n >= this.START) this.playing = true;
    if (this.playing && this.n < o.length) this.playing = false;   // underrun: rebuffer
    if (!this.playing) { o.fill(0); return true; }
    for (let i = 0; i < o.length; i++) { o[i] = this.buf[this.r]; this.r = (this.r + 1) % len; }
    this.n -= o.length;
    return true;
  }
}
registerProcessor('pcm', PCM);
