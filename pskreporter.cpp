#include "pskreporter.h"
#include <iostream>
#include "tinyxml2.h"
#include <curl/curl.h>
#include <algorithm>

using namespace tinyxml2;

std::string epochToUTC(long epoch)
{
	time_t t = static_cast<time_t>(epoch);
	char buf[20];
	strftime(buf, sizeof(buf), "%H:%M:%S", gmtime(&t)); // HH:MM:SS UTC
	return std::string(buf);
}

// CURL write callback
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

void pskreporter(std::string call, std::vector<ReceptionReport> &reports, std::vector<ActiveCallsign> &activeList)
{
	std::string url = "https://retrieve.pskreporter.info/query?senderCallsign=" + call;
	std::string readBuffer;

	// Fetch XML from URL
	CURL *curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
			return ;
		}
		curl_easy_cleanup(curl);
	}

	// Parse XML
	XMLDocument doc;
	XMLError err = doc.Parse(readBuffer.c_str());
	if (err != XML_SUCCESS)
	{
		std::cerr << "XML parse error: " << doc.ErrorStr() << "\n";
		return ;
	}

	XMLElement *root = doc.FirstChildElement("receptionReports");
	if (!root)
	{
		std::cerr << "No <receptionReports> element found.\n";
		return ;
	}

	// Parse receptionReport elements
	XMLElement *report = root->FirstChildElement("receptionReport");
	while (report)
	{
		ReceptionReport rr;
		rr.mode = report->Attribute("mode") ? report->Attribute("mode") : "";
		rr.call = report->Attribute("receiverCallsign") ? report->Attribute("receiverCallsign") : "";
		report->QueryInt64Attribute("frequency", &rr.frequency);
		rr.locator = report->Attribute("receiverLocator") ? report->Attribute("receiverLocator") : "";
		rr.SNR = report->Attribute("sNR") ? report->Attribute("sNR") : "";
		rr.timeEpoch = report->Attribute("flowStartSeconds") ? atol(report->Attribute("flowStartSeconds")) : 0;
		rr.timeUTC = epochToUTC(rr.timeEpoch);
		reports.push_back(rr);

		report = report->NextSiblingElement("receptionReport");
	}

	std::sort(reports.begin(), reports.end(),
		 [](const ReceptionReport &a, const ReceptionReport &b) {
			 return a.locator < b.locator;
		 });
	
	// Parse activeCallsign elements
	XMLElement *active = root->FirstChildElement("activeCallsign");
	while (active)
	{
		ActiveCallsign ac;
		ac.callsign = active->Attribute("callsign") ? active->Attribute("callsign") : "";
		active->QueryIntAttribute("reports", &ac.reports);
		ac.DXCC = active->Attribute("DXCC") ? active->Attribute("DXCC") : "";
		ac.DXCCcode = active->Attribute("DXCCcode") ? active->Attribute("DXCCcode") : "";
		active->QueryInt64Attribute("frequency", &ac.frequency);
		activeList.push_back(ac);

		active = active->NextSiblingElement("activeCallsign");
	}
}