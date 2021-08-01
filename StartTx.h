#pragma once
class StartTx
{
public:
	void start_tranceiver();


private:
	
	double  ifrate  = 0.53e6;  //1.0e6;//
	bool    stereo  = true;
	int     pcmrate = 48000;	
};

