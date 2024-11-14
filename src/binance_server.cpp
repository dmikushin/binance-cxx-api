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

binance::Server::Server(const char* hostname_,const char* prefix, bool simulation_) : hostname(hostname_),prefix(prefix) ,simulation(simulation_) { }

const std::string& binance::Server::getHostname() const 
{ 
	return hostname; 
}

bool binance::Server::isSimulator() const { return simulation; }

// GET /api/v3/time
binanceError_t binance::Server::getTime(Json::Value &json_result)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_serverTime>");

	string url(hostname);
	url += prefix +"/time";

	string str_result;
	getCurl(str_result, url);

	if (str_result.size() == 0)
		status = binanceErrorEmptyServerResponse;
	else
	{
		try
		{
			Json::Reader reader;
			json_result.clear();
			reader.parse(str_result, json_result);
			CHECK_SERVER_ERR(json_result);
		}
		catch (exception &e)
		{
		 	Logger::write_log("<get_serverTime> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_serverTime> Done.");

	return status;
}

// Curl's callback
static size_t getCurlCb(void *content, size_t size, size_t nmemb, std::string *buffer)
{
	Logger::write_log("<curl_cb> ");

	size_t newLength = size * nmemb;
	size_t oldLength = buffer->size();

	buffer->resize(oldLength + newLength);

	std::copy((char*)content, (char*)content + newLength, buffer->begin() + oldLength);

	Logger::write_log("<curl_cb> Done.");

	return newLength;
}

binanceError_t binance::Server::getCurl(string& result_json, const string& url)
{
	vector<string> v;
	string action = "GET";
	string post_data = "";
	return getCurlWithHeader(result_json, url, v, post_data, action);
}

class SmartCURL
{
	CURL* curl;

public :

	CURL* get() { return curl; }

	SmartCURL()
	{
		curl = curl_easy_init();
	}

	~SmartCURL()
	{
		curl_easy_cleanup(curl);
	}
};

// Do the curl
binanceError_t binance::Server::getCurlWithHeader(string& str_result, 
	const string& url, const vector<string>& extra_http_header, const string& post_data, const string& action)
{
	binanceError_t status = binanceSuccess;
	
	Logger::write_log("<curl_api>");

	SmartCURL curl;

	while (curl.get())
	{
		curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, getCurlCb);
		curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &str_result);
		curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, false);
		if (curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, true) != CURLE_OK)
		{
			Logger::write_log("<curl_api> curl_easy_setopt(CURLOPT_SSL_VERIFYPEER) is not supported");
			status = binanceErrorCurlFailed;
			break;
		}

		if (extra_http_header.size() > 0)
		{
			struct curl_slist *chunk = NULL;
			for (int i = 0; i < extra_http_header.size(); i++)
				chunk = curl_slist_append(chunk, extra_http_header[i].c_str());

 			curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, chunk);
 		}

 		if (post_data.size() > 0 || action == "POST" || action == "PUT" || action == "DELETE")
 		{
 			if (action == "PUT" || action == "DELETE")
 				curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, action.c_str());
 			curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, post_data.c_str());
 		}

		CURLcode res;

		try
		{
			res = curl_easy_perform(curl.get());
		}
		catch (std::bad_alloc &e)
		{
			status = binanceErrorCurlOutOfMemory;
		}
		
		if (status == binanceSuccess)
		{
			// Check for errors.
			if (res != CURLE_OK)
			{
				Logger::write_log("<curl_api> curl_easy_perform() failed: %s", curl_easy_strerror(res));
				status = binanceErrorCurlFailed;
			}
		}

		break;
	}

	Logger::write_log("<curl_api> Done.");

	return status;
}

