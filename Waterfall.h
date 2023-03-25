#pragma once
#include "lvgl.h"
#include "sma.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <mutex>
#include <vector>

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

class Waterfall
{
};