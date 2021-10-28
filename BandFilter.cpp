#include "BandFilter.h"

void BandFilter::initFilter()
{
	connected = false;
	for (auto& col : Settings_file.address)
	{
		PCF8574 i2cdevice(col);
		if (i2cdevice.begin(0))
		{
			pcf8574.push_back(i2cdevice);
			connected = true;
		}
		else
		{
			connected = false;
		}
	}
}

void BandFilter::SetBand(int band, bool rx)
{
	int index = vfo.getBandIndex(band);
	int i = pcf8574.size();
	if (i > 0 && connected)
	{
		int ii = 0;
		for (auto& col : pcf8574)
		{
			uint8_t cc = 0;
			if (rx)
				cc = Settings_file.command_rx[i*index + ii];
			else
				cc = Settings_file.command_tx[i*index + ii];
			col.write8(cc);
			ii++;
		}		
		return;
	}
}