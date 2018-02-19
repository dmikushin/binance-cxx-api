/*
	Author: tensaix2j
	Date  : 2017/10/15

	C++ library for Binance API.
*/

#ifndef BINANCE_H
#define BINANCE_H

#include <cstdio>
#include <cstring>
#include <limits.h> // HOST_NAME_MAX
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

#include <curl/curl.h>
#include <jsoncpp/json/json.h>

#define CHECK_SERVER_ERR(result)                                     \
    do {                                                             \
        using namespace binance;                                     \
        bool err = false;                                            \
        if (result.isObject())                                       \
        {                                                            \
            const vector<string> keys = result.getMemberNames();     \
            for (int i = 0, e = keys.size(); i < e; i++)             \
            {                                                        \
                const string& ikey = keys[i];                        \
                if (ikey == "code")                                  \
                {                                                    \
                    for (int j = 0, e = keys.size(); j < e; j++)     \
                    {                                                \
                        const string& jkey = keys[j];                \
                        if (jkey == "msg") { err = true; break; }    \
                    }                                                \
                }                                                    \
                                                                     \
                if (err) break;                                      \
            }                                                        \
        }                                                            \
        if (!err) break;                                             \
                                                                     \
        char hostname[HOST_NAME_MAX] = "";                           \
        gethostname(hostname, HOST_NAME_MAX);                        \
        fprintf(stderr, "BINANCE error %s \"%s\" on %s at %s:%d\n",  \
            result["code"].asString().c_str(),                       \
            result["msg"].asString().c_str(),                        \
            hostname, __FILE__, __LINE__);                           \
        if (!getenv("FREEZE_ON_ERROR")) {                            \
            fprintf(stderr, "You may want to set "                   \
                "FREEZE_ON_ERROR environment "                       \
                "variable to debug the case\n");                     \
            exit(-1);                                                \
        }                                                            \
        else {                                                       \
            fprintf(stderr, "thread 0x%zx of pid %d @ %s "           \
               "is entering infinite loop\n",                        \
               (size_t)pthread_self(), (int)getpid(), hostname);     \
            while (1) usleep(1000000); /* 1 sec */                   \
        }                                                            \
    } while (0);

#define BINANCE_ERR_CHECK(x)                                         \
    do {                                                             \
        using namespace binance;                                     \
        binanceError_t err = x; if (err != binanceSuccess) {         \
        char hostname[HOST_NAME_MAX] = "";                           \
        gethostname(hostname, HOST_NAME_MAX);                        \
        fprintf(stderr, "BINANCE error %d \"%s\" on %s at %s:%d\n",  \
            (int)err, binanceGetErrorString(err), hostname,          \
            __FILE__, __LINE__);                                     \
        if (!getenv("FREEZE_ON_ERROR")) {                            \
            fprintf(stderr, "You may want to set "                   \
                "FREEZE_ON_ERROR environment "                       \
                "variable to debug the case\n");                     \
            exit(-1);                                                \
        }                                                            \
        else {                                                       \
            fprintf(stderr, "thread 0x%zx of pid %d @ %s "           \
               "is entering infinite loop\n",                        \
               (size_t)pthread_self(), (int)getpid(), hostname);     \
            while (1) usleep(1000000); /* 1 sec */                   \
        }                                                            \
    }} while (0)

namespace binance
{
	enum binanceError_t
	{
		binanceSuccess = 0,
		binanceErrorInvalidServerResponse,
		binanceErrorEmptyServerResponse,
		binanceErrorParsingServerResponse,
		binanceErrorInvalidSymbol,
		binanceErrorMissingAccountKeys,
		binanceErrorCurlFailed,
		binanceErrorCurlOutOfMemory,
	};

	const char* binanceGetErrorString(const binanceError_t err);

	template<typename T> std::string toString(const T& val)
	{
		std::ostringstream out;
		out << val;
		return out.str();
	}

	std::string toString(double val);

	binanceError_t getCurl(std::string &url, std::string &result_json);

	binanceError_t getCurlWithHeader(std::string &url, std::string &result_json,
		std::vector <std::string> &extra_http_header, std::string &post_data, std::string &action);

	size_t getCurlCb(void *content, size_t size, size_t nmemb, std::string *buffer);

	class Market
	{
		const std::string hostname;

	public :

		Market(const char* hostname = "https://api.binance.com");

		binanceError_t getServerTime(Json::Value &json_result); 

		binanceError_t getAllPrices(Json::Value &json_result);
		binanceError_t getPrice(const char *symbol, double& price);

		binanceError_t getAllBookTickers(Json::Value &json_result);
		binanceError_t getBookTicker(const char *symbol, Json::Value &json_result);

		binanceError_t getDepth(const char *symbol, int limit, Json::Value &json_result);
		binanceError_t getAggTrades(const char *symbol, int fromId, time_t startTime, time_t endTime,
			int limit, Json::Value &json_result);

		binanceError_t get24hr(const char *symbol, Json::Value &json_result);

		binanceError_t getKlines(const char *symbol, const char *interval, int limit,
			time_t startTime, time_t endTime, Json::Value &json_result);
	};

	// API + Secret keys required
	class Account
	{
		const std::string hostname;
		std::string api_key, secret_key;

	public :

		Account(const char* hostname = "https://api.binance.com",
			const std::string api_key = "", const std::string secret_key = "");

		binanceError_t getInfo(Json::Value &json_result, long recvWindow = 0);

		binanceError_t getTrades(const char *symbol, int limit, long fromId,
			long recvWindow, Json::Value &json_result);

		binanceError_t getOpenOrders(Json::Value &json_result, long recvWindow = 0);

		binanceError_t getOpenOrders(Json::Value &json_result, const char *symbol, long recvWindow = 0);

		binanceError_t getAllOrders(Json::Value &json_result, const char *symbol,
			long orderId = 0, int limit = 0, long recvWindow = 0);

		binanceError_t sendOrder(const char *symbol, const char *side, const char *type, const char *timeInForce,
			double quantity, double price, const char *newClientOrderId, double stopPrice, double icebergQty,
			long recvWindow, Json::Value &json_result);

		binanceError_t getOrder(const char *symbol, long orderId, const char *origClientOrderId,
			long recvWindow, Json::Value &json_result);

		binanceError_t cancelOrder(const char *symbol, long orderId, const char *origClientOrderId, const char *newClientOrderId,
			long recvWindow, Json::Value &json_result);

		// API key required
		binanceError_t startUserDataStream(Json::Value &json_result);
		binanceError_t keepUserDataStream(const char *listenKey);
		binanceError_t closeUserDataStream(const char *listenKey);

		// WAPI
		binanceError_t withdraw(const char *asset, const char *address, const char *addressTag,
			double amount, const char *name, long recvWindow, Json::Value &json_result);

		binanceError_t getDepositHistory(const char *asset, int status, long startTime, long endTime,
			long recvWindow, Json::Value &json_result);

		binanceError_t getWithdrawHistory(const char *asset, int status, long startTime, long endTime,
			long recvWindow, Json::Value &json_result);

		binanceError_t getDepositAddress(const char *asset, long recvWindow, Json::Value &json_result);
	};
}

#endif // BINANCE_H

