#ifndef ft8_h
#define ft8_h

extern "C" {
  // let Python call just entry(), rather than a C++ mangled name.
  typedef int (*cb_t)(int *a91, double hz0, double hz1, double off,
                      const char *, double snr, int pass,
                      int correct_bits);
  // same as Python class CDECODE 
  //
  struct cdecode {
    double hz0;
    double hz1;
    double off;
    int *bits; // 174
  };
  void entry(double xsamples[], int nsamples, int start, int rate,
             double min_hz,
             double max_hz,
             int hints1[], int hints2[], double time_left,
             double total_time_left, cb_t cb,
             int, struct cdecode *);
  double set(char *param, char *val);
}

#endif
