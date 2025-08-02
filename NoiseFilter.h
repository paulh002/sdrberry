#pragma once


class NoiseFilter
{
  private:
	std::unique_ptr<FastFourier> fft;
	
  public:
	NoiseFilter();
	void Process(IQSampleVector &filter_in, IQSampleVector &filter_out);
};

