#include "fft.h"
#include <mutex>
#include <unistd.h>
#include <assert.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util.h"

// MEASURE=0, ESTIMATE=64, PATIENT=32
int fftw_type = FFTW_ESTIMATE;


#define TIMING 0

// a cached fftw plan, for both of:
// fftw_plan_dft_r2c_1d(n, m_in, m_out, FFTW_ESTIMATE);
// fftw_plan_dft_c2r_1d(n, m_in, m_out, FFTW_ESTIMATE);
class Plan {
public:
  int n_;
  int type_;

  //
  // real -> complex
  //
  fftw_complex *c_; // (n_ / 2) + 1 of these
  double *r_; // n_ of these
  fftw_plan fwd_; // forward plan
  fftw_plan rev_; // reverse plan

  //
  // complex -> complex
  //
  fftw_complex *cc1_; // n
  fftw_complex *cc2_; // n
  fftw_plan cfwd_; // forward plan
  fftw_plan crev_; // reverse plan

  // how much CPU time spent in FFTs that use this plan.
#if TIMING
  double time_;
#endif
  const char *why_;
  int uses_;
};

static std::mutex plansmu;
static Plan *plans[1000];
static int nplans;
static int plan_master_pid = 0;

Plan *
get_plan(int n, const char *why)
{
  // cache fftw plans in the parent process,
  // so they will already be there for fork()ed children.

  plansmu.lock();
  
  if(plan_master_pid == 0){
    plan_master_pid = getpid();
  }
    
  for(int i = 0; i < nplans; i++){
    if(plans[i]->n_ == n
       && plans[i]->type_ == fftw_type
#if TIMING
       && strcmp(plans[i]->why_, why) == 0
#endif
       ){
      Plan *p = plans[i];
      p->uses_ += 1;
      plansmu.unlock();
      return p;
    }
  }

  double t0 = now();

  // fftw_make_planner_thread_safe();

  // the fftw planner is not thread-safe.
  // can't rely on plansmu because both ft8.so
  // and snd.so may be using separate copies of fft.cc.
  // the lock file really should be per process.
  int lockfd = creat("/tmp/fft-plan-lock", 0666);
  assert(lockfd >= 0);
  fchmod(lockfd, 0666);
  int lockret = flock(lockfd, LOCK_EX);
  assert(lockret == 0);

  fftw_set_timelimit(5);

  //
  // real -> complex
  //

  Plan *p = new Plan;
  
  p->n_ = n;
#if TIMING
  p->time_ = 0;
#endif
  p->uses_ = 1;
  p->why_ = why;
  p->r_ = (double*) fftw_malloc(n * sizeof(double));
  assert(p->r_);
  p->c_ = (fftw_complex*) fftw_malloc(((n/2)+1) * sizeof(fftw_complex));
  assert(p->c_);
  
  // FFTW_ESTIMATE
  // FFTW_MEASURE
  // FFTW_PATIENT
  // FFTW_EXHAUSTIVE
  int type = fftw_type;
  if(getpid() != plan_master_pid){
    type = FFTW_ESTIMATE;
  }
  p->type_ = type;
  p->fwd_ = fftw_plan_dft_r2c_1d(n, p->r_, p->c_, type);
  assert(p->fwd_);
  p->rev_ = fftw_plan_dft_c2r_1d(n, p->c_, p->r_, type);
  assert(p->rev_);
  
  //
  // complex -> complex
  //
  p->cc1_ = (fftw_complex*) fftw_malloc(n * sizeof(fftw_complex));
  assert(p->cc1_);
  p->cc2_ = (fftw_complex*) fftw_malloc(n * sizeof(fftw_complex));
  assert(p->cc2_);
  p->cfwd_ = fftw_plan_dft_1d(n, p->cc1_, p->cc2_, FFTW_FORWARD, type);
  assert(p->cfwd_);
  p->crev_ = fftw_plan_dft_1d(n, p->cc2_, p->cc1_, FFTW_BACKWARD, type);
  assert(p->crev_);

  flock(lockfd, LOCK_UN);
  close(lockfd);

  assert(nplans+1 < 1000);
  
  plans[nplans] = p;
  __sync_synchronize();
  nplans += 1;

  if(0 && getpid() == plan_master_pid){
    double t1 = now();
    fprintf(stderr, "miss pid=%d master=%d n=%d t=%.3f total=%d type=%d, %s\n",
            getpid(), plan_master_pid, n, t1 - t0, nplans, type, why);
  }

  plansmu.unlock();

  return p;
}

//
// do just one FFT on samples[i0..i0+block]
// real inputs, complex outputs.
// output has (block / 2) + 1 points.
//
std::vector<std::complex<double>>
one_fft(const std::vector<double> &samples, int i0, int block,
        const char *why, Plan *p)
{
  assert(i0 >= 0);
  assert(block > 1);
  
  int nsamples = samples.size();
  int nbins = (block / 2) + 1;

  if(p){
    assert(p->n_ == block);
    p->uses_ += 1;
  } else {
    p = get_plan(block, why);
  }
  fftw_plan m_plan = p->fwd_;

#if TIMING
  double t0 = now();
#endif

  assert((int) samples.size() - i0 >= block);

  int m_in_allocated = 0;
  double *m_in = (double*) samples.data() + i0;

  if((((unsigned long long)m_in) % 16) != 0){
    // m_in must be on a 16-byte boundary for FFTW.
    m_in = (double *) fftw_malloc(sizeof(double) * p->n_);
    assert(m_in);
    m_in_allocated = 1;
    for(int i = 0; i < block; i++){
      if(i0 + i < nsamples){
        m_in[i] = samples[i0 + i];
      } else {
        m_in[i] = 0;
      }
    }
  }

  fftw_complex *m_out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) *
                                                     ((p->n_ / 2) + 1));
  assert(m_out);

  fftw_execute_dft_r2c(m_plan, m_in, m_out);

  std::vector<std::complex<double>> out(nbins);

  for(int bi = 0; bi < nbins; bi++){
    double re = m_out[bi][0];
    double im = m_out[bi][1];
    out[bi] = std::complex<double>(re, im);
  }

  if(m_in_allocated)
    fftw_free(m_in);
  fftw_free(m_out);

#if TIMING
  p->time_ += now() - t0;
#endif

  return out;
}

//
// do a full set of FFTs, one per symbol-time.
// bins[time][frequency]
//
ffts_t
ffts(const std::vector<double> &samples, int i0, int block, const char *why)
{
  assert(i0 >= 0);
  assert(block > 1 && (block % 2) == 0);
  
  int nsamples = samples.size();
  int nbins = (block / 2) + 1;
  int nblocks = (nsamples - i0) / block;
  ffts_t bins(nblocks);
  for(int si = 0; si < nblocks; si++){
    bins[si].resize(nbins);
  }

  Plan *p = get_plan(block, why);
  fftw_plan m_plan = p->fwd_;

#if TIMING
  double t0 = now();
#endif

  // allocate our own b/c using p->m_in and p->m_out isn't thread-safe.
  double *m_in = (double *) fftw_malloc(sizeof(double) * p->n_);
  fftw_complex *m_out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) *
                                                     ((p->n_ / 2) + 1));
  assert(m_in && m_out);

  // double *m_in = p->r_;
  // fftw_complex *m_out = p->c_;

  for(int si = 0; si < nblocks; si++){
    int off = i0 + si * block;
    for(int i = 0; i < block; i++){
      if(off + i < nsamples){
        double x = samples[off + i];
        m_in[i] = x;
      } else {
        m_in[i] = 0;
      }
    }

    fftw_execute_dft_r2c(m_plan, m_in, m_out);

    for(int bi = 0; bi < nbins; bi++){
      double re = m_out[bi][0];
      double im = m_out[bi][1];
      std::complex<double> c(re, im);
      bins[si][bi] = c;
    }
  }

  fftw_free(m_in);
  fftw_free(m_out);

#if TIMING
  p->time_ += now() - t0;
#endif

  return bins;
}

//
// do just one FFT on samples[i0..i0+block]
// real inputs, complex outputs.
// output has block points.
//
std::vector<std::complex<double>>
one_fft_c(const std::vector<double> &samples, int i0, int block, const char *why)
{
  assert(i0 >= 0);
  assert(block > 1);
  
  int nsamples = samples.size();

  Plan *p = get_plan(block, why);
  fftw_plan m_plan = p->cfwd_;

#if TIMING
  double t0 = now();
#endif

  fftw_complex *m_in  = (fftw_complex*) fftw_malloc(block * sizeof(fftw_complex));
  fftw_complex *m_out = (fftw_complex*) fftw_malloc(block * sizeof(fftw_complex));
  assert(m_in && m_out);

  for(int i = 0; i < block; i++){
    if(i0 + i < nsamples){
      m_in[i][0] = samples[i0 + i]; // real
    } else {
      m_in[i][0] = 0;
    }
    m_in[i][1] = 0; // imaginary
  }

  fftw_execute_dft(m_plan, m_in, m_out);

  std::vector<std::complex<double>> out(block);

  double norm = 1.0 / sqrt(block);
  for(int bi = 0; bi < block; bi++){
    double re = m_out[bi][0];
    double im = m_out[bi][1];
    std::complex<double> c(re, im);
    c *= norm;
    out[bi] = c;
  }
    
  fftw_free(m_in);
  fftw_free(m_out);

#if TIMING
  p->time_ += now() - t0;
#endif

  return out;
}

std::vector<std::complex<double>>
one_fft_cc(const std::vector<std::complex<double>> &samples, int i0, int block, const char *why)
{
  assert(i0 >= 0);
  assert(block > 1);
  
  int nsamples = samples.size();

  Plan *p = get_plan(block, why);
  fftw_plan m_plan = p->cfwd_;

#if TIMING
  double t0 = now();
#endif

  fftw_complex *m_in  = (fftw_complex*) fftw_malloc(block * sizeof(fftw_complex));
  fftw_complex *m_out = (fftw_complex*) fftw_malloc(block * sizeof(fftw_complex));
  assert(m_in && m_out);

  for(int i = 0; i < block; i++){
    if(i0 + i < nsamples){
      m_in[i][0] = samples[i0 + i].real();
      m_in[i][1] = samples[i0 + i].imag();
    } else {
      m_in[i][0] = 0;
      m_in[i][1] = 0;
    }
  }

  fftw_execute_dft(m_plan, m_in, m_out);

  std::vector<std::complex<double>> out(block);

  //double norm = 1.0 / sqrt(block);
  for(int bi = 0; bi < block; bi++){
    double re = m_out[bi][0];
    double im = m_out[bi][1];
    std::complex<double> c(re, im);
    //c *= norm;
    out[bi] = c;
  }
    
  fftw_free(m_in);
  fftw_free(m_out);

#if TIMING
  p->time_ += now() - t0;
#endif

  return out;
}

std::vector<std::complex<double>>
one_ifft_cc(const std::vector<std::complex<double>> &bins, const char *why)
{
  int block = bins.size();

  Plan *p = get_plan(block, why);
  fftw_plan m_plan = p->crev_;

#if TIMING
  double t0 = now();
#endif

  fftw_complex *m_in = (fftw_complex*) fftw_malloc(block * sizeof(fftw_complex));
  fftw_complex *m_out = (fftw_complex *) fftw_malloc(block * sizeof(fftw_complex));
  assert(m_in && m_out);

  for(int bi = 0; bi < block; bi++){
    double re = bins[bi].real();
    double im = bins[bi].imag();
    m_in[bi][0] = re;
    m_in[bi][1] = im;
  }

  fftw_execute_dft(m_plan, m_in, m_out);

  std::vector<std::complex<double>> out(block);
  double norm = 1.0 / sqrt(block);
  for(int i = 0; i < block; i++){
    double re = m_out[i][0];
    double im = m_out[i][1];
    std::complex<double> c(re, im);
    c *= norm;
    out[i] = c;
  }

  fftw_free(m_in);
  fftw_free(m_out);

#if TIMING
  p->time_ += now() - t0;
#endif

  return out;
}

std::vector<double>
one_ifft(const std::vector<std::complex<double>> &bins, const char *why)
{
  int nbins = bins.size();
  int block = (nbins - 1) * 2;

  Plan *p = get_plan(block, why);
  fftw_plan m_plan = p->rev_;

#if TIMING
  double t0 = now();
#endif

  fftw_complex *m_in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) *
                                                    ((p->n_ / 2) + 1));
  double *m_out = (double *) fftw_malloc(sizeof(double) * p->n_);

  for(int bi = 0; bi < nbins; bi++){
    double re = bins[bi].real();
    double im = bins[bi].imag();
    m_in[bi][0] = re;
    m_in[bi][1] = im;
  }

  fftw_execute_dft_c2r(m_plan, m_in, m_out);

  std::vector<double> out(block);
  for(int i = 0; i < block; i++){
    out[i] = m_out[i];
  }

  fftw_free(m_in);
  fftw_free(m_out);

#if TIMING
  p->time_ += now() - t0;
#endif

  return out;
}

//
// return the analytic signal for signal x,
// just like scipy.signal.hilbert(), from which
// this code is copied.
//
// the return value is x + iy, where y is the hilbert transform of x.
//
std::vector<std::complex<double>>
analytic(const std::vector<double> &x, const char *why)
{
  ulong n = x.size();

  std::vector<std::complex<double>> y = one_fft_c(x, 0, n, why);
  assert(y.size() == n);

  // leave y[0] alone.
  // double the first (positive) half of the spectrum.
  // zero out the second (negative) half of the spectrum.
  // y[n/2] is the nyquist bucket if n is even; leave it alone.
  if((n % 2) == 0){
    for(ulong i = 1; i < n/2; i++)
      y[i] *= 2;
    for(ulong i = n/2+1; i < n; i++)
      y[i] = 0;
  } else {
    for(ulong i = 1; i < (n+1)/2; i++)
      y[i] *= 2;
    for(ulong i = (n+1)/2; i < n; i++)
      y[i] = 0;
  }
      
  std::vector<std::complex<double>> z = one_ifft_cc(y, why);

  return z;
}

//
// general-purpose shift x in frequency by hz.
// uses hilbert transform to avoid sidebands.
// but it does wrap around at 0 hz and the nyquist frequency.
//
// note analytic() does an FFT over the whole signal, which
// is expensive, and often re-used, but it turns out it
// isn't a big factor in overall run-time.
//
// like weakutil.py's freq_shift().
//
std::vector<double>
hilbert_shift(const std::vector<double> &x, double hz0, double hz1, int rate)
{
  // y = scipy.signal.hilbert(x)
  std::vector<std::complex<double>> y = analytic(x, "hilbert_shift");
  assert(y.size() == x.size());

  double dt = 1.0 / rate;
  int n = x.size();

  std::vector<double> ret(n);
  
  for(int i = 0; i < n; i++){
    // complex "local oscillator" at hz.
    double hz = hz0 + (i / (double)n) * (hz1 - hz0);
    std::complex<double> lo = std::exp(std::complex<double>(0.0, 2 * M_PI * hz * dt * i));
    ret[i] = (lo * y[i]).real();
  }

  return ret;
}

void
fft_stats()
{
  for(int i = 0; i < nplans; i++){
    Plan *p = plans[i];
    printf("%-13s %6d %9d %6.3f\n",
           p->why_,
           p->n_,
           p->uses_,
#if TIMING
           p->time_
#else
           0.0
#endif
           );
  }
}
