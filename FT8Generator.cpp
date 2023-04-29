#include "FT8Generator.h"
#include "wsjtx.h"
#include <regex>

/*R"(
        ^\s*                                      # optional leading spaces
        ( [A-Z]{0,2} | [A-Z][0-9] | [0-9][A-Z] )  # part 1
        ( [0-9][A-Z]{0,3} )                       # part 2
        (/R | /P)?                                # optional suffix
        \s*$                                      # optional trailing spaces
*/
bool stdCall(std::string const &w)
{
	std::regex standard_call_re{R"(^\s*([A-Z]{0,2}|[A-Z][0-9]|[0-9][A-Z])([0-9][A-Z]{0,3})(/R|/P)?\s*$)", std::regex::icase}; 
	return std::regex_match(w, standard_call_re);
}

vector<float> FT8Generator::generate(int frequency, encoding code, std::string message)
{
	bool is_ft4{false};
	vector<float> signal;
	int nsym = (is_ft4) ? FT4_NN : FT8_NN;
	
	int i3 = 0;
	int n3 = 0;
	char ft8msgbits[77];
	std::string msgsent;

	message.resize(38);
	msgsent.resize(38);
	genft8_((char *)message.c_str(), &i3, &n3, (char *)msgsent.c_str(), const_cast<char *>(ft8msgbits),
			const_cast<int *>(itone), 37, 37);

	printf("FSK tones: ");
	for (int j = 0; j < nsym; ++j)
	{
		printf("%d", itone[j]);
	}
	printf("\n");

	int nsps = 4 * 1920;
	float fsample = audioinput->get_samplerate();
	float bt = 2.0;
	int icmplx = 0;
	int nwave = nsym * nsps;
	float f0 = frequency;

	float symbol_period = (is_ft4) ? FT4_SYMBOL_PERIOD : FT8_SYMBOL_PERIOD;
	float symbol_bt = (is_ft4) ? FT4_SYMBOL_BT : FT8_SYMBOL_BT;
	float slot_time = (is_ft4) ? FT4_SLOT_TIME : FT8_SLOT_TIME;
	int num_samples = (int)(0.5f + nsym * symbol_period * audioinput->get_samplerate());
	
	signal.clear();
	signal.resize(num_samples);

	gen_ft8wave_(const_cast<int *>(itone), &nsym, &nsps, &bt, &fsample, &f0, signal.data(),
				 signal.data(), &icmplx, &nwave);
	signal.resize(nwave);
	printf("frequency %d number of tones %d, samplerate %d no samples %d\n", frequency, nsym, audioinput->get_samplerate(), nwave);
	save_wav(signal.data(), signal.size(), audioinput->get_samplerate(), "./wave.wav");
	return signal;
}