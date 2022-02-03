
//
// An FT8 receiver in C++.
//
// Many ideas and protocol details borrowed from Franke
// and Taylor's WSJT-X code.
//
// Robert Morris, AB1HL
//

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex>
#include <fftw3.h>
#include <vector>
#include <algorithm>
#include <complex>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <functional>
#include <map>
#include "util.h"
#include "fft.h"
#include "ft8.h"

// 1920-point FFT at 12000 samples/second
// 6.25 Hz spacing, 0.16 seconds/symbol
// encode chain:
//   77 bits
//   append 14 bits CRC (for 91 bits)
//   LDPC(174,91) yields 174 bits
//   that's 58 3-bit FSK-8 symbols
//   gray code each 3 bits
//   insert three 7-symbol Costas sync arrays
//     at symbol #s 0, 36, 72 of final signal
//   thus: 79 FSK-8 symbols
// total transmission time is 12.64 seconds

// tunable parameters
int nthreads = 8; // number of parallel threads, for multi-core
int npasses_one = 3;  // number of spectral subtraction passes
int npasses_two = 3;  // number of spectral subtraction passes
int ldpc_iters = 25; // how hard LDPC decoding should work
int snr_win = 7; // averaging window, in symbols, for SNR conversion
int snr_how = 3; // technique to measure "N" for SNR. 0 means median of the 8 tones.
double shoulder200 = 10; // for 200 sps bandpass filter
double shoulder200_extra = 0.0; // for bandpass filter
double second_hz_win = 3.5; // +/- hz
int second_hz_n = 8; // divide total window into this many pieces
double second_off_win = 0.5; // +/- search window in symbol-times
int second_off_n = 10;
int third_hz_n = 3;
double third_hz_win = 0.25;
int third_off_n = 4;
double third_off_win = 0.075;
double log_tail = 0.1;
double log_rate = 8.0;
int problt_how_noise = 0;
int problt_how_sig = 0;
int use_apriori = 1;
int use_hints = 2; // 1 means use all hints, 2 means just CQ hints
int win_type = 1;
int osd_depth = 0; // 6; // don't increase beyond 6, produces too much garbage
int osd_ldpc_thresh = 70; // demand this many correct LDPC parity bits before OSD
int ncoarse = 1; // number of offsets per hz produced by coarse()
int ncoarse_blocks = 1;
double tminus = 2.2; // start looking at 0.5 - tminus seconds
double tplus = 2.4;
int coarse_off_n = 4;
int coarse_hz_n = 4;
double already_hz = 27;
double overlap = 20;
int overlap_edges = 0;
double nyquist = 0.925;
int oddrate = 1;
double pass0_frac = 1.0;
int reduce_how = 2;
double go_extra = 3.5;
int do_reduce = 1;
int pass_threshold = 1;
int strength_how = 4;
int known_strength_how = 7;
int coarse_strength_how = 6;
double reduce_shoulder = -1;
double reduce_factor = 0.25;
double reduce_extra = 0;
double coarse_all = -1;
int second_count = 3;
int soft_phase_win = 2;
double subtract_ramp = 0.11;
extern int fftw_type; // fft.cc. MEASURE=0, ESTIMATE=64, PATIENT=32
int soft_ones = 2;
int soft_pairs = 1;
int soft_triples = 1;
int do_second = 1;
int do_fine_hz = 1;
int do_fine_off = 1;
int do_third = 2;
double fine_thresh = 0.19;
int fine_max_off = 2;
int fine_max_tone = 4;
int known_sparse = 1;
double c_soft_weight = 7;
int c_soft_win = 2;
int bayes_how = 1;

//
// return a Hamming window of length n.
//
std::vector<double>
hamming(int n)
{
  std::vector<double> h(n);
  for(int k = 0; k < n; k++){
    h[k] = 0.54 - 0.46 * cos(2 * M_PI * k / (n - 1.0));
  }
  return h;
}

//
// blackman window
//
std::vector<double>
blackman(int n)
{
  std::vector<double> h(n);
  for(int k = 0; k < n; k++){
    h[k] = 0.42 - 0.5 * cos(2 * M_PI * k / n) + 0.08*cos(4 * M_PI * k / n);
  }
  return h;
}

//
// symmetric blackman window
//
std::vector<double>
sym_blackman(int n)
{
  std::vector<double> h(n);
  for(int k = 0; k < (n/2)+1; k++){
    h[k] = 0.42 - 0.5 * cos(2 * M_PI * k / n) + 0.08*cos(4 * M_PI * k / n);
  }
  for(int k = n-1; k >= (n/2)+1; --k){
    h[k] = h[(n-1)-k];
  }
  return h;
}

//
// blackman-harris window
//
std::vector<double>
blackmanharris(int n)
{
  double a0 = 0.35875;
  double a1 = 0.48829;
  double a2 = 0.14128;
  double a3 = 0.01168;
  std::vector<double> h(n);
  for(int k = 0; k < n; k++){
    // symmetric
    h[k] =
      a0 
      - a1 * cos(2 * M_PI * k / (n-1))
      + a2 * cos(4 * M_PI * k / (n-1))
      - a3 * cos(6 * M_PI * k / (n-1));
    // periodic
    //h[k] =
    //  a0 
    //  - a1 * cos(2 * M_PI * k / n)
    //  + a2 * cos(4 * M_PI * k / n)
    //  - a3 * cos(6 * M_PI * k / n);
  }
  return h;
}

//
// check the FT8 CRC-14
//
int
check_crc(const int a91[91])
{
  int aa[91];
  int non_zero = 0;
  for(int i = 0; i < 91; i++){
    if(i < 77){
      aa[i] = a91[i];
    } else {
      aa[i] = 0;
    }
    if(aa[i])
      non_zero++;
  }
  int out1[14];

  extern void ft8_crc(int msg1[], int msglen, int out[14]);

  // don't bother with all-zero messages.
  if(non_zero == 0)
    return 0;
  
  // why 82? why not 77?
  ft8_crc(aa, 82, out1);

  for(int i = 0; i < 14; i++){
    if(out1[i] != a91[91-14+i]){
      return 0;
    }
  }
  return 1;
}

//
// manage statistics for soft decoding, to help
// decide how likely each symbol is to be correct,
// to drive LDPC decoding.
//
// meaning of the how (problt_how) parameter:
// 0: gaussian
// 1: index into the actual distribution
// 2: do something complex for the tails.
// 3: index into the actual distribution plus gaussian for tails.
// 4: similar to 3.
// 5: laplace
//
class Stats {
public:
  std::vector<double> a_;
  double sum_;
  bool finalized_;
  double mean_; // cached
  double stddev_; // cached
  double b_; // cached
  int how_;
  
public:
  Stats(int how) : sum_(0), finalized_(false), how_(how) { }

  void add(double x) {
    a_.push_back(x);
    sum_ += x;
    finalized_ = false;
  }

  void finalize() {
    finalized_ = true;
    
    int n = a_.size();
    mean_ = sum_ / n;

    double var = 0;
    double bsum = 0;
    for(int i = 0; i < n; i++){
      double y = a_[i] - mean_;
      var += y * y;
      bsum += fabs(y);
    }
    var /= n;
    stddev_ = sqrt(var);
    b_ = bsum / n;

    // prepare for binary search to find where values lie
    // in the distribution.
    if(how_ != 0 && how_ != 5)
      std::sort(a_.begin(), a_.end());
  }

  double mean() {
    if(!finalized_)
      finalize();
    return mean_;
  }

  double stddev() {
    if(!finalized_)
      finalize();
    return stddev_;
  }

  // fraction of distribution that's less than x.
  // assumes normal distribution.
  // this is PHI(x), or the CDF at x,
  // or the integral from -infinity
  // to x of the PDF.
  double gaussian_problt(double x) {
    double SDs = (x - mean()) / stddev();
    double frac = 0.5 * (1.0 + erf(SDs / sqrt(2.0)));
    return frac;
  }

  // https://en.wikipedia.org/wiki/Laplace_distribution
  // m and b from page 116 of Mark Owen's Practical Signal Processing.
  double laplace_problt(double x) {
    double m = mean();

    double cdf;
    if(x < m){
      cdf = 0.5 * exp((x - m) / b_);
    } else {
      cdf = 1.0 - 0.5 * exp(-(x - m) / b_);
    }
    
    return cdf;
  }

  // look into the actual distribution.
  double problt(double x) {
    if(!finalized_)
      finalize();

    if(how_ == 0){
      return gaussian_problt(x);
    }

    if(how_ == 5){
      return laplace_problt(x);
    }

    // binary search.
    auto it = std::lower_bound(a_.begin(), a_.end(), x);
    int i = it - a_.begin();
    int n = a_.size();

    if(how_ == 1){
      // index into the distribution.
      // works poorly for values that are off the ends
      // of the distribution, since those are all
      // mapped to 0.0 or 1.0, regardless of magnitude.
      return i / (double) n;
    }

    if(how_ == 2){
      // use a kind of logistic regression for
      // values near the edges of the distribution.
      if(i < log_tail * n){
        double x0 = a_[(int)(log_tail * n)];
        double y = 1.0 / (1.0 + exp(-log_rate*(x-x0)));
        // y is 0..0.5
        y /= 5;
        return y;
      } else if(i > (1-log_tail) * n){
        double x0 = a_[(int)((1-log_tail) * n)];
        double y = 1.0 / (1.0 + exp(-log_rate*(x-x0)));
        // y is 0.5..1
        // we want (1-log_tail)..1
        y -= 0.5;
        y *= 2;
        y *= log_tail;
        y += (1-log_tail);
        return y;
      } else {
        return i / (double) n;
      }
    }

    if(how_ == 3){
      // gaussian for values near the edge of the distribution.
      if(i < log_tail * n){
        return gaussian_problt(x);
      } else if(i > (1-log_tail) * n){
        return gaussian_problt(x);
      } else {
        return i / (double) n;
      }
    }

    if(how_ == 4){
      // gaussian for values outside the distribution.
      if(x < a_[0] || x > a_.back()){
        return gaussian_problt(x);
      } else {
        return i / (double) n;
      }
    }

    assert(0);
  }
};

// a-priori probability of each of the 174 LDPC codeword
// bits being one. measured from reconstructed correct
// codewords, into ft8bits, then python bprob.py.
// from ft8-n4
double apriori174[] = {
  0.47, 0.32, 0.29, 0.37, 0.52, 0.36, 0.40, 0.42, 0.42, 0.53, 0.44,
  0.44, 0.39, 0.46, 0.39, 0.38, 0.42, 0.43, 0.45, 0.51, 0.42, 0.48,
  0.31, 0.45, 0.47, 0.53, 0.59, 0.41, 0.03, 0.50, 0.30, 0.26, 0.40,
  0.65, 0.34, 0.49, 0.46, 0.49, 0.69, 0.40, 0.45, 0.45, 0.60, 0.46,
  0.43, 0.49, 0.56, 0.45, 0.55, 0.51, 0.46, 0.37, 0.55, 0.52, 0.56,
  0.55, 0.50, 0.01, 0.19, 0.70, 0.88, 0.75, 0.75, 0.74, 0.73, 0.18,
  0.71, 0.35, 0.60, 0.58, 0.36, 0.60, 0.38, 0.50, 0.02, 0.01, 0.98,
  0.48, 0.49, 0.54, 0.50, 0.49, 0.53, 0.50, 0.49, 0.49, 0.51, 0.51,
  0.51, 0.47, 0.50, 0.53, 0.51, 0.46, 0.51, 0.51, 0.48, 0.51, 0.52,
  0.50, 0.52, 0.51, 0.50, 0.49, 0.53, 0.52, 0.50, 0.46, 0.47, 0.48,
  0.52, 0.50, 0.49, 0.51, 0.49, 0.49, 0.50, 0.50, 0.50, 0.50, 0.51,
  0.50, 0.49, 0.49, 0.55, 0.49, 0.51, 0.48, 0.55, 0.49, 0.48, 0.50,
  0.51, 0.50, 0.51, 0.50, 0.51, 0.53, 0.49, 0.54, 0.50, 0.48, 0.49,
  0.46, 0.51, 0.51, 0.52, 0.49, 0.51, 0.49, 0.51, 0.50, 0.49, 0.50,
  0.50, 0.47, 0.49, 0.52, 0.49, 0.51, 0.49, 0.48, 0.52, 0.48, 0.49,
  0.47, 0.50, 0.48, 0.50, 0.49, 0.51, 0.51, 0.51, 0.49,
};

class FT8 {
public:
  std::thread *th_;

  double min_hz_;
  double max_hz_;
  std::vector<double> samples_;  // input to each pass
  std::vector<double> nsamples_; // subtract from here

  int start_; // sample number of 0.5 seconds into samples[]
  int rate_;  // samples/second
  double deadline_; // start time + budget
  double final_deadline_; // keep going this long if no decodes
  std::vector<int> hints1_;
  std::vector<int> hints2_;
  int pass_;
  double down_hz_;

  static std::mutex cb_mu_;
  cb_t cb_; // call-back into Python

  std::mutex hack_mu_;
  int hack_size_;
  int hack_off_;
  int hack_len_;
  double hack_0_;
  double hack_1_;
  const double *hack_data_;
  std::vector<std::complex<double>> hack_bins_;
  std::vector<cdecode> prevdecs_;

  Plan *plan32_;

  FT8(const std::vector<double> &samples,
      double min_hz,
      double max_hz,
      int start, int rate,
      int hints1[], int hints2[], double deadline,
      double final_deadline, cb_t cb,
      std::vector<cdecode> prevdecs)
  {
    samples_ = samples;
    min_hz_ = min_hz;
    max_hz_ = max_hz;
    prevdecs_ = prevdecs;
    start_ = start;
    rate_ = rate;
    deadline_ = deadline;
    final_deadline_ = final_deadline;
    cb_ = cb;
    down_hz_ = 0;

    for(int i = 0; hints1[i]; i++){
      hints1_.push_back(hints1[i]);
    }
    for(int i = 0; hints2[i]; i++){
      hints2_.push_back(hints2[i]);
    }

    hack_size_ = -1;
    hack_data_ = 0;
    hack_off_ = -1;
    hack_len_ = -1;

    plan32_ = 0;
  }

  ~FT8() {
  }


// strength of costas block of signal with tone 0 at bi0,
// and symbol zero at si0.
double
one_coarse_strength(const ffts_t &bins, int bi0, int si0)
{
  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };

  assert(si0 >= 0 && si0+72+8 <= (int) bins.size());
  assert(bi0 >= 0 && bi0 + 8 <= (int) bins[0].size());

  double sig = 0.0;
  double noise = 0.0;

  if(coarse_all >= 0){
    for(int si = 0; si < 79; si++){
      double mx;
      int mxi = -1;
      double sum = 0;
      for(int i = 0; i < 8; i++){
        double x = std::abs(bins[si0+si][bi0+i]);
        sum += x;
        if(mxi < 0 || x > mx){
          mxi = i;
          mx = x;
        }
      }
      if(si >= 0 && si < 7){
        double x = std::abs(bins[si0+si][bi0+costas[si-0]]);
        sig += x;
        noise += sum - x;
      } else if(si >= 36 && si < 36+7){
        double x = std::abs(bins[si0+si][bi0+costas[si-36]]);
        sig += x;
        noise += sum - x;
      } else if(si >= 72 && si < 72+7){
        double x = std::abs(bins[si0+si][bi0+costas[si-72]]);
        sig += x;
        noise += sum - x;
      } else {
        sig += coarse_all * mx;
        noise += coarse_all * (sum - mx);
      }
    }
  } else {
    // coarse_all = -1
    // just costas symbols
    for(int si = 0; si < 7; si++){
      for(int bi = 0; bi < 8; bi++){
        double x = 0;
        x += std::abs(bins[si0+si][bi0+bi]);
        x += std::abs(bins[si0+36+si][bi0+bi]);
        x += std::abs(bins[si0+72+si][bi0+bi]);
        if(bi == costas[si]){
          sig += x;
        } else {
          noise += x;
        }
      }
    }
  }

  if(coarse_strength_how == 0){
     return sig - noise;
  } else if(coarse_strength_how == 1){
    return sig - noise/7;
  } else if(coarse_strength_how == 2){
    return sig / (noise/7);
  } else if(coarse_strength_how == 3){
    return sig / (sig + (noise/7));
  } else if(coarse_strength_how == 4){
    return sig;
  } else if(coarse_strength_how == 5){
    return sig / (sig + noise);
  } else if(coarse_strength_how == 6){
    // this is it.
    return sig / noise;
  } else {
    assert(0);
  }
}

// return symbol length in samples at the given rate.
// insist on integer symbol lengths so that we can
// use whole FFT bins.
int
blocksize(int rate)
{
  // FT8 symbol length is 1920 at 12000 samples/second.
  int xblock = 1920 / (12000.0 / rate);
  assert(xblock == (int) xblock);
  int block = xblock;
  return block;
}

class Strength {
public:
  double hz_;
  int off_;
  double strength_; // higher is better
};

//
// look for potential signals by searching FFT bins for Costas symbol
// blocks. returns a vector of candidate positions.
//
std::vector<Strength>
coarse(const ffts_t &bins, int si0, int si1)
{
  int block = blocksize(rate_);
  int nbins = bins[0].size();
  double bin_hz = rate_ / (double) block;
  int min_bin = min_hz_ / bin_hz;
  int max_bin = max_hz_ / bin_hz;

  std::vector<Strength> strengths;
  
  for(int bi = min_bin; bi < max_bin && bi+8 <= nbins; bi++){
    std::vector<Strength> sv;
    for(int si = si0; si < si1 && si + 79 < (int) bins.size(); si++){
      double s = one_coarse_strength(bins, bi, si);
      Strength st;
      st.strength_ = s;
      st.hz_ = bi * 6.25;
      st.off_ = si * block;
      sv.push_back(st);
    }
    if(sv.size() < 1)
      break;

    // save best ncoarse offsets, but require that they be separated
    // by at least one symbol time.

    std::sort(sv.begin(), sv.end(),
              [](const Strength &a, const Strength &b) -> bool
              { return a.strength_ > b.strength_; } );

    strengths.push_back(sv[0]);

    int nn = 1;
    for(int i = 1; nn < ncoarse && i < (int) sv.size(); i++){
      if(std::abs(sv[i].off_ - sv[0].off_) > ncoarse_blocks*block){
        strengths.push_back(sv[i]);
        nn++;
      }
    }
  }

  return strengths;
}

//
// reduce the sample rate from arate to brate.
// center hz0..hz1 in the new nyquist range.
// but first filter to that range.
// sets delta_hz to hz moved down.
//
std::vector<double>
reduce_rate(const std::vector<double> &a, double hz0, double hz1,
            int arate, int brate,
            double &delta_hz)
{
  assert(brate < arate);
  assert(hz1 - hz0 <= brate / 2);

  // the pass band is hz0..hz1
  // stop bands are 0..hz00 and hz11..nyquist.
  double hz00, hz11;

  hz0 = std::max(0.0, hz0 - reduce_extra);
  hz1 = std::min(arate / 2.0, hz1 + reduce_extra);

  if(reduce_shoulder > 0){
    hz00 = hz0 - reduce_shoulder;
    hz11 = hz1 + reduce_shoulder;
  } else {
    double mid = (hz0 + hz1) / 2;
    hz00 = mid - (brate * reduce_factor);
    hz00 = std::min(hz00, hz0);
    hz11 = mid + (brate * reduce_factor);
    hz11 = std::max(hz11, hz1);
  }

  int alen = a.size();
  std::vector<std::complex<double>> bins1 = one_fft(a, 0, alen,
                                                    "reduce_rate1", 0);
  int nbins1 = bins1.size();
  double bin_hz = arate / (double) alen;

  if(reduce_how == 2){
    // band-pass filter the FFT output.
    bins1 = fbandpass(bins1, bin_hz,
                      hz00,
                      hz0,
                      hz1,
                      hz11);
  }
  
  if(reduce_how == 3){
    for(int i = 0; i < nbins1; i++){
      if(i < (hz0 / bin_hz)){
        bins1[i] = 0;
      } else if(i > (hz1 / bin_hz)){
        bins1[i] = 0;
      }
    }
  }

  // shift down.
  int omid = ((hz0 + hz1) / 2) / bin_hz;
  int nmid = (brate / 4.0) / bin_hz;

  int delta = omid - nmid; // amount to move down
  assert(delta < nbins1);
  int blen = round(alen * (brate / (double) arate));
  std::vector<std::complex<double>> bbins(blen / 2 + 1);
  for(int i = 0; i < (int) bbins.size(); i++){
    if(delta > 0){
      bbins[i] = bins1[i + delta];
    } else {
      bbins[i] = bins1[i];
    }
  }

  // use ifft to reduce the rate.
  std::vector<double> vvv = one_ifft(bbins, "reduce_rate2");

  delta_hz = delta * bin_hz;

  return vvv;
}

void
go(int npasses)
{
  // cache to avoid cost of fftw planner mutex.
  plan32_ = get_plan(32, "cache32");
    
  if(0){
    fprintf(stderr, "go: %.0f .. %.0f, %.0f, rate=%d\n",
            min_hz_, max_hz_, max_hz_ - min_hz_, rate_);
  }

  // trim to make samples_ a good size for FFTW.
  int nice_sizes[] = { 18000, 18225, 36000, 36450,
    54000, 54675, 72000, 72900,
    144000, 145800, 216000, 218700,
    0 };
  int nice = -1;
  for(int i = 0; nice_sizes[i]; i++){
    int sz = nice_sizes[i];
    if(fabs(samples_.size() - sz) < 0.05 * samples_.size()){
      nice = sz;
      break;
    }
  }
  if(nice != -1){
    samples_.resize(nice);
  }

  assert(min_hz_ >= 0 && max_hz_ + 50 <= rate_/2);

  // can we reduce the sample rate?
  int nrate = -1;
  for(int xrate = 100; xrate < rate_; xrate += 100){
    if(xrate < rate_ && (oddrate || (rate_ % xrate) == 0)){
      if(((max_hz_ - min_hz_) + 50 + 2*go_extra) < nyquist * (xrate / 2)){
        nrate = xrate;
        break;
      }
    }
  }

  if(do_reduce && nrate > 0 && nrate < rate_ * 0.75){
    // filter and reduce the sample rate from rate_ to nrate.

    double t0 = now();
    int osize = samples_.size();
    
    double delta_hz; // how much it moved down
    samples_ = reduce_rate(samples_,
                           min_hz_-3.1-go_extra,
                           max_hz_+50-3.1+go_extra,
                           rate_, nrate, delta_hz);

    double t1 = now();
    if(t1 - t0 > 0.1){
      fprintf(stderr, "reduce oops, size %d -> %d, rate %d -> %d, took %.2f\n",
              osize,
              (int) samples_.size(),
              rate_,
              nrate,
              t1 - t0);
    }
    if(0){
      fprintf(stderr, "%.0f..%.0f, range %.0f, rate %d -> %d, delta hz %.0f, %.6f sec\n",
              min_hz_, max_hz_,
              max_hz_ - min_hz_,
              rate_, nrate, delta_hz, t1 - t0);
    }

    if(delta_hz > 0){
      down_hz_ = delta_hz; // to adjust hz for Python.
      min_hz_ -= down_hz_;
      max_hz_ -= down_hz_;
      for(int i = 0; i < (int) prevdecs_.size(); i++){
        prevdecs_[i].hz0 -= delta_hz;
        prevdecs_[i].hz1 -= delta_hz;
      }
    }
    assert(max_hz_ + 50 < nrate / 2);
    assert(min_hz_ >= 0);

    double ratio = nrate / (double) rate_;
    rate_ = nrate;
    start_ = round(start_ * ratio);
  }

  int block = blocksize(rate_);

  // start_ is the sample number of 0.5 seconds, the nominal start time.

  // make sure there's at least tplus*rate_ samples after the end.
  if(start_ + tplus*rate_ + 79 * block + block > samples_.size()){
    int need = start_ + tplus*rate_ + 79 * block - samples_.size();

    // round up to a whole second, to ease fft plan caching.
    if((need % rate_) != 0)
      need += rate_ - (need % rate_);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, samples_.size()-1);
    auto rnd = std::bind(distribution, generator);

    std::vector<double> v(need);
    for(int i = 0; i < need; i++){
      //v[i] = 0;
      v[i] = samples_[rnd()];
    }
    samples_.insert(samples_.end(), v.begin(), v.end());
  }
  
  int si0 = (start_ - tminus*rate_) / block;
  if(si0 < 0)
    si0 = 0;
  int si1 = (start_ + tplus*rate_) / block;

  // a copy from which to subtract.
  nsamples_ = samples_;

  int any = 0;
  for(int i = 0; i < (int) prevdecs_.size(); i++){
    auto d = prevdecs_[i];
    if(d.hz0 >= min_hz_ && d.hz0 <= max_hz_){
      // reconstruct correct 79 symbols from LDPC output.
      std::vector<int> re79 = recode(d.bits);

      // fine up hz/off again now that we have more samples
      double best_hz = (d.hz0 + d.hz1) / 2.0;
      double best_off = d.off; // seconds
      search_both_known(samples_, rate_, re79,
                        best_hz,
                        best_off,
                        best_hz, best_off);

      // subtract from nsamples_.
      subtract(re79, best_hz, best_hz, best_off);
      any += 1;
    }
  }
  if(any){
    samples_ = nsamples_;
  }

  for(pass_ = 0; pass_ < npasses; pass_++){
    double total_remaining = deadline_ - now();
    double remaining = total_remaining / (npasses - pass_);
    if(pass_ == 0){
      remaining *= pass0_frac;
    }
    double deadline = now() + remaining;
    
    int new_decodes = 0;
    samples_ = nsamples_;

    std::vector<Strength> order;

    //
    // search coarsely for Costas blocks.
    // in fractions of bins in off and hz.
    //

    // just do this once, re-use for every fractional fft_shift
    // and down_v7_f() to 200 sps.
    std::vector<std::complex<double>> bins = one_fft(samples_, 0, samples_.size(),
                                                     "go1", 0);

    for(int hz_frac_i = 0; hz_frac_i < coarse_hz_n; hz_frac_i++){
      // shift down by hz_frac
      double hz_frac = hz_frac_i * (6.25 / coarse_hz_n);
      std::vector<double> samples1;
      if(hz_frac_i == 0){
        samples1 = samples_;
      } else {
        samples1 = fft_shift_f(bins, rate_, hz_frac);
      }
      
      for(int off_frac_i = 0; off_frac_i < coarse_off_n; off_frac_i++){
        int off_frac = off_frac_i * (block / coarse_off_n);
        ffts_t bins = ffts(samples1, off_frac, block, "go2");
        std::vector<Strength> oo = coarse(bins, si0, si1);
        for(int i = 0; i < (int) oo.size(); i++){
          oo[i].hz_ += hz_frac;
          oo[i].off_ += off_frac;
        }
        order.insert(order.end(), oo.begin(), oo.end());
      }
    }

    //
    // sort strongest-first.
    //
    std::sort(order.begin(), order.end(),
              [](const Strength &a, const Strength &b) -> bool
              { return a.strength_ > b.strength_; } );
    
    char already[2000]; // XXX
    for(int i = 0; i < (int)(sizeof(already)/sizeof(already[0])); i++)
      already[i] = 0;
    
    for(int ii = 0; ii < (int) order.size(); ii++){
      double tt = now();
      if(ii > 0 &&
         tt > deadline &&
         (tt > deadline_ || new_decodes >= pass_threshold) &&
         (pass_ < npasses-1 || tt > final_deadline_)){
        break;
      }

      double hz = order[ii].hz_;
      if(already[(int)round(hz / already_hz)])
        continue;
      int off = order[ii].off_;
      int ret = one(bins, samples_.size(), hz, off);
      if(ret){
        if(ret == 2){
          new_decodes++;
        }
        already[(int)round(hz / already_hz)] = 1;
      }
    }
  }
}

//
// what's the strength of the Costas sync blocks of
// the signal starting at hz and off?
//
double
one_strength(const std::vector<double> &samples200, double hz, int off)
{
  int bin0 = round(hz / 6.25);

  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };
  int starts[] = { 0, 36, 72 };

  double sig = 0;
  double noise = 0;

  for(int which = 0; which < 3; which++){
    int start = starts[which];
    for(int si = 0; si < 7; si++){
      auto fft = one_fft(samples200, off+(si+start)*32, 32, "one_strength", plan32_);
      for(int bi = 0; bi < 8; bi++){
        double x = std::abs(fft[bin0+bi]);
        if(bi == costas[si]){
          sig += x;
        } else {
          noise += x;
        }
      }
    }
  }
  
  if(strength_how == 0){
     return sig - noise;
  } else if(strength_how == 1){
    return sig - noise/7;
  } else if(strength_how == 2){
    return sig / (noise/7);
  } else if(strength_how == 3){
    return sig / (sig + (noise/7));
  } else if(strength_how == 4){
    return sig;
  } else if(strength_how == 5){
    return sig / (sig + noise);
  } else if(strength_how == 6){
    return sig / noise;
  } else {
    assert(0);
  }
}

//
// given a complete known signal's symbols in syms,
// how strong is it? used to look for the best
// offset and frequency at which to subtract a
// decoded signal.
//
double
one_strength_known(const std::vector<double> &samples,
                   int rate,
                   const std::vector<int> &syms,
                   double hz, int off)
{
  int block = blocksize(rate);
  assert(syms.size() == 79);
  
  int bin0 = round(hz / 6.25);

  double sig = 0;
  double noise = 0;

  double sum7 = 0;
  std::complex<double> prev = 0;
  
  for(int si = 0; si < 79; si += known_sparse){
    auto fft = one_fft(samples, off+si*block, block, "one_strength_known", 0);
    if(known_strength_how == 7){
      std::complex<double> c = fft[bin0+syms[si]];
      if(si > 0){
        sum7 += std::abs(c - prev);
      }
      prev = c;
    } else {
      for(int bi = 0; bi < 8; bi++){
        double x = std::abs(fft[bin0+bi]);
        if(bi == syms[si]){
          sig += x;
        } else {
          noise += x;
        }
      }
    }
  }

  if(known_strength_how == 0){
     return sig - noise;
  } else if(known_strength_how == 1){
    return sig - noise/7;
  } else if(known_strength_how == 2){
    return sig / (noise/7);
  } else if(known_strength_how == 3){
    return sig / (sig + (noise/7));
  } else if(known_strength_how == 4){
    return sig;
  } else if(known_strength_how == 5){
    return sig / (sig + noise);
  } else if(known_strength_how == 6){
    return sig / noise;
  } else if(known_strength_how == 7){
    return -sum7;
  } else {
    assert(0);
  }
}

int
search_time_fine(const std::vector<double> &samples200,
                 int off0, int offN,
                 double hz,
                 int gran,
                 double &str)
{
  if(off0 < 0)
    off0 = 0;

  //
  // shift in frequency to put hz at 25.
  // only shift the samples we need, both for speed,
  // and try to always shift down the same number of samples
  // to make it easier to cache fftw plans.
  //
  int len = (offN - off0) + 79*32 + 32;
  if(off0 + len > (int) samples200.size()){
    // len = samples200.size() - off0;
    // don't provoke random-length FFTs.
    return -1;
  }
  std::vector<double> downsamples200 = shift200(samples200, off0, len, hz);

  int best_off = -1;
  double best_sum = 0.0;

  for(int g = 0; g <= (offN-off0) && g + 79*32 <= len; g += gran){
    double sum = one_strength(downsamples200, 25, g);
    if(sum > best_sum || best_off == -1){
      best_off = g;
      best_sum = sum;
    }
  }

  str = best_sum;
  assert(best_off >= 0);
  return off0 + best_off;
}

int
search_time_fine_known(const std::vector<std::complex<double>> &bins,
                       int rate,
                       const std::vector<int> &syms,
                       int off0, int offN,
                       double hz,
                       int gran,
                       double &str)
{
  if(off0 < 0)
    off0 = 0;

  // nearest FFT bin center.
  double hz0 = round(hz / 6.25) * 6.25;

  // move hz to hz0, so it is centered in a symbol-sized bin.
  std::vector<double> downsamples = fft_shift_f(bins, rate, hz - hz0);

  int best_off = -1;
  int block = blocksize(rate);
  double best_sum = 0.0;

  for(int g = off0 ; g <= offN; g += gran){
    if(g >= 0 && g + 79*block <= (int) downsamples.size()){
      double sum = one_strength_known(downsamples, rate, syms, hz0, g);
      if(sum > best_sum || best_off == -1){
        best_off = g;
        best_sum = sum;
      }
    }
  }

  if(best_off < 0)
    return -1;

  str = best_sum;
  return best_off;
}

//
// search for costas blocks in an MxN time/frequency grid.
// hz0 +/- hz_win in hz_inc increments. hz0 should be near 25.
// off0 +/- off_win in off_inc incremenents.
//
std::vector<Strength>
search_both(const std::vector<double> &samples200,
            double hz0, int hz_n, double hz_win,
            int off0, int off_n, int off_win)
{
  assert(hz0 >= 25 - 6.25/2 && hz0 <= 25 + 6.25/2);

  std::vector<Strength> strengths;
  
  double hz_inc = 2 * hz_win / hz_n;
  int off_inc = round(2 * off_win / (double) off_n);
  if(off_inc < 1)
    off_inc = 1;

  for(double hz = hz0 - hz_win; hz <= hz0 + hz_win + 0.01; hz += hz_inc){
    double str = 0;
    int off = search_time_fine(samples200, off0 - off_win, off0 + off_win, hz,
                               off_inc, str);
    if(off >= 0){
      Strength st;
      st.hz_ = hz;
      st.off_ = off;
      st.strength_ = str;
      strengths.push_back(st);
    }
  }

  return strengths;
}

void
search_both_known(const std::vector<double> &samples,
                  int rate,
                  const std::vector<int> &syms,
                  double hz0,
                  double off_secs0, // seconds
                  double &hz_out, double &off_out)
{
  assert(hz0 >= 0 && hz0+50 < rate / 2);

  int off0 = round(off_secs0 * (double) rate);

  int off_win = third_off_win * blocksize(rate_);
  if(off_win < 1)
    off_win = 1;
  int off_inc = trunc((2.0 * off_win) / (third_off_n - 1.0));
  if(off_inc < 1)
    off_inc = 1;
  
  int got_best = 0;
  double best_hz = 0;
  int best_off = 0;
  double best_strength = 0;

  std::vector<std::complex<double>> bins = one_fft(samples, 0, samples.size(), "stfk", 0);

  double hz_start, hz_inc, hz_end;
  if(third_hz_n > 1){
    hz_inc = (2.0 * third_hz_win) / (third_hz_n - 1.0);
    hz_start = hz0 - third_hz_win;
    hz_end = hz0 + third_hz_win;
  } else {
    hz_inc = 1;
    hz_start = hz0;
    hz_end = hz0;
  }

  for(double hz = hz_start; hz <= hz_end + 0.0001; hz += hz_inc){
    double strength = 0;
    int off = search_time_fine_known(bins, rate, syms,
                                     off0 - off_win, off0 + off_win, hz,
                                     off_inc, strength);
    if(off >= 0 && (got_best == 0 || strength > best_strength)){
      got_best = 1;
      best_hz = hz;
      best_off = off;
      best_strength = strength;
    }
  }

  if(got_best){
    hz_out = best_hz;
    off_out = best_off / (double) rate;
  }
}

//
// shift frequency by shifting the bins of one giant FFT.
// so no problem with phase mismatch &c at block boundaries.
// surprisingly fast at 200 samples/second.
// shifts *down* by hz.
//
std::vector<double>
fft_shift(const std::vector<double> &samples, int off, int len,
          int rate, double hz)
{
  std::vector<std::complex<double>> bins;

  // horrible hack to avoid repeated FFTs on the same input.
  hack_mu_.lock();
  if((int) samples.size() == hack_size_ && samples.data() == hack_data_ &&
     off == hack_off_ && len == hack_len_ &&
     samples[0] == hack_0_ && samples[1] == hack_1_){
    bins = hack_bins_;
  } else {
    bins = one_fft(samples, off, len, "fft_shift", 0);
    hack_bins_ = bins;
    hack_size_ = samples.size();
    hack_off_ = off;
    hack_len_ = len;
    hack_0_ = samples[0];
    hack_1_ = samples[1];
    hack_data_ = samples.data();
  }
  hack_mu_.unlock();

  return fft_shift_f(bins, rate, hz);
}

//
// shift down by hz.
//
std::vector<double>
fft_shift_f(const std::vector<std::complex<double>> &bins, int rate, double hz)
{
  int nbins = bins.size();
  int len = (nbins - 1) * 2;

  double bin_hz = rate / (double) len;
  int down = round(hz / bin_hz);
  std::vector<std::complex<double>> bins1(nbins);
  for(int i = 0; i < nbins; i++){
    int j = i + down;
    if(j >= 0 && j < nbins){
      bins1[i] = bins[j];
    } else {
      bins1[i] = 0;
    }
  }
  std::vector<double> out = one_ifft(bins1, "fft_shift");
  return out;
}

// shift the frequency by a fraction of 6.25,
// to center hz on bin 4 (25 hz).
std::vector<double>
shift200(const std::vector<double> &samples200, int off, int len, double hz)
{
  if(std::abs(hz - 25) < 0.001 && off == 0 && len == (int) samples200.size()){
    return samples200;
  } else {
    return fft_shift(samples200, off, len, 200, hz - 25.0);
  }
  // return hilbert_shift(samples200, hz - 25.0, hz - 25.0, 200);
}

// returns a mini-FFT of 79 8-tone symbols.
ffts_t
extract(const std::vector<double> &samples200, double hz, int off)
{

  ffts_t bins3 = ffts(samples200, off, 32, "extract");

  ffts_t m79(79);
  for(int si = 0; si < 79; si++){
    m79[si].resize(8);
    if(si < (int)bins3.size()){
      for(int bi = 0; bi < 8; bi++){
        auto x = bins3[si][4+bi];
        m79[si][bi] = x;
      }
    } else {
      for(int bi = 0; bi < 8; bi++){
        m79[si][bi] = 0;
      }
    }
  }

  return m79;
}

//
// m79 is a 79x8 array of complex.
//
ffts_t
un_gray_code_c(const ffts_t &m79)
{
  ffts_t m79a(79);

  int map[] = { 0, 1, 3, 2, 6, 4, 5, 7 };
  for(int si = 0; si < 79; si++){
    m79a[si].resize(8);
    for(int bi = 0; bi < 8; bi++){
      m79a[si][map[bi]] = m79[si][bi];
    }
  }

  return m79a;
}

//
// m79 is a 79x8 array of double.
//
std::vector< std::vector<double> > 
un_gray_code_r(const std::vector<std::vector<double>> &m79)
{
  std::vector< std::vector<double> > m79a(79);

  int map[] = { 0, 1, 3, 2, 6, 4, 5, 7 };
  for(int si = 0; si < 79; si++){
    m79a[si].resize(8);
    for(int bi = 0; bi < 8; bi++){
      m79a[si][map[bi]] = m79[si][bi];
    }
  }

  return m79a;
}

//
// normalize levels by windowed median.
// this helps, but why?
//
std::vector< std::vector<double> >
convert_to_snr(const std::vector< std::vector<double> > &m79)
{
  if(snr_how < 0 || snr_win < 0)
    return m79;

  //
  // for each symbol time, what's its "noise" level?
  //
  std::vector<double> mm(79);
  for(int si = 0; si < 79; si++){
    std::vector<double> v(8);
    double sum = 0.0;
    for(int bi = 0; bi < 8; bi++){
      double x = m79[si][bi];
      v[bi] = x;
      sum += x;
    }
    if(snr_how != 1)
      std::sort(v.begin(), v.end());
    if(snr_how == 0){
      // median
      mm[si] = (v[3] + v[4]) / 2;
    } else if(snr_how == 1){
      mm[si] = sum / 8;
    } else if(snr_how == 2){
      // all but strongest tone.
      mm[si] = (v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6]) / 7;
    } else if(snr_how == 3){
      mm[si] = v[0]; // weakest tone
    } else if(snr_how == 4){
      mm[si] = v[7]; // strongest tone
    } else if(snr_how == 5){
      mm[si] = v[6]; // second-strongest tone
    } else {
      mm[si] = 1.0;
    }
  }

  // we're going to take a windowed average.
  std::vector<double> winwin;
  if(snr_win > 0){
    winwin = blackman(2*snr_win+1);
  } else {
    winwin.push_back(1.0);
  }

  std::vector<std::vector<double>> n79(79);
    
  for(int si = 0; si < 79; si++){
    double sum = 0;
    for(int dd = si - snr_win; dd <= si + snr_win; dd++){
      int wi = dd - (si - snr_win);
      if(dd >= 0 && dd < 79){
        sum += mm[dd] * winwin[wi];
      } else if(dd < 0){
        sum += mm[0] * winwin[wi];
      } else {
        sum += mm[78] * winwin[wi];
      }
    }
    n79[si].resize(8);
    for(int bi = 0; bi < 8; bi++){
      n79[si][bi] = m79[si][bi] / sum;
    }
  }

  return n79;
}

//
// normalize levels by windowed median.
// this helps, but why?
//
std::vector< std::vector<std::complex<double>> >
c_convert_to_snr(const std::vector< std::vector<std::complex<double>> > &m79)
{
  if(snr_how < 0 || snr_win < 0)
    return m79;

  //
  // for each symbol time, what's its "noise" level?
  //
  std::vector<double> mm(79);
  for(int si = 0; si < 79; si++){
    std::vector<double> v(8);
    double sum = 0.0;
    for(int bi = 0; bi < 8; bi++){
      double x = std::abs(m79[si][bi]);
      v[bi] = x;
      sum += x;
    }
    if(snr_how != 1)
      std::sort(v.begin(), v.end());
    if(snr_how == 0){
      // median
      mm[si] = (v[3] + v[4]) / 2;
    } else if(snr_how == 1){
      mm[si] = sum / 8;
    } else if(snr_how == 2){
      // all but strongest tone.
      mm[si] = (v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6]) / 7;
    } else if(snr_how == 3){
      mm[si] = v[0]; // weakest tone
    } else if(snr_how == 4){
      mm[si] = v[7]; // strongest tone
    } else if(snr_how == 5){
      mm[si] = v[6]; // second-strongest tone
    } else {
      mm[si] = 1.0;
    }
  }

  // we're going to take a windowed average.
  std::vector<double> winwin;
  if(snr_win > 0){
    winwin = blackman(2*snr_win+1);
  } else {
    winwin.push_back(1.0);
  }

  std::vector<std::vector<std::complex<double>>> n79(79);
    
  for(int si = 0; si < 79; si++){
    double sum = 0;
    for(int dd = si - snr_win; dd <= si + snr_win; dd++){
      int wi = dd - (si - snr_win);
      if(dd >= 0 && dd < 79){
        sum += mm[dd] * winwin[wi];
      } else if(dd < 0){
        sum += mm[0] * winwin[wi];
      } else {
        sum += mm[78] * winwin[wi];
      }
    }
    n79[si].resize(8);
    for(int bi = 0; bi < 8; bi++){
      n79[si][bi] = m79[si][bi] / sum;
    }
  }

  return n79;
}

//
// statistics to decide soft probabilities,
// to drive LDPC decoder.
// distribution of strongest tones, and
// distribution of noise.
//
void
make_stats(const std::vector<std::vector<double>> &m79,
           Stats &bests,
           Stats &all)
{
  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };

  for(int si = 0; si < 79; si++){
    if(si < 7 || (si >= 36 && si < 36 + 7) || si >= 72){
      // Costas.
      int ci;
      if(si >= 72) ci = si - 72;
      else if(si >= 36) ci = si - 36;
      else ci = si;
      for(int bi = 0; bi < 8; bi++){
        double x = m79[si][bi];
        all.add(x);
        if(bi == costas[ci]){
          bests.add(x);
        }
      }
    } else {
      double mx = 0;
      for(int bi = 0; bi < 8; bi++){
        double x = m79[si][bi];
        if(x > mx)
          mx = x;
        all.add(x);
      }
      bests.add(mx);
    }
  }
}

//
// convert 79x8 complex FFT bins to magnitudes.
//
// exploits local phase coherence by decreasing magnitudes of bins
// whose phase is far from the phases of nearby strongest tones.
//
// relies on each tone being reasonably well centered in its FFT bin
// (in time and frequency) so that each tone completes an integer
// number of cycles and thus preserves phase from one symbol to the
// next.
//
std::vector<std::vector<double>>
soft_c2m(const ffts_t &c79)
{
  std::vector< std::vector<double> > m79(79);
  std::vector<double> raw_phases(79); // of strongest tone in each symbol time
  for(int si = 0; si < 79; si++){
    m79[si].resize(8);
    int mxi = -1;
    double mx;
    double mx_phase;
    for(int bi = 0; bi < 8; bi++){
      double x = std::abs(c79[si][bi]);
      m79[si][bi] = x;
      if(mxi < 0 || x > mx){
        mxi = bi;
        mx = x;
        mx_phase = std::arg(c79[si][bi]); // -pi .. pi
      }
    }
    raw_phases[si] = mx_phase;
  }

  if(soft_phase_win <= 0)
    return m79;

  // phase around each symbol.
  std::vector<double> phases(79);

  // for each symbol time, median of nearby phases
  for(int si = 0; si < 79; si++){
    std::vector<double> v;
    for(int si1 = si - soft_phase_win; si1 <= si + soft_phase_win; si1++){
      if(si1 >= 0 && si1 < 79){
        double x = raw_phases[si1];
        v.push_back(x);
      }
    }

    // choose the phase that has the lowest total distance to other
    // phases. like median but avoids -pi..pi wrap-around.
    int n = v.size();
    int best = -1;
    double best_score = 0;
    for(int i = 0; i < n; i++){
      double score = 0;
      for(int j = 0; j < n; j++){
        if(i == j)
          continue;
        double d = fabs(v[i] - v[j]);
        if(d > M_PI)
          d = 2*M_PI - d;
        score += d;
      }
      if(best == -1 || score < best_score){
        best = i;
        best_score = score;
      }
    }
    phases[si] = v[best];
  }

  // project each tone against the median phase around that symbol time.
  for(int si = 0; si < 79; si++){
    for(int bi = 0; bi < 8; bi++){
      double mag = std::abs(c79[si][bi]);
      double angle = std::arg(c79[si][bi]);
      double d = angle - phases[si];
      double factor = 0.1;
      if(d < M_PI/2 && d > -M_PI/2){
        factor = cos(d);
      }
      m79[si][bi] = factor * mag;
    }
  }
  
  return m79;
}

//
// guess the probability that a bit is zero vs one,
// based on strengths of strongest tones that would
// give it those values. for soft LDPC decoding.
//
// returns log-likelihood, zero is positive, one is negative.
//
double
bayes(double best_zero, double best_one, int lli,
      Stats &bests, Stats &all)
{
  double maxlog = 4.97;
  double ll = 0;
      
  double pzero = 0.5;
  double pone = 0.5;
  if(use_apriori){
    pzero = 1.0 - apriori174[lli];
    pone = apriori174[lli];
  }
  
  //
  // Bayes combining rule normalization from:
  // http://cs.wellesley.edu/~anderson/writing/naive-bayes.pdf
  //
  // a = P(zero)P(e0|zero)P(e1|zero)
  // b = P(one)P(e0|one)P(e1|one)
  // p = a / (a + b)
  //
  // also see Mark Owen's book Practical Signal Processing,
  // Chapter 6.
  //

  // zero
  double a = pzero *
    bests.problt(best_zero) *
    (1.0 - all.problt(best_one));
  if(bayes_how == 1)
    a *= all.problt(all.mean() + (best_zero - best_one));
  
  // one
  double b = pone *
    bests.problt(best_one) *
    (1.0 - all.problt(best_zero));
  if(bayes_how == 1)
    b *= all.problt(all.mean() + (best_one - best_zero));
  
  double p;
  if(a + b == 0){
    p = 0.5;
  } else {
    p = a / (a + b);
  }

  if(1 - p == 0.0){
    ll = maxlog;
  } else {
    ll = log(p / (1 - p));
  }
  
  if(ll > maxlog)
    ll = maxlog;
  if(ll < -maxlog)
    ll = -maxlog;

  return ll;
}

//
// c79 is 79x8 complex tones, before un-gray-coding.
//
void
soft_decode(const ffts_t &c79, double ll174[])
{
  std::vector< std::vector<double> > m79(79);

  // m79 = absolute values of c79.
  // still pre-un-gray-coding so we know which
  // are the correct Costas tones.
  m79 = soft_c2m(c79);

  m79 = convert_to_snr(m79);
    
  // statistics to decide soft probabilities.
  // distribution of strongest tones, and
  // distribution of noise.
  Stats bests(problt_how_sig);
  Stats all(problt_how_noise);
  make_stats(m79, bests, all);

  m79 = un_gray_code_r(m79);

  int lli = 0;
  for(int i79 = 0; i79 < 79; i79++){
    if(i79 < 7 || (i79 >= 36 && i79 < 36+7) || i79 >= 72){
      // Costas, skip
      continue;
    }

    // for each of the three bits, look at the strongest tone
    // that would make it a zero, and the strongest tone that
    // would make it a one. use Bayes to decide which is more
    // likely, comparing each against the distribution of noise
    // and the distribution of strongest tones.
    // most-significant-bit first.

    for(int biti = 0; biti < 3; biti++){
      // tone numbers that make this bit zero or one.
      int zeroi[4];
      int onei[4];
      if(biti == 0){
        // high bit
        zeroi[0] = 0; zeroi[1] = 1; zeroi[2] = 2; zeroi[3] = 3;
        onei[0] = 4; onei[1] = 5; onei[2] = 6; onei[3] = 7;
      }
      if(biti == 1){
        // middle bit
        zeroi[0] = 0; zeroi[1] = 1; zeroi[2] = 4; zeroi[3] = 5;
        onei[0] = 2; onei[1] = 3; onei[2] = 6; onei[3] = 7;
      }
      if(biti == 2){
        // low bit
        zeroi[0] = 0; zeroi[1] = 2; zeroi[2] = 4; zeroi[3] = 6;
        onei[0] = 1; onei[1] = 3; onei[2] = 5; onei[3] = 7;
      }

      // strongest tone that would make this bit be zero.
      int got_best_zero = 0;
      double best_zero = 0;
      for(int i = 0; i < 4; i++){
        double x = m79[i79][zeroi[i]];
        if(got_best_zero == 0 || x > best_zero){
          got_best_zero = 1;
          best_zero = x;
        }
      }

      // strongest tone that would make this bit be one.
      int got_best_one = 0;
      double best_one = 0;
      for(int i = 0; i < 4; i++){
        double x = m79[i79][onei[i]];
        if(got_best_one == 0 || x > best_one){
          got_best_one = 1;
          best_one = x;
        }
      }

      double ll = bayes(best_zero, best_one, lli, bests, all);
      
      ll174[lli++] = ll;
    }
  }
  assert(lli == 174);
}

//
// c79 is 79x8 complex tones, before un-gray-coding.
//
void
c_soft_decode(const ffts_t &c79x, double ll174[])
{
  ffts_t c79 = c_convert_to_snr(c79x);

  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };
  std::complex<double> maxes[79];
  for(int i = 0; i < 79; i++){
    std::complex<double> m;
    if(i < 7){
      // Costas.
      m = c79[i][costas[i]];
    } else if(i >= 36 && i < 36 + 7){
      // Costas.
      m = c79[i][costas[i-36]];
    } else if(i >= 72){
      // Costas.
      m = c79[i][costas[i-72]];
    } else {
      int got = 0;
      for(int j = 0; j < 8; j++){
        if(got == 0 || std::abs(c79[i][j]) > std::abs(m)){
          got = 1;
          m = c79[i][j];
        }
      }
    }
    maxes[i] = m;
  }

  std::vector<std::vector<double>> m79(79);
  for(int i = 0; i < 79; i++){
    m79[i].resize(8);
    for(int j = 0; j < 8; j++){
      std::complex<double> c = c79[i][j];
      int n = 0;
      double sum = 0;
      for(int k = i - c_soft_win; k <= i + c_soft_win; k++){
        if(k < 0 || k >= 79)
          continue;
        if(k == i){
          sum -= c_soft_weight * std::abs(c);
        } else {
          // we're expecting all genuine tones to have
          // about the same phase and magnitude.
          // so set m79[i][j] to the distance from the
          // phase/magnitude predicted by surrounding
          // genuine-looking tones.
          std::complex<double> c1 = maxes[k];
          std::complex<double> d = c1 - c;
          sum += std::abs(d);
        }
        n += 1;
      }
      m79[i][j] = 0 - (sum / n);
    }
  }
    
  // statistics to decide soft probabilities.
  // distribution of strongest tones, and
  // distribution of noise.
  Stats bests(problt_how_sig);
  Stats all(problt_how_noise);
  make_stats(m79, bests, all);

  m79 = un_gray_code_r(m79);

  int lli = 0;
  for(int i79 = 0; i79 < 79; i79++){
    if(i79 < 7 || (i79 >= 36 && i79 < 36+7) || i79 >= 72){
      // Costas, skip
      continue;
    }

    // for each of the three bits, look at the strongest tone
    // that would make it a zero, and the strongest tone that
    // would make it a one. use Bayes to decide which is more
    // likely, comparing each against the distribution of noise
    // and the distribution of strongest tones.
    // most-significant-bit first.

    for(int biti = 0; biti < 3; biti++){
      // tone numbers that make this bit zero or one.
      int zeroi[4];
      int onei[4];
      if(biti == 0){
        // high bit
        zeroi[0] = 0; zeroi[1] = 1; zeroi[2] = 2; zeroi[3] = 3;
        onei[0] = 4; onei[1] = 5; onei[2] = 6; onei[3] = 7;
      }
      if(biti == 1){
        // middle bit
        zeroi[0] = 0; zeroi[1] = 1; zeroi[2] = 4; zeroi[3] = 5;
        onei[0] = 2; onei[1] = 3; onei[2] = 6; onei[3] = 7;
      }
      if(biti == 2){
        // low bit
        zeroi[0] = 0; zeroi[1] = 2; zeroi[2] = 4; zeroi[3] = 6;
        onei[0] = 1; onei[1] = 3; onei[2] = 5; onei[3] = 7;
      }

      // strongest tone that would make this bit be zero.
      int got_best_zero = 0;
      double best_zero = 0;
      for(int i = 0; i < 4; i++){
        double x = m79[i79][zeroi[i]];
        if(got_best_zero == 0 || x > best_zero){
          got_best_zero = 1;
          best_zero = x;
        }
      }

      // strongest tone that would make this bit be one.
      int got_best_one = 0;
      double best_one = 0;
      for(int i = 0; i < 4; i++){
        double x = m79[i79][onei[i]];
        if(got_best_one == 0 || x > best_one){
          got_best_one = 1;
          best_one = x;
        }
      }

      double ll = bayes(best_zero, best_one, lli, bests, all);
      
      ll174[lli++] = ll;
    }
  }
  assert(lli == 174);
}

//
// turn 79 symbol numbers into 174 bits.
// strip out the three Costas sync blocks,
// leaving 58 symbol numbers.
// each represents three bits.
// (all post-un-gray-code).
// str is per-symbol strength; must be positive.
// each returned element is < 0 for 1, > 0 for zero,
// scaled by str.
//
std::vector<double>
extract_bits(const std::vector<int> &syms, const std::vector<double> str)
{
  assert(syms.size() == 79);
  assert(str.size() == 79);

  std::vector<double> bits;
  for(int si = 0; si < 79; si++){
    if(si < 7 || (si >= 36 && si < 36+7) || si >= 72){
      // costas -- skip
    } else {
      bits.push_back((syms[si] & 4) == 0 ? str[si] : -str[si]);
      bits.push_back((syms[si] & 2) == 0 ? str[si] : -str[si]);
      bits.push_back((syms[si] & 1) == 0 ? str[si] : -str[si]);
    }
  }

  return bits;
}

// decode successive pairs of symbols. exploits the likelyhood
// that they have the same phase, by summing the complex
// correlations for each possible pair and using the max.
void
soft_decode_pairs(const ffts_t &m79x,
                  double ll174[])
{
  ffts_t m79 = c_convert_to_snr(m79x);
                       
  struct BitInfo {
    double zero; // strongest correlation that makes it zero
    double one;  // and one
  };
  std::vector<BitInfo> bitinfo(79*3);
  for(int i = 0; i < (int) bitinfo.size(); i++){
    bitinfo[i].zero = 0;
    bitinfo[i].one = 0;
  }

  Stats all(problt_how_noise);
  Stats bests(problt_how_sig);

  int map[] = { 0, 1, 3, 2, 6, 4, 5, 7 }; // un-gray-code

  for(int si = 0; si < 79; si += 2){
    double mx = 0;
    double corrs[8*8];
    for(int s1 = 0; s1 < 8; s1++){
      for(int s2 = 0; s2 < 8; s2++){
        // sum up the correlations.
        std::complex<double> csum = m79[si][s1];
        if(si+1 < 79)
          csum += m79[si+1][s2];
        double x = std::abs(csum);
          
        corrs[s1*8+s2] = x;
        if(x > mx)
          mx = x;

        all.add(x);

        // first symbol
        int i = map[s1];
        for(int bit = 0; bit < 3; bit++){
          int bitind = (si + 0) * 3 + (2 - bit);
          if((i & (1 << bit))){
            // symbol i would make this bit a one.
            if(x > bitinfo[bitind].one){
              bitinfo[bitind].one = x;
            }
          } else {
            // symbol i would make this bit a zero.
            if(x > bitinfo[bitind].zero){
              bitinfo[bitind].zero = x;
            }
          }
        }

        // second symbol
        if(si + 1 < 79){
          i = map[s2];
          for(int bit = 0; bit < 3; bit++){
            int bitind = (si + 1) * 3 + (2 - bit);
            if((i & (1 << bit))){
              // symbol i would make this bit a one.
              if(x > bitinfo[bitind].one){
                bitinfo[bitind].one = x;
              }
            } else {
              // symbol i would make this bit a zero.
              if(x > bitinfo[bitind].zero){
                bitinfo[bitind].zero = x;
              }
            }
          }
        }

      }
    }
    if(si == 0 || si == 36 || si == 72){
      bests.add(corrs[3*8+1]);
    } else if(si == 2 || si == 38 || si == 74){
      bests.add(corrs[4*8+0]);
    } else if(si == 4 || si == 40 || si == 76){
      bests.add(corrs[6*8+5]);
    } else {
      bests.add(mx);
    }
  }

  int lli = 0;
  for(int si = 0; si < 79; si++){
    if(si < 7 || (si >= 36 && si < 36+7) || si >= 72){
      // costas
      continue;
    }
    for(int i = 0; i < 3; i++){
      double best_zero = bitinfo[si*3+i].zero;
      double best_one = bitinfo[si*3+i].one;
      // ll174[lli++] = best_zero > best_one ? 4.99 : -4.99;

      double ll = bayes(best_zero, best_one, lli, bests, all);
      
      ll174[lli++] = ll;
    }
  }
  assert(lli == 174);
}

void
soft_decode_triples(const ffts_t &m79x,
                    double ll174[])
{
  ffts_t m79 = c_convert_to_snr(m79x);
  
  struct BitInfo {
    double zero; // strongest correlation that makes it zero
    double one;  // and one
  };
  std::vector<BitInfo> bitinfo(79*3);
  for(int i = 0; i < (int) bitinfo.size(); i++){
    bitinfo[i].zero = 0;
    bitinfo[i].one = 0;
  }

  Stats all(problt_how_noise);
  Stats bests(problt_how_sig);

  int map[] = { 0, 1, 3, 2, 6, 4, 5, 7 }; // un-gray-code

  for(int si = 0; si < 79; si += 3){
    double mx = 0;
    double corrs[8*8*8];
    for(int s1 = 0; s1 < 8; s1++){
      for(int s2 = 0; s2 < 8; s2++){
        for(int s3 = 0; s3 < 8; s3++){
          std::complex<double> csum = m79[si][s1];
          if(si+1 < 79)
            csum += m79[si+1][s2];
          if(si+2 < 79)
            csum += m79[si+2][s3];
          double x = std::abs(csum);

          corrs[s1*64+s2*8+s3] = x;
          if(x > mx)
            mx = x;

          all.add(x);

          // first symbol
          int i = map[s1];
          for(int bit = 0; bit < 3; bit++){
            int bitind = (si + 0) * 3 + (2 - bit);
            if((i & (1 << bit))){
              // symbol i would make this bit a one.
              if(x > bitinfo[bitind].one){
                bitinfo[bitind].one = x;
              }
            } else {
              // symbol i would make this bit a zero.
              if(x > bitinfo[bitind].zero){
                bitinfo[bitind].zero = x;
              }
            }
          }

          // second symbol
          if(si + 1 < 79){
            i = map[s2];
            for(int bit = 0; bit < 3; bit++){
              int bitind = (si + 1) * 3 + (2 - bit);
              if((i & (1 << bit))){
                // symbol i would make this bit a one.
                if(x > bitinfo[bitind].one){
                  bitinfo[bitind].one = x;
                }
              } else {
                // symbol i would make this bit a zero.
                if(x > bitinfo[bitind].zero){
                  bitinfo[bitind].zero = x;
                }
              }
            }
          }

          // third symbol
          if(si + 2 < 79){
            i = map[s3];
            for(int bit = 0; bit < 3; bit++){
              int bitind = (si + 2) * 3 + (2 - bit);
              if((i & (1 << bit))){
                // symbol i would make this bit a one.
                if(x > bitinfo[bitind].one){
                  bitinfo[bitind].one = x;
                }
              } else {
                // symbol i would make this bit a zero.
                if(x > bitinfo[bitind].zero){
                  bitinfo[bitind].zero = x;
                }
              }
            }
          }
        }
      }
    }

    // costas: 3, 1, 4, 0, 6, 5, 2 
    if(si == 0 || si == 36 || si == 72){
      bests.add(corrs[3*64+1*8+4]);
    } else if(si == 3 || si == 39 || si == 75){
      bests.add(corrs[0*64+6*8+5]);
    } else {
      bests.add(mx);
    }
  }

  int lli = 0;
  for(int si = 0; si < 79; si++){
    if(si < 7 || (si >= 36 && si < 36+7) || si >= 72){
      // costas
      continue;
    }
    for(int i = 0; i < 3; i++){
      double best_zero = bitinfo[si*3+i].zero;
      double best_one = bitinfo[si*3+i].one;
      // ll174[lli++] = best_zero > best_one ? 4.99 : -4.99;

      double ll = bayes(best_zero, best_one, lli, bests, all);
      
      ll174[lli++] = ll;
    }
  }
  assert(lli == 174);
}

//
// given log likelyhood for each bit, try LDPC and OSD decoders.
// on success, puts corrected 174 bits into a174[].
//
int
decode(const double ll174[], int a174[], int use_osd, std::string &comment)
{
  void ldpc_decode(double llcodeword[], int iters, int plain[], int *ok);
  void ldpc_decode_log(double codeword[], int iters, int plain[], int *ok);

  int plain[174]; // will be 0/1 bits.
  int ldpc_ok = 0;     // 83 will mean success.

  ldpc_decode((double*)ll174, ldpc_iters, plain, &ldpc_ok);

  int ok_thresh = 83; // 83 is perfect
  if(ldpc_ok >= ok_thresh){
    // plain[] is 91 systematic data bits, 83 parity bits.
    for(int i = 0; i < 174; i++){
      a174[i] = plain[i];
    }
    if(check_crc(a174)){
      // success!
      return 1;
    }
  }

  if(use_osd && osd_depth >= 0 && ldpc_ok >= osd_ldpc_thresh){
    extern int osd_decode(double codeword[174], int depth, int out[91], int*);
    extern void ldpc_encode(int plain[91], int codeword[174]);

    int oplain[91];
    int got_depth = -1;
    int osd_ok = osd_decode((double*)ll174, osd_depth, oplain, &got_depth);
    if(osd_ok){
      // reconstruct all 174.
      comment += "OSD-" + std::to_string(got_depth) + "-" + std::to_string(ldpc_ok);
      ldpc_encode(oplain, a174);
      return 1;
    }
  }
  
  return 0;
}

//
// bandpass filter some FFT bins.
// smooth transition from stop-band to pass-band,
// so that it's not a brick-wall filter, so that it
// doesn't ring.
//
std::vector<std::complex<double>>
fbandpass(const std::vector<std::complex<double>> &bins0,
          double bin_hz,
          double low_outer,  // start of transition
          double low_inner,  // start of flat area
          double high_inner, // end of flat area
          double high_outer) // end of transition
{
  // assert(low_outer >= 0);
  assert(low_outer <= low_inner);
  assert(low_inner <= high_inner);
  assert(high_inner <= high_outer);
  // assert(high_outer <= bin_hz * bins0.size());

  int nbins = bins0.size();
  std::vector<std::complex<double>> bins1(nbins);

  for(int i = 0; i < nbins; i++){
    double ihz = i * bin_hz;
    // cos(x)+flat+cos(x) taper
    double factor;
    if(ihz <= low_outer || ihz >= high_outer){
      factor = 0;
    } else if(ihz >= low_outer && ihz < low_inner){
      // rising shoulder
#if 1
      factor = (ihz - low_outer) / (low_inner-low_outer); // 0 .. 1
#else
      double theta = (ihz - low_outer) / (low_inner-low_outer); // 0 .. 1
      theta -= 1; // -1 .. 0
      theta *= 3.14159; // -pi .. 0
      factor = cos(theta); // -1 .. 1
      factor = (factor + 1) / 2; // 0 .. 1
#endif
    } else if(ihz > high_inner && ihz <= high_outer){
      // falling shoulder
#if 1
      factor =  (high_outer - ihz) / (high_outer-high_inner); // 1 .. 0
#else
      double theta =  (high_outer - ihz) / (high_outer-high_inner); // 1 .. 0
      theta = 1.0 - theta; // 0 .. 1
      theta *= 3.14159; // 0 .. pi
      factor = cos(theta); // 1 .. -1
      factor = (factor + 1) / 2; // 1 .. 0
#endif
    } else {
      factor = 1.0;
    }
    bins1[i] = bins0[i] * factor;
  }

  return bins1;
}

//
// move hz down to 25, filter+convert to 200 samples/second.
//
// like fft_shift(). one big FFT, move bins down and
// zero out those outside the band, then IFFT,
// then re-sample.
//
// XXX maybe merge w/ fft_shift() / shift200().
//
std::vector<double>
down_v7(const std::vector<double> &samples, double hz)
{
  int len = samples.size();
  std::vector<std::complex<double>> bins = one_fft(samples, 0, len, "down_v7a", 0);

  return down_v7_f(bins, len, hz);
}

std::vector<double>
down_v7_f(const std::vector<std::complex<double>> &bins, int len, double hz)
{
  int nbins = bins.size();

  double bin_hz = rate_ / (double) len;
  int down = round((hz - 25) / bin_hz);
  std::vector<std::complex<double>> bins1(nbins);
  for(int i = 0; i < nbins; i++){
    int j = i + down;
    if(j >= 0 && j < nbins){
      bins1[i] = bins[j];
    } else {
      bins1[i] = 0;
    }
  }

  // now filter to fit in 200 samples/second.

  double low_inner = 25.0 - shoulder200_extra;
  double low_outer = low_inner - shoulder200;
  if(low_outer < 0)
    low_outer = 0;
  double high_inner = 75 - 6.25 + shoulder200_extra;
  double high_outer = high_inner + shoulder200;
  if(high_outer > 100)
    high_outer = 100;

  bins1 = fbandpass(bins1, bin_hz,
                    low_outer, low_inner,
                    high_inner, high_outer);

  // convert back to time domain and down-sample to 200 samples/second.
  int blen = round(len * (200.0 / rate_));
  std::vector<std::complex<double>> bbins(blen / 2 + 1);
  for(int i = 0; i < (int) bbins.size(); i++)
    bbins[i] = bins1[i];
  std::vector<double> out = one_ifft(bbins, "down_v7b");
  
  return out;
}

//
// putative start of signal is at hz and symbol si0.
// 
// return 2 if it decodes to a brand-new message.
// return 1 if it decodes but we've already seen it,
//   perhaps in a different pass.
// return 0 if we could not decode.
//
// XXX merge with one_iter().
//
int
one(const std::vector<std::complex<double>> &bins, int len, double hz, int off)
{
  //
  // set up to search for best frequency and time offset.
  //

  //
  // move down to 25 hz and re-sample to 200 samples/second,
  // i.e. 32 samples/symbol.
  //
  std::vector<double> samples200 = down_v7_f(bins, len, hz);

  int off200 = round((off / (double) rate_) * 200.0);

  int ret = one_iter(samples200, off200, hz);
  return ret;
}

// return 2 if it decodes to a brand-new message.
// return 1 if it decodes but we've already seen it,
//   perhaps in a different pass.
// return 0 if we could not decode.
int
one_iter(const std::vector<double> &samples200, int best_off, double hz_for_cb)
{
  if(do_second){
    std::vector<Strength> strengths =
      search_both(samples200,
                  25, second_hz_n, second_hz_win,
                  best_off, second_off_n, second_off_win * 32);
    //
    // sort strongest-first.
    //
    std::sort(strengths.begin(), strengths.end(),
              [](const Strength &a, const Strength &b) -> bool
              { return a.strength_ > b.strength_; } );
    
    for(int i = 0; i < (int) strengths.size() && i < second_count; i++){
      double hz = strengths[i].hz_;
      int off = strengths[i].off_;
      int ret = one_iter1(samples200, off, hz, hz_for_cb, hz_for_cb);
      if(ret > 0){
        return ret;
      }
    }
  } else {
    int ret = one_iter1(samples200, best_off, 25, hz_for_cb, hz_for_cb);
    return ret;
  }

  return 0;
}

//
// estimate SNR, yielding numbers vaguely similar to WSJT-X.
// m79 is a 79x8 complex FFT output.
//
double
guess_snr(const ffts_t &m79)
{
  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };
  double noises = 0;
  double signals = 0;

  for(int i = 0; i < 7; i++){
    signals += std::abs(m79[i][costas[i]]);
    signals += std::abs(m79[36+i][costas[i]]);
    signals += std::abs(m79[72+i][costas[i]]);
    noises += std::abs(m79[i][(costas[i]+4)%8]);
    noises += std::abs(m79[36+i][(costas[i]+4)%8]);
    noises += std::abs(m79[72+i][(costas[i]+4)%8]);
  }

  for(int i = 0; i < 79; i++){
    if(i < 7 || (i >= 36 && i < 36+7) || (i >= 72 && i < 72+7))
      continue;
    std::vector<double> v(8);
    for(int j = 0; j < 8; j++){
      v[j] = std::abs(m79[i][j]);
    }
    std::sort(v.begin(), v.end());
    signals += v[7]; // strongest tone, probably the signal
    noises += (v[2]+v[3]+v[4])/3;
  }

  noises /= 79;
  signals /= 79;

  noises *= noises; // square yields power
  signals *= signals;

  double raw = signals / noises;
  raw -= 1; // turn (s+n)/n into s/n
  if(raw < 0.1)
    raw = 0.1;
  raw /= (2500.0 / 2.7); // 2.7 hz noise b/w -> 2500 hz b/w
  double snr = 10 * log10(raw);
  snr += 5;
  snr *= 1.4;
  return snr;
}

//
// compare phases of successive symbols to guess whether
// the starting offset is a little too high or low.
// we expect each symbol to have the same phase.
// an error in causes the phase to advance at a steady rate.
// so if hz is wrong, we expect the phase to advance
// or retard at a steady pace.
// an error in offset causes each symbol to start at
// a phase that depends on the symbol's frequency;
// a particular offset error causes a phase error
// that depends on frequency.
// hz0 is actual FFT bin number of m79[...][0] (always 4).
//
// the output adj_hz is relative to the FFT bin center;
// a positive number means the real signal seems to be
// a bit higher in frequency that the bin center.
//
// adj_off is the amount to change the offset, in samples.
// should be subtracted from offset.
//
void
fine(const ffts_t &m79, int bin0, double &adj_hz, double &adj_off)
{
  adj_hz = 0.0;
  adj_off = 0.0;
  
  // tone number for each of the 79 symbols.
  int sym[79];
  double symval[79];
  double symphase[79];
  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };
  for(int i = 0; i < 79; i++){
    if(i < 7){
      sym[i] = costas[i];
    } else if(i >= 36 && i < 36+7){
      sym[i] = costas[i-36];
    } else if(i >= 72){
      sym[i] = costas[i-72];
    } else {
      int mxj = -1;
      double mx = 0;
      for(int j = 0; j < 8; j++){
        double x = std::abs(m79[i][j]);
        if(mxj < 0 || x > mx){
          mx = x;
          mxj = j;
        }
      }
      sym[i] = mxj;
    }
    symphase[i] = std::arg(m79[i][sym[i]]);
    symval[i] = std::abs(m79[i][sym[i]]);
  }

  double sum = 0;
  double weight_sum = 0;
  for(int i = 0; i < 79-1; i++){
    double d = symphase[i+1] - symphase[i];
    while(d > M_PI)
      d -= 2*M_PI;
    while(d < -M_PI)
      d += 2*M_PI;
    double w = symval[i];
    sum += d * w;
    weight_sum += w;
  }
  double mean = sum / weight_sum;

  double err_rad = mean; // radians per symbol time

  double err_hz = (err_rad / (2*M_PI)) / 0.16; // cycles per symbol time

  // if each symbol's phase is a bit more than we expect,
  // that means the real frequency is a bit higher
  // than we thought, so increase our estimate.
  adj_hz = err_hz;

  //
  // now think about offset error.
  //
  // the higher tones have many cycles per
  // symbol -- e.g. tone 7 has 11 cycles
  // in each symbol. a one- or two-sample
  // offset error at such a high tone will
  // change the phase by pi or more,
  // which makes the phase-to-samples
  // conversion ambiguous. so only try
  // to distinguish early-ontime-late,
  // not the amount.
  //
  int nearly = 0;
  int nlate = 0;
  double early = 0.0;
  double late = 0.0;
  for(int i = 1; i < 79; i++){
    double ph0 = std::arg(m79[i-1][sym[i-1]]);
    double ph = std::arg(m79[i][sym[i]]);
    double d = ph - ph0;
    d -= err_rad; // correct for hz error.
    while(d > M_PI)
      d -= 2*M_PI;
    while(d < -M_PI)
      d += 2*M_PI;

    // if off is correct, each symbol will have the same phase (modulo
    // the above hz correction), since each FFT bin holds an integer
    // number of cycles.

    // if off is too small, the phase is altered by the trailing part
    // of the previous symbol. if the previous tone was lower,
    // the phase won't have advanced as much as expected, and
    // this symbol's phase will be lower than the previous phase.
    // if the previous tone was higher, the phase will be more
    // advanced than expected. thus off too small leads to
    // a phase difference that's the reverse of the tone difference.

    // if off is too high, then the FFT started a little way into
    // this symbol, which causes the phase to be advanced a bit.
    // of course the previous symbol's phase was also advanced
    // too much. if this tone is higher than the previous symbol,
    // its phase will be more advanced than the previous. if
    // less, less.

    // the point: if successive phases and tone differences
    // are positively correlated, off is too high. if negatively,
    // too low.

    // fine_max_tone:
    // if late, ignore if a high tone, since ambiguous.
    // if early, ignore if prev is a high tone.

    if(sym[i] > sym[i-1]){
      if(d > 0 && sym[i] <= fine_max_tone){
        nlate++;
        late += d / std::abs(sym[i] - sym[i-1]);
      }
      if(d < 0 && sym[i-1] <= fine_max_tone){
        nearly++;
        early += fabs(d) / std::abs(sym[i] - sym[i-1]);
      }
    } else if(sym[i] < sym[i-1]){
      if(d > 0 && sym[i-1] <= fine_max_tone){
        nearly++;
        early += d / std::abs(sym[i] - sym[i-1]);
      }
      if(d < 0 && sym[i] <= fine_max_tone){
        nlate++;
        late += fabs(d) / std::abs(sym[i] - sym[i-1]);
      }
    }
  }


  if(nearly > 0)
    early /= nearly;
  if(nlate > 0)
    late /= nlate;

  //printf("early %d %.1f, late %d %.1f\n", nearly, early, nlate, late);

  // assumes 32 samples/symbol.
  if(nearly > 2*nlate){
    adj_off = round(32 * early / fine_thresh);
    if(adj_off > fine_max_off)
      adj_off = fine_max_off;
  } else if(nlate > 2*nearly){
    adj_off = 0 - round(32 * late / fine_thresh);
    if(fabs(adj_off) > fine_max_off)
      adj_off = -fine_max_off;
  }
}

//
// the signal is at roughly 25 hz in samples200.
// 
// return 2 if it decodes to a brand-new message.
// return 1 if it decodes but we've already seen it,
//   perhaps in a different pass.
// return 0 if we could not decode.
//
int
one_iter1(const std::vector<double> &samples200x,
          int best_off, double best_hz,
          double hz0_for_cb, double hz1_for_cb)
{
  // put best_hz in the middle of bin 4, at 25.0.
  std::vector<double> samples200 = shift200(samples200x, 0, samples200x.size(),
                                            best_hz);

  // mini 79x8 FFT.
  ffts_t m79 = extract(samples200, 25, best_off);

  // look at symbol-to-symbol phase change to try
  // to improve best_hz and best_off.
  if(do_fine_hz || do_fine_off){
    double adj_hz = 0;
    double adj_off = 0;
    fine(m79, 4, adj_hz, adj_off);
    if(do_fine_hz == 0)
      adj_hz = 0;
    if(do_fine_off == 0)
      adj_off = 0;
    if(fabs(adj_hz) < 6.25/4 && fabs(adj_off) < 4){
      best_hz += adj_hz;
      best_off += round(adj_off);
      if(best_off < 0)
        best_off = 0;
      samples200 = shift200(samples200x, 0, samples200x.size(), best_hz);
      m79 = extract(samples200, 25, best_off);
    }
  }

  double ll174[174];

  if(soft_ones){
    if(soft_ones == 1){
      soft_decode(m79, ll174);
    } else {
      c_soft_decode(m79, ll174);
    }
    int ret = try_decode(samples200, ll174, best_hz, best_off,
                         hz0_for_cb, hz1_for_cb, 1, "", m79);
    if(ret)
      return ret;
  }

  if(soft_pairs){
    double p174[174];
    soft_decode_pairs(m79, p174);
    int ret = try_decode(samples200, p174, best_hz, best_off,
                         hz0_for_cb, hz1_for_cb, 1, "", m79);
    if(ret)
      return ret;
    if(soft_ones == 0)
      memcpy(ll174, p174, sizeof(ll174));
  }

  if(soft_triples){
    double p174[174];
    soft_decode_triples(m79, p174);
    int ret = try_decode(samples200, p174, best_hz, best_off,
                         hz0_for_cb, hz1_for_cb, 1, "", m79);
    if(ret)
      return ret;
  }

  if(use_hints){
    for(int hi = 0; hi < (int)hints1_.size(); hi++){
      int h = hints1_[hi]; // 28-bit number, goes in ll174 0..28
      if(use_hints == 2 && h != 2){
        // just CQ
        continue;
      }
      double n174[174];
      for(int i = 0; i < 174; i++){
        if(i < 28){
          int bit = h & (1 << 27);
          if(bit){
            n174[i] = -4.97;
          } else {
            n174[i] = 4.97;
          }
          h <<= 1;
        } else {
          n174[i] = ll174[i];
        }
      }
      int ret = try_decode(samples200, n174, best_hz, best_off,
                           hz0_for_cb, hz1_for_cb, 0, "hint1", m79);
      if(ret){
        return ret;
      }
    }
  }
  
  if(use_hints == 1){
    for(int hi = 0; hi < (int)hints2_.size(); hi++){
      int h = hints2_[hi]; // 28-bit number, goes in ll174 29:29+28
      double n174[174];
      for(int i = 0; i < 174; i++){
        if(i >= 29 && i < 29+28){
          int bit = h & (1 << 27);
          if(bit){
            n174[i] = -4.97;
          } else {
            n174[i] = 4.97;
          }
          h <<= 1;
        } else {
          n174[i] = ll174[i];
        }
      }
      int ret = try_decode(samples200, n174, best_hz, best_off,
                           hz0_for_cb, hz1_for_cb, 0, "hint2", m79);
      if(ret){
        return ret;
      }
    }
  }

  return 0;
}

//
// subtract a corrected decoded signal from nsamples_,
// perhaps revealing a weaker signal underneath,
// to be decoded in a subsequent pass.
//
// re79[] holds the error-corrected symbol numbers.
//
void
subtract(const std::vector<int> re79,
         double hz0,
         double hz1,
         double off_sec)
{
  int block = blocksize(rate_);
  double bin_hz = rate_ / (double) block;
  int off0 = off_sec * rate_;

  double mhz = (hz0 + hz1) / 2.0;
  int bin0 = round(mhz / bin_hz);

  // move nsamples so that signal is centered in bin0.
  double diff0 = (bin0 * bin_hz) - hz0;
  double diff1 = (bin0 * bin_hz) - hz1;
  std::vector<double> moved = hilbert_shift(nsamples_, diff0, diff1, rate_);

  ffts_t bins = ffts(moved, off0, block, "subtract");

  if(bin0 + 8 > (int) bins[0].size())
    return;
  if((int) bins.size() < 79)
    return;

  std::vector<double> phases(79);
  std::vector<double> amps(79);
  for(int i = 0; i < 79; i++){
    int sym = bin0 + re79[i];
    std::complex<double> c = bins[i][sym];
    phases[i] = std::arg(c);

    // FFT multiplies magnitudes by number of bins,
    // or half the number of samples.
    amps[i] = std::abs(c) / (block / 2.0);
  }

  int ramp = round(block * subtract_ramp);
  if(ramp < 1)
    ramp = 1;

  // initial ramp part of first symbol.
  {
    int sym = bin0 + re79[0];
    double phase = phases[0];
    double amp = amps[0];
    double hz = 6.25 * sym;
    double dtheta = 2 * M_PI / (rate_ / hz); // advance per sample
    for(int jj = 0; jj < ramp; jj++){
      double theta = phase + jj * dtheta;
      double x = amp * cos(theta);
      x *= jj / (double) ramp;
      int iii = off0 + block*0 + jj;
      moved[iii] -= x;
    }
  }

  for(int si = 0; si < 79; si++){
    int sym = bin0 + re79[si];

    double phase = phases[si];
    double amp = amps[si];
    
    double hz = 6.25 * sym;
    double dtheta = 2 * M_PI / (rate_ / hz); // advance per sample

    // we've already done the first ramp for this symbol.
    // now for the steady part between ramps.
    for(int jj = ramp; jj < block-ramp; jj++){
      double theta = phase + jj * dtheta;
      double x = amp * cos(theta);
      int iii = off0 + block*si + jj;
      moved[iii] -= x;
    }

    // now the two ramps, from us to the next symbol.
    // we need to smoothly change the frequency,
    // approximating wsjt-x's gaussian frequency shift,
    // and also end up matching the next symbol's phase,
    // which is often different from this symbol due
    // to inaccuracies in hz or offset.

    // at start of this symbol's off-ramp.
    double theta = phase + (block-ramp) * dtheta;

    double hz1;
    double phase1;
    if(si+1 >= 79){
      hz1 = hz;
      phase1 = phase;
    } else {
      int sym1 = bin0 + re79[si+1];
      hz1 = 6.25 * sym1;
      phase1 = phases[si+1];
    }
    double dtheta1 = 2 * M_PI / (rate_ / hz1);

    // add this to dtheta for each sample, to gradually
    // change the frequency.
    double inc = (dtheta1 - dtheta) / (2.0 * ramp);

    // after we've applied all those inc's, what will the
    // phase be at the end of the next symbol's initial ramp,
    // if we don't do anything to correct it?
    double actual = theta + dtheta*2.0*ramp + inc*4.0*ramp*ramp/2.0;

    // what phase does the next symbol want to be at when
    // its on-ramp finishes?
    double target = phase1 + dtheta1*ramp;

    // ???
    while(fabs(target - actual) > M_PI){
      if(target < actual)
        target += 2*M_PI;
      else
        target -= 2*M_PI;
    }

    // adj is to be spread evenly over the off-ramp and on-ramp samples.
    double adj = target - actual;

    int end = block + ramp;
    if(si == 79-1)
      end = block;
    
    for(int jj = block-ramp; jj < end; jj++){
      int iii = off0 + block*si + jj;
      double x = amp * cos(theta);

      // trail off to zero at the very end.
      if(si == 79-1)
        x *= 1.0 - ((jj - (block - ramp)) / (double) ramp);
      
      moved[iii] -= x;

      theta += dtheta;
      dtheta += inc;
      theta += adj / (2.0 * ramp);
    }
  }

  nsamples_ = hilbert_shift(moved, -diff0, -diff1, rate_);
}

//
// decode, give to callback, and subtract.
//
// return 2 if it decodes to a brand-new message.
// return 1 if it decodes but we've already seen it,
//   perhaps in a different pass.
// return 0 if we could not decode.
//
int
try_decode(const std::vector<double> &samples200,
           double ll174[174],
           double best_hz, int best_off_samples, double hz0_for_cb, double hz1_for_cb,
           int use_osd, const char *comment1,
           const ffts_t &m79)
{
  int a174[174];
  std::string comment(comment1);

  if(decode(ll174, a174, use_osd, comment)){
    // a174 is corrected 91 bits of plain message plus 83 bits of LDPC parity.

    // how many of the corrected 174 bits match the received signal in ll174?
    int correct_bits = 0;
    for(int i = 0; i < 174; i++){
      if(ll174[i] < 0 && a174[i] == 1){
        correct_bits += 1;
      } else if(ll174[i] > 0 && a174[i] == 0){
        correct_bits += 1;
      }
    }

    // reconstruct correct 79 symbols from LDPC output.
    std::vector<int> re79 = recode(a174);

    if(do_third == 1){
      // fine-tune offset and hz for better subtraction.
      double best_off = best_off_samples / 200.0;
      search_both_known(samples200, 200, re79,
                        best_hz, best_off,
                        best_hz, best_off);
      best_off_samples = round(best_off * 200.0);
    }

    // convert starting sample # from 200 samples/second back to rate_.
    // also hz.
    double best_off = best_off_samples / 200.0; // convert to seconds
    best_hz = hz0_for_cb + (best_hz - 25.0);

    if(do_third == 2){
      // fine-tune offset and hz for better subtraction.
      search_both_known(samples_, rate_, re79,
                        best_hz, best_off,
                        best_hz, best_off);
    }

    double snr = guess_snr(m79);
    
    if(cb_ != 0){
      cb_mu_.lock();
      int ret = cb_(a174, best_hz + down_hz_, best_hz + down_hz_,
                    best_off, comment.c_str(), snr, pass_, correct_bits);
      cb_mu_.unlock();
      if(ret == 2){
        // a new decode. subtract it from nsamples_.
        subtract(re79, best_hz, best_hz, best_off);
      }

      return ret;
    }
    return 1;
  } else {
    return 0;
  }
}

//
// given 174 bits corrected by LDPC, work
// backwards to the symbols that must have
// been sent.
// used to help ensure that subtraction subtracts
// at the right place.
//
std::vector<int>
recode(int a174[])
{
  int i174 = 0;
  int costas[] = { 3, 1, 4, 0, 6, 5, 2 };
  std::vector<int> out79;
  for(int i79 = 0; i79 < 79; i79++){
    if(i79 < 7){
      out79.push_back(costas[i79]);
    } else if(i79 >= 36 && i79 < 36+7){
      out79.push_back(costas[i79-36]);
    } else if(i79 >= 72){
      out79.push_back(costas[i79-72]);
    } else {
      int sym = (a174[i174+0] << 2) | (a174[i174+1] << 1) | (a174[i174+2] << 0);
      i174 += 3;
      // gray code
      int map[] = { 0, 1, 3, 2, 5, 6, 4, 7 };
      sym = map[sym];
      out79.push_back(sym);
    }
  }
  assert(out79.size() == 79);
  assert(i174 == 174);
  return out79;
}

};

std::mutex FT8::cb_mu_;

//
// Python calls these.
//
void
entry(double xsamples[], int nsamples, int start, int rate,
      double min_hz,
      double max_hz,
      int hints1[],
      int hints2[],
      double time_left, double total_time_left, cb_t cb,
      int nprevdecs,
      struct cdecode *xprevdecs)
{
  double t0 = now();
  double deadline = t0 + time_left;
  double final_deadline = t0 + total_time_left;

  // decodes from previous runs, for subtraction.
  std::vector<cdecode> prevdecs;
  for(int i = 0; i < nprevdecs; i++){
    prevdecs.push_back(xprevdecs[i]);
  }

  std::vector<double> samples(nsamples);
  for(int i = 0; i < nsamples; i++){
    samples[i] = xsamples[i];
  }

  if(min_hz < 0){
    min_hz = 0;
  }
  if(max_hz > rate/2){
    max_hz = rate/2;
  }
  double per = (max_hz - min_hz) / nthreads;

  std::vector<FT8 *> thv;

  for(int i = 0; i < nthreads; i++){
    double hz0 = min_hz + i * per;
    if(i > 0 || overlap_edges)
      hz0 -= overlap;
    
    double hz1 = min_hz + (i + 1) * per;
    if(i != nthreads-1 || overlap_edges)
      hz1 += overlap;

    hz0 = std::max(hz0, 0.0);
    hz1 = std::min(hz1, (rate / 2.0) - 50);

    FT8 *ft8 = new FT8(samples,
                       hz0, hz1,
                       start, rate,
                       hints1, hints2,
                       deadline, final_deadline, cb,
                       prevdecs);

    int npasses = nprevdecs > 0 ? npasses_two : npasses_one;

    ft8->th_ = new std::thread( [ ft8, npasses ] () { ft8->go(npasses); } );

    thv.push_back(ft8);
  }

  for(int i = 0; i < (int) thv.size(); i++){
    thv[i]->th_->join();
    delete thv[i]->th_;
    delete thv[i];
  }
}

double
set(char *param, char *val)
{
  struct sss {
    const char *name;
    void *addr;
    int type; // 0 int, 1 double
  };
  struct sss params[] =
    {
     { "snr_win", &snr_win, 0 },
     { "snr_how", &snr_how, 0 },
     { "ldpc_iters", &ldpc_iters, 0 },
     { "shoulder200", &shoulder200, 1 },
     { "shoulder200_extra", &shoulder200_extra, 1 },
     { "second_hz_n", &second_hz_n, 0 },
     { "second_hz_win", &second_hz_win, 1 },
     { "second_off_n", &second_off_n, 0 },
     { "second_off_win", &second_off_win, 1 },
     { "third_hz_n", &third_hz_n, 0 },
     { "third_hz_win", &third_hz_win, 1 },
     { "third_off_n", &third_off_n, 0 },
     { "third_off_win", &third_off_win, 1 },
     { "log_tail", &log_tail, 1 },
     { "log_rate", &log_rate, 1 },
     { "problt_how_noise", &problt_how_noise, 0 },
     { "problt_how_sig", &problt_how_sig, 0 },
     { "use_apriori", &use_apriori, 0 },
     { "use_hints", &use_hints, 0 },
     { "win_type", &win_type, 0 },
     { "osd_depth", &osd_depth, 0 },
     { "ncoarse", &ncoarse, 0 },
     { "ncoarse_blocks", &ncoarse_blocks, 0 },
     { "tminus", &tminus, 1 },
     { "tplus", &tplus, 1 },
     { "coarse_off_n", &coarse_off_n, 0 },
     { "coarse_hz_n", &coarse_hz_n, 0 },
     { "already_hz", &already_hz, 1 },
     { "nthreads", &nthreads, 0 },
     { "npasses_one", &npasses_one, 0 },
     { "npasses_two", &npasses_two, 0 },
     { "overlap", &overlap, 1 },
     { "nyquist", &nyquist, 1 },
     { "oddrate", &oddrate, 0 },
     { "osd_ldpc_thresh", &osd_ldpc_thresh, 0 },
     { "pass0_frac", &pass0_frac, 1 },
     { "go_extra", &go_extra, 1 },
     { "reduce_how", &reduce_how, 0 },
     { "do_reduce", &do_reduce, 0 },
     { "pass_threshold", &pass_threshold, 0 },
     { "strength_how", &strength_how, 0 },
     { "known_strength_how", &known_strength_how, 0 },
     { "reduce_shoulder", &reduce_shoulder, 1 },
     { "reduce_factor", &reduce_factor, 1 },
     { "reduce_extra", &reduce_extra, 1 },
     { "overlap_edges", &overlap_edges, 0 },
     { "coarse_strength_how", &coarse_strength_how, 0 },
     { "coarse_all", &coarse_all, 1 },
     { "second_count", &second_count, 0 },
     { "fftw_type", &fftw_type, 0 },
     { "soft_phase_win", &soft_phase_win, 0 },
     { "subtract_ramp", &subtract_ramp, 1 },
     { "soft_pairs", &soft_pairs, 0 },
     { "soft_triples", &soft_triples, 0 },
     { "do_second", &do_second, 0 },
     { "do_fine_hz", &do_fine_hz, 0 },
     { "do_fine_off", &do_fine_off, 0 },
     { "do_third", &do_third, 0 },
     { "fine_thresh", &fine_thresh, 1 },
     { "fine_max_off", &fine_max_off, 0 },
     { "fine_max_tone", &fine_max_tone, 0 },
     { "known_sparse", &known_sparse, 0 },
     { "soft_ones", &soft_ones, 0 },
     { "c_soft_weight", &c_soft_weight, 1 },
     { "c_soft_win", &c_soft_win, 0 },
     { "bayes_how", &bayes_how, 0 },
    };
  int nparams = sizeof(params) / sizeof(params[0]);

  for(int i = 0; i < nparams; i++){
    if(strcmp(param, params[i].name) == 0){
      if(val[0]){
        if(params[i].type == 0){
          *(int*)params[i].addr = round(atof(val));
        } else if(params[i].type == 1){
          *(double*)params[i].addr = atof(val);
        } else {
          assert(0);
        }
      }
      if(params[i].type == 0){
        return *(int*)params[i].addr;
      } else if(params[i].type == 1){
        return *(double*)params[i].addr;
      } else {
        fprintf(stderr, "weird type %d\n", params[i].type);
        return 0;
      }
    }
  }
  fprintf(stderr, "ft8.cc set(%s, %s) unknown parameter\n", param, val);
  exit(1);
  return 0;
}
