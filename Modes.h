#pragma once
#include<string>

const int mode_lsb = 1;
const int mode_usb = 2;
const int mode_am = 3;
const int mode_dsb = 4;
const int mode_cw = 5;
const int mode_ft8 = 6;
const int mode_ft4 = 7;
const int mode_broadband_fm = 8;
const int mode_narrowband_fm = 9;
const int mode_rtty = 10;
const int mode_echo = 11;
const int mode_freedv = 12;
const int mode_wspr = 13;
const int mode_tune = 14;

enum tranceiverMode
{
	modelsb,
	modeusb,
	modeam,
	modedsb,
	modecw,
	modeft8,
	modeft4,
	modebroadband_fm,
	modenarrowband_fm,
	modertty,
	modeecho,
	modefreedv,
	modewspr
};

extern std::string mode_string(int mode);
