#include "MorseDecoder.h"
#include "gui_bar.h"

MorseDecoder::MorseDecoder(float ifrate)
{
	sampling_freq = ifrate;
	bw = sampling_freq / n;
	int k = (int)(0.5 + ((n * target_freq) / sampling_freq));
	omega = (2.0 * M_PI * k) / n;
	sine = sin(omega);
	cosine = cos(omega);
	coeff = 2.0 * cosine;

	unsigned int order = 4; // filter order
	float fc = 0.00208333333333333333333333333333f;		// cutoff frequency 100Hz / 48000Hz
	float f0 = 0.01041666666666666666666666666667f;		// center frequency 500Hz / 48000Hz
	float Ap = 1.0f;		// pass-band ripple
	float As = 40.0f;		// stop-band attenuation

	q = iirfilt_crcf_create_prototype(LIQUID_IIRDES_BUTTER, LIQUID_IIRDES_BANDPASS, LIQUID_IIRDES_TF, order=1, fc, f0, Ap, As);
	iirfilt_crcf_print(q);
}

MorseDecoder::~MorseDecoder()
{
	iirfilt_crcf_destroy(q);
}

void MorseDecoder::decode(const IQSampleVector &samples_in)
{
	float y{0.0},	magnitude;
	int				i = 0;
	IQSampleVector	filter_out;

	for (auto con : samples_in)
	{
		complex<float> v;

		// run filter
		iirfilt_crcf_execute(q, con, &v);
		filter_out.insert(filter_out.end(), v);
	}

	for (auto &out : filter_out)
	{
		y += std::real(out * std::conj(out));
	}
	magnitude = y / samples_in.size();
	//printf("%f\n", magnitude);
	if (magnitude > magnitudelimit_low)
	{
		magnitudelimit = (magnitudelimit + ((magnitude - magnitudelimit) / 6));
	} /// moving average filter
	if (magnitudelimit < magnitudelimit_low)
		magnitudelimit = magnitudelimit_low;

	// Now check the magnitude //
	if (magnitude > magnitudelimit * 0.3) // just to have some space up
	{
		realstate = 1;
	}
	else
	{
		realstate = 0;
	}
	
	if (realstate != realstatebefore)
	{
		laststarttime = std::chrono::high_resolution_clock::now();
	}
	
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - laststarttime);
	//if ((millis() - laststarttime) > nbtime)
	if (timePassed.count() > nbtime)
	{
		if (realstate != filteredstate)
		{
			filteredstate = realstate;
		}
	}

	if (filteredstate) 
	{
		gbar.set_led(true);
	}
	else
	{
		gbar.set_led(false);
	}
	
	if (filteredstate != filteredstatebefore)
	{
		if (filteredstate == HIGH)
		{
			starttimehigh = std::chrono::high_resolution_clock::now(); //millis();
			lowduration = std::chrono::duration_cast<std::chrono::microseconds>(now - startttimelow);
		}

		if (filteredstate == LOW)
		{
			startttimelow = std::chrono::high_resolution_clock::now(); //millis();
			auto now = std::chrono::high_resolution_clock::now();
			highduration = std::chrono::duration_cast<std::chrono::microseconds>(now - starttimehigh);

			//highduration = (millis() - starttimehigh);
			if (highduration.count() < (2 * hightimesavg.count()) || hightimesavg.count() == 0)
			{
				hightimesavg = (highduration + hightimesavg + hightimesavg) / 3; // now we know avg dit time ( rolling 3 avg)
			}
			if (highduration > (5 * hightimesavg))
			{
				hightimesavg = highduration + hightimesavg; // if speed decrease fast ..
			}
		}
	}

	// Now check the baud rate based on dit or dah duration either 1, 3 or 7 pauses
	if (filteredstate != filteredstatebefore)
	{
		stop = LOW;
		if (filteredstate == LOW)
		{ // we did end on a HIGH
			if (highduration < (hightimesavg * 2) && highduration > (hightimesavg * 0.6))
			{ /// 0.6 filter out false dits
				strcat(CodeBuffer, ".");
				//printf(". ");
			}
			if (highduration > (hightimesavg * 2) && highduration < (hightimesavg * 6))
			{
				strcat(CodeBuffer, "-");
				//printf("- ");
				wpm = (wpm + (1200000 / ((highduration.count()) / 3))) / 2; //// the most precise we can do ;o)
				gbar.set_cw_wpm(wpm);
				}
		}

		if (filteredstate == HIGH)
		{ //// we did end a LOW
			float lacktime = 1;
			if (wpm > 25)
				lacktime = 1.0; ///  when high speeds we have to have a little more pause before new letter or new word
			if (wpm > 30)
				lacktime = 1.2;
			if (wpm > 35)
				lacktime = 1.5;
			if (lowduration.count() > (hightimesavg.count() * (2 * lacktime)) && lowduration.count() < hightimesavg.count() * (5 * lacktime))
			{ // letter space
				CodeToChar();
				CodeBuffer[0] = '\0';
				//AddCharacter('/');
				//Serial.print("/");
			}
			if (lowduration.count() >= hightimesavg.count() * (5 * lacktime))
			{ // word space
				CodeToChar();
				CodeBuffer[0] = '\0';
				AddCharacter(' ');
			}
		}
	}

	now = std::chrono::high_resolution_clock::now();
	auto timePassed1 = std::chrono::duration_cast<std::chrono::microseconds>(now - startttimelow);
	if (timePassed1.count() > (highduration.count() * 6) && stop == LOW)
	{
		CodeToChar();
		CodeBuffer[0] = '\0';
		stop = HIGH;
	}
	// the end of main loop clean up//
	realstatebefore = realstate;
	lasthighduration = highduration;
	filteredstatebefore = filteredstate;
}

void MorseDecoder::read(std::string &message)
{
	message = DisplayLine;
}

void MorseDecoder::CodeToChar()
{ // translate cw code to ascii character//
	char decode_char = '{';
	if (strcmp(CodeBuffer, ".-") == 0)
		decode_char = char('a');
	if (strcmp(CodeBuffer, "-...") == 0)
		decode_char = char('b');
	if (strcmp(CodeBuffer, "-.-.") == 0)
		decode_char = char('c');
	if (strcmp(CodeBuffer, "-..") == 0)
		decode_char = char('d');
	if (strcmp(CodeBuffer, ".") == 0)
		decode_char = char('e');
	if (strcmp(CodeBuffer, "..-.") == 0)
		decode_char = char('f');
	if (strcmp(CodeBuffer, "--.") == 0)
		decode_char = char('g');
	if (strcmp(CodeBuffer, "....") == 0)
		decode_char = char('h');
	if (strcmp(CodeBuffer, "..") == 0)
		decode_char = char('i');
	if (strcmp(CodeBuffer, ".---") == 0)
		decode_char = char('j');
	if (strcmp(CodeBuffer, "-.-") == 0)
		decode_char = char('k');
	if (strcmp(CodeBuffer, ".-..") == 0)
		decode_char = char('l');
	if (strcmp(CodeBuffer, "--") == 0)
		decode_char = char('m');
	if (strcmp(CodeBuffer, "-.") == 0)
		decode_char = char('n');
	if (strcmp(CodeBuffer, "---") == 0)
		decode_char = char('o');
	if (strcmp(CodeBuffer, ".--.") == 0)
		decode_char = char('p');
	if (strcmp(CodeBuffer, "--.-") == 0)
		decode_char = char('q');
	if (strcmp(CodeBuffer, ".-.") == 0)
		decode_char = char('r');
	if (strcmp(CodeBuffer, "...") == 0)
		decode_char = char('s');
	if (strcmp(CodeBuffer, "-") == 0)
		decode_char = char('t');
	if (strcmp(CodeBuffer, "..-") == 0)
		decode_char = char('u');
	if (strcmp(CodeBuffer, "...-") == 0)
		decode_char = char('v');
	if (strcmp(CodeBuffer, ".--") == 0)
		decode_char = char('w');
	if (strcmp(CodeBuffer, "-..-") == 0)
		decode_char = char('x');
	if (strcmp(CodeBuffer, "-.--") == 0)
		decode_char = char('y');
	if (strcmp(CodeBuffer, "--..") == 0)
		decode_char = char('z');

	if (strcmp(CodeBuffer, ".----") == 0)
		decode_char = char('1');
	if (strcmp(CodeBuffer, "..---") == 0)
		decode_char = char('2');
	if (strcmp(CodeBuffer, "...--") == 0)
		decode_char = char('3');
	if (strcmp(CodeBuffer, "....-") == 0)
		decode_char = char('4');
	if (strcmp(CodeBuffer, ".....") == 0)
		decode_char = char('5');
	if (strcmp(CodeBuffer, "-....") == 0)
		decode_char = char('6');
	if (strcmp(CodeBuffer, "--...") == 0)
		decode_char = char('7');
	if (strcmp(CodeBuffer, "---..") == 0)
		decode_char = char('8');
	if (strcmp(CodeBuffer, "----.") == 0)
		decode_char = char('9');
	if (strcmp(CodeBuffer, "-----") == 0)
		decode_char = char('0');

	if (strcmp(CodeBuffer, "..--..") == 0)
		decode_char = char('?');
	if (strcmp(CodeBuffer, ".-.-.-") == 0)
		decode_char = char('.');
	if (strcmp(CodeBuffer, "--..--") == 0)
		decode_char = char(',');
	if (strcmp(CodeBuffer, "-.-.--") == 0)
		decode_char = char('!');
	if (strcmp(CodeBuffer, ".--.-.") == 0)
		decode_char = char('@');
	if (strcmp(CodeBuffer, "---...") == 0)
		decode_char = char(':');
	if (strcmp(CodeBuffer, "-....-") == 0)
		decode_char = char('-');
	if (strcmp(CodeBuffer, "-..-.") == 0)
		decode_char = char('/');

	if (strcmp(CodeBuffer, "-.--.") == 0)
		decode_char = char('(');
	if (strcmp(CodeBuffer, "-.--.-") == 0)
		decode_char = char(')');
	if (strcmp(CodeBuffer, ".-...") == 0)
		decode_char = char('_');
	if (strcmp(CodeBuffer, "...-..-") == 0)
		decode_char = char('$');
	if (strcmp(CodeBuffer, "...-.-") == 0)
		decode_char = char('>');
	if (strcmp(CodeBuffer, ".-.-.") == 0)
		decode_char = char('<');
	if (strcmp(CodeBuffer, "...-.") == 0)
		decode_char = char('~');
	if (strcmp(CodeBuffer, ".-.-") == 0)
		decode_char = char('a'); // a umlaut
	if (strcmp(CodeBuffer, "---.") == 0)
		decode_char = char('o'); // o accent
	if (strcmp(CodeBuffer, ".--.-") == 0)
		decode_char = char('a'); // a accent
	if (decode_char != '{')
	{
		AddCharacter(decode_char);
	}
}

void MorseDecoder::AddCharacter(char newchar)
{
	DisplayLine.push_back(newchar) ;
	if (DisplayLine.size() > 40)
		DisplayLine.erase(0,1);
	gbar.set_cw_message(DisplayLine);
}