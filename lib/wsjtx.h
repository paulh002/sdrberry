#pragma once
typedef size_t fortran_charlen_t;

extern "C" {
//----------------------------------------------------- C and Fortran routines
void symspec_(struct dec_data *, int *k, double *trperiod, int *nsps, int *ingain,
			  bool *bLowSidelobes, int *minw, float *px, float s[], float *df3,
			  int *nhsym, int *npts8, float *m_pxmax, int *npct);

void hspec_(short int d2[], int *k, int *nutc0, int *ntrperiod, int *nrxfreq, int *ntol,
			bool *bmsk144, bool *btrain, double const pcoeffs[], int *ingain,
			char mycall[], char hiscall[], bool *bshmsg, bool *bswl, char ddir[], float green[],
			float s[], int *jh, float *pxmax, float *rmsNoGain, char line[],
			fortran_charlen_t, fortran_charlen_t, fortran_charlen_t, fortran_charlen_t);

void genft8_(char *msg, int *i3, int *n3, char *msgsent, char ft8msgbits[],
			 int itone[], fortran_charlen_t, fortran_charlen_t);

void genft4_(char *msg, int *ichk, char *msgsent, char ft4msgbits[], int itone[],
			 fortran_charlen_t, fortran_charlen_t);

void genfst4_(char *msg, int *ichk, char *msgsent, char fst4msgbits[],
			  int itone[], int *iwspr, fortran_charlen_t, fortran_charlen_t);

void gen_ft8wave_(int itone[], int *nsym, int *nsps, float *bt, float *fsample, float *f0,
				  float xjunk[], float wave[], int *icmplx, int *nwave);

void gen_ft4wave_(int itone[], int *nsym, int *nsps, float *fsample, float *f0,
				  float xjunk[], float wave[], int *icmplx, int *nwave);

void gen_fst4wave_(int itone[], int *nsym, int *nsps, int *nwave, float *fsample,
				   int *hmod, float *f0, int *icmplx, float xjunk[], float wave[]);

void genwave_(int itone[], int *nsym, int *nsps, int *nwave, float *fsample,
			  int *hmod, float *f0, int *icmplx, float xjunk[], float wave[]);

void gen4_(char *msg, int *ichk, char *msgsent, int itone[],
		   int *itext, fortran_charlen_t, fortran_charlen_t);

void gen9_(char *msg, int *ichk, char *msgsent, int itone[],
		   int *itext, fortran_charlen_t, fortran_charlen_t);

void genmsk_128_90_(char *msg, int *ichk, char *msgsent, int itone[], int *itype,
					fortran_charlen_t, fortran_charlen_t);

void gen65_(char *msg, int *ichk, char *msgsent, int itone[],
			int *itext, fortran_charlen_t, fortran_charlen_t);

void genq65_(char *msg, int *ichk, char *msgsent, int itone[],
			 int *i3, int *n3, fortran_charlen_t, fortran_charlen_t);

void genwspr_(char *msg, char *msgsent, int itone[], fortran_charlen_t, fortran_charlen_t);

void azdist_(char *MyGrid, char *HisGrid, double *utch, int *nAz, int *nEl,
			 int *nDmiles, int *nDkm, int *nHotAz, int *nHotABetter,
			 fortran_charlen_t, fortran_charlen_t);

void morse_(char *msg, int *icw, int *ncw, fortran_charlen_t);

int ptt_(int nport, int ntx, int *iptt, int *nopen);

void wspr_downsample_(short int d2[], int *k);

int savec2_(char *fname, int *TR_seconds, double *dial_freq, fortran_charlen_t);

void avecho_(short id2[], int *dop, int *nfrit, int *nqual, float *f1,
			 float *level, float *sigdb, float *snr, float *dfreq,
			 float *width);

void fast_decode_(short id2[], int narg[], double *trperiod,
				  char msg[], char mycall[], char hiscall[],
				  fortran_charlen_t, fortran_charlen_t, fortran_charlen_t);
void degrade_snr_(short d2[], int *n, float *db, float *bandwidth);

void wav12_(short d2[], short d1[], int *nbytes, short *nbitsam2);

void refspectrum_(short int d2[], bool *bclearrefspec,
				  bool *brefspec, bool *buseref, const char *c_fname, fortran_charlen_t);

void freqcal_(short d2[], int *k, int *nkhz, int *noffset, int *ntol,
			  char line[], fortran_charlen_t);

void fix_contest_msg_(char *MyGrid, char *msg, fortran_charlen_t, fortran_charlen_t);

void calibrate_(char data_dir[], int *iz, double *a, double *b, double *rms,
				double *sigmaa, double *sigmab, int *irc, fortran_charlen_t);

void foxgen_();

void plotsave_(float swide[], int *m_w, int *m_h1, int *irow);

void chkcall_(char *w, char *basc_call, bool cok, int len1, int len2);

void get_ft4msg_(int *idecode, char *line, int len);

void chk_samples_(int *m_ihsym, int *k, int *m_hsymStop);

void __packjt77_MOD_unpack77(char *c77, int *nrx, char *msg, int *unpk77_success, fortran_charlen_t, fortran_charlen_t);
void __packjt77_MOD_pack77(char *msg, int *i3, int *n3, char *c77, fortran_charlen_t, fortran_charlen_t);
}