#include "FT8Generator.h"
#include <wsjtx_lib.h>
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

std::vector<float> FT8Generator::generate(int frequency, encoding code, std::string message)
{
	wsjtx_lib wsjtx;

	wsjtx.encode();
	printf("frequency %d number of tones %d, samplerate %d no samples %d\n", frequency, nsym, audioinput->get_samplerate(), nwave);
	save_wav(signal.data(), signal.size(), audioinput->get_samplerate(), "./wave.wav");
	return signal;
}