#include "BandFilter.h"

void BandFilter::initFilter()
{
	for (auto& col : Settings_file.address)
	{
		PCF8574 i2cdevice(col);
		if (i2cdevice.begin(0))
		{
			pcf8574.push_back(i2cdevice);
			printf("Connected to %d \n", (int)i2cdevice.getAddress());
		}
		else
		{
			pcf8574.push_back(i2cdevice);
			printf("Cannot connect to %d \n", (int)i2cdevice.getAddress());
		}
	}
}

void BandFilter::SetBand(int band, bool rx)
{
	int index = vfo.getBandIndex(band);
	int i = pcf8574.size();
	if (i > 0 && !bandfilter_pass_trough)
	{
		int ii = 0;
		printf("i2c ");
		for (auto& col : pcf8574)
		{
			uint8_t cc = 0;
			if (rx && Settings_file.command_rx.size() >= (i * index + ii))
			{
				cc = Settings_file.command_rx[i * index + ii];
				if (col.getConnected())
					col.write8(cc);
				printf("%d ", cc);
			}
			if (!rx && Settings_file.command_tx.size() >= (i * index + ii))
			{
				cc = Settings_file.command_tx[i * index + ii];
				if (col.getConnected())
					col.write8(cc);
				printf("%d ", cc);
			}
			ii++;
		}
		printf("\n");
		return;
	}
	if (i > 0 && bandfilter_pass_trough)
	{
		int ii = 0;
		printf("i2c ");
		for (auto &col : pcf8574)
		{
			uint8_t cc = 0;
			if (rx && Settings_file.passthrough_rx.size() >= i)
			{
				cc = Settings_file.passthrough_rx[ii];
				if (col.getConnected())
					col.write8(cc);
				printf("%d ", cc);
			}

			if (!rx && Settings_file.passthrough_tx.size() >= i)
			{
				cc = Settings_file.passthrough_tx[ii];
				if (col.getConnected())
					col.write8(cc);
				printf("%d ", cc);
			}
			ii++;
		}
		printf("\n");
	}
}

void BandFilter::Setpasstrough(bool b)
{
	bandfilter_pass_trough = b;
	SetBand(vfo.get_band_in_meters(), vfo.get_rx());
}