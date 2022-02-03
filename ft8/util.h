#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <complex>

double now();
void writewav(const std::vector<double> &samples, const char *filename, int rate);
std::vector<double> readwav(const char *filename, int &rate_out);
void writetxt(std::vector<double> v, const char *filename);
std::complex<double> goertzel(std::vector<double> v, int rate, int i0, int n, double hz);
double vmax(const std::vector<double> &v);
std::vector<double> vreal(const std::vector<std::complex<double>> &a);
std::vector<double> vimag(const std::vector<std::complex<double>> &a);
std::vector<std::complex<double>> gfsk_c(const std::vector<int> &symbols,
                                         double hz0, double hz1,
                                         double spacing, int rate, int symsamples,
                                         double phase0,
                                         const std::vector<double> &gwin);
std::vector<double> gfsk_r(const std::vector<int> &symbols,
                           double hz0, double hz1,
                           double spacing, int rate, int symsamples,
                           double phase0,
                           const std::vector<double> &gwin);
std::vector<double> gfsk_window(int samples_per_symbol, double b);

typedef unsigned long ulong;
typedef unsigned int uint;

#endif
