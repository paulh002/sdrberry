#include "i2coutput.h"


void i2coutput::set_rxtx(bool tx)
{
	if (isconnected)
	{
		if (tx)
			write_pin(0, 1);
		else
			write_pin(0, 0);
	}
}

void i2coutput::clear()
{
	if (isconnected)
	{
		write(0);
	}
}