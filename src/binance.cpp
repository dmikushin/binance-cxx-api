/*
	Author: tensaix2j
	Date  : 2017/10/15

	C++ library for Binance API.
*/

#include "binance.h"
#include "binance_logger.h"
#include "binance_utils.h"

using namespace binance;
using namespace std;

#define BINANCE_CASE_STR(err) case err : { static const string str_##err = #err; return str_##err.c_str(); }

const char* binance::binanceGetErrorString(const binanceError_t err)
{
	switch (err)
	{
	BINANCE_CASE_STR(binanceSuccess);
	BINANCE_CASE_STR(binanceErrorInvalidServerResponse);
	BINANCE_CASE_STR(binanceErrorEmptyServerResponse);
	BINANCE_CASE_STR(binanceErrorParsingServerResponse);
	BINANCE_CASE_STR(binanceErrorInvalidSymbol);
	BINANCE_CASE_STR(binanceErrorMissingAccountKeys);
	BINANCE_CASE_STR(binanceErrorCurlFailed);
	BINANCE_CASE_STR(binanceErrorCurlOutOfMemory);
	}
}

std::string binance::toString(double val)
{
	std::ostringstream out;
	out.precision(8);
	out.setf(std::ios_base::fixed);
	out << val;
	return out.str();
}

// Curl's callback
size_t binance::getCurlCb(void *content, size_t size, size_t nmemb, std::string *buffer)
{
	Logger::write_log("<curl_cb> ");

	size_t newLength = size * nmemb;
	size_t oldLength = buffer->size();

	buffer->resize(oldLength + newLength);

	std::copy((char*)content,(char*)content + newLength, buffer->begin() + oldLength);

	Logger::write_log("<curl_cb> Done.");

	return newLength;
}

binanceError_t binance::getCurl(string &url, string &result_json)
{
	vector<string> v;
	string action = "GET";
	string post_data = "";
	return getCurlWithHeader(url, result_json, v, post_data, action);
}

// Do the curl
binanceError_t binance::getCurlWithHeader(
	string &url, string &str_result, vector<string> &extra_http_header, string &post_data, string &action)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<curl_api>");

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getCurlCb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str_result);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

		if (extra_http_header.size() > 0)
		{
			struct curl_slist *chunk = NULL;
			for (int i = 0; i < extra_http_header.size(); i++)
				chunk = curl_slist_append(chunk, extra_http_header[i].c_str());

 			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
 		}

 		if (post_data.size() > 0 || action == "POST" || action == "PUT" || action == "DELETE")
 		{
 			if (action == "PUT" || action == "DELETE") {
 				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, action.c_str());
 			}
 			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
 		}

		try
		{
			res = curl_easy_perform(curl);
		}
		catch (std::bad_alloc &e)
		{
			status = binanceErrorCurlOutOfMemory;
		}

		/* Check for errors */
		if (res != CURLE_OK)
		{
			Logger::write_log("<curl_api> curl_easy_perform() failed: %s", curl_easy_strerror(res));
			status = binanceErrorCurlFailed;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	Logger::write_log("<curl_api> Done.");

	return status;
}

