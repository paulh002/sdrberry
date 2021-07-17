#include <cmath>
#include <iostream>
#include <csignal>
#include <complex>
#include "FMDemodulator.h"

#define PI 3.14159265359
#define INV2PI 0.15915494309

const float coeff_1 = PI / 4, coeff_2 = 3 * coeff_1;  //For Atan2, Pi/4 and 3Pi/4

//-----------------------------------------------
// Fast arctan2: Jim Shima, 1999/04/23
float arctan2(float y, float x)
{
	float abs_y, angle, r;
	abs_y = fabs(y) + 1e-10;       // kludge to prevent 0/0 condition
	if(x >= 0)
	{
		r = (x - abs_y) / (x + abs_y);
		angle = coeff_1 - coeff_1 * r;
	}
	else
	{
		r = (x + abs_y) / (abs_y - x);
		angle = coeff_2 - coeff_1 * r;
	}
	if (y < 0)
		return (-angle);     // negate if in quad III or IV
		else
		return(angle);
}