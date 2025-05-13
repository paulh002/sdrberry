#pragma once
#include "i2cgeneric.h"
class i2coutput :
    public i2cgeneric
{
  public:
	void set_rxtx(bool tx);
	void clear();
};

