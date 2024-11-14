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
#include <json/json.h>

#ifndef HOST_NAME_MAX
# if defined(_POSIX_HOST_NAME_MAX)
#  define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
# elif defined(MAXHOSTNAMELEN)
#  define HOST_NAME_MAX MAXHOSTNAMELEN
# endif
#endif /* HOST_NAME_MAX */

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
		binanceErrorUnknown,
	};

	const char* binanceGetErrorString(const binanceError_t err);

	template<typename T> std::string toString(const T& val)
	{
		std::ostringstream out;
		out << val;
		return out.str();
	}

	std::string toString(double val);

	class Server
	{
		const std::string hostname;
		const bool simulation;
		std::string sessionId;

	public :

		Server(const char* hostname = "https://api.binance.com",const char* prefix = "/api/v3", bool simulation = false);
		
		const std::string& getHostname() const;
		bool isSimulator() const;
		
		binanceError_t getTime(Json::Value &json_result);
		binanceError_t setTime(const time_t time, unsigned int scale = 1);

		static binanceError_t getCurl(std::string &result_json, const std::string& url);

		static binanceError_t getCurlWithHeader(std::string& result_json, const std::string& url,
			const std::vector<std::string>& extra_http_header, const std::string& post_data, const std::string& action);
	
		const std::string prefix;

	};

	class Market
	{
		const std::string& hostname;
		const Server& server;
		std::string prefix;


	public :

		Market(const binance::Server& server);

		binanceError_t getAllPrices(Json::Value &json_result);
		binanceError_t getPrice(const char *symbol, double& price);
		binanceError_t getPriceTick(const char *symbol, double& askPrice, double& bidPrice, double& askQty, double& bidQty);

		binanceError_t getAllBookTickers(Json::Value &json_result);
		binanceError_t getBookTicker(Json::Value &json_result, const char *symbol);

		binanceError_t getDepth(Json::Value &json_result, const char *symbol, int limit = 100);
		binanceError_t getAggTrades(Json::Value &json_result, const char *symbol, int fromId, int limit = 500);
		binanceError_t getAggTrades(Json::Value &json_result, const char *symbol, time_t startTime, time_t endTime, int limit = 500);

		binanceError_t get24hr(Json::Value &json_result, const char *symbol);
		binanceError_t get24hrTick(const char *symbol, double& lastPrice,
		double& askPrice, double& askQty, double& bidPrice, double& bidQty,
		double& highPrice, double& lowPrice, double& priceChangePercent, double& quoteVolume);

		binanceError_t getLastFundingRate(Json::Value &json_result, const char *symbol);
		binanceError_t getServerTime(Json::Value &json_result);

		binanceError_t getKlines(Json::Value &json_result, const char *symbol, const char *interval,
			time_t startTime = 0, time_t endTime = 0, int limit = 500);
		binanceError_t getExchangeInfo(Json::Value &json_result);
		binanceError_t getExchangeInfoLocaly(Json::Value &json_result);
		binanceError_t getLotSize(const char *symbol, double& maxQty, double& minQty, double& stepSize);
		binanceError_t getTickSize(const char *symbol, double& maxQty, double& minQty, double& stepSize);
		binanceError_t getMinNotional(const char *symbol, double& minNotional);
	};

	// API + Secret keys required
	class Account
	{
		const std::string& hostname;
		const Server& server;
		
		std::string api_key, secret_key;

	public :

		static const std::string default_api_key_path;
		static const std::string default_secret_key_path;

		Account(const binance::Server& server,
			const std::string api_key = "", const std::string secret_key = "");

		const std::string prefix;

		bool keysAreSet() const;

		binanceError_t getInfo(Json::Value &json_result, long recvWindow = 0);

		binanceError_t getTrades(Json::Value &json_result, const char *symbol, int limit = 500);

		binanceError_t getTradesSigned(Json::Value &json_result, const char *symbol, long fromId = -1,
			long recvWindow = 0, int limit = 500);

		binanceError_t getHistoricalTrades(Json::Value &json_result, const char *symbol, long fromId = -1, int limit = 500);

		binanceError_t getOpenOrders(Json::Value &json_result, long recvWindow = 0);

		binanceError_t getOpenOrders(Json::Value &json_result, const char *symbol, long recvWindow = 0);

		binanceError_t getAllOrders(Json::Value &json_result, const char *symbol,
			long orderId = 0, int limit = 0, long recvWindow = 0);

		binanceError_t sendOrder(Json::Value &json_result, const char *symbol, const char *side, const char *type,
			const char *timeInForce, double quantity, double price, const char *newClientOrderId, double stopPrice,
			double icebergQty, long recvWindow);

		binanceError_t sendTestOrder(Json::Value &json_result, const char *symbol, const char *side, const char *type,
			const char *timeInForce, double quantity, double price, const char *newClientOrderId, double stopPrice,
			double icebergQty, long recvWindow);

		binanceError_t getOrder(Json::Value &json_result, const char *symbol,
			long orderId, const char *origClientOrderId, long recvWindow);

		binanceError_t cancelOrder(Json::Value &json_result, const char *symbol,
			long orderId, const char *origClientOrderId, const char *newClientOrderId, long recvWindow);

		// API key required
		binanceError_t startUserDataStream(Json::Value &json_result);
		binanceError_t keepUserDataStream(const char *listenKey);
		binanceError_t closeUserDataStream(const char *listenKey);

		// WAPI
		binanceError_t withdraw(Json::Value &json_result,
			const char *asset, const char *network, const char *address, const char *addressTag,
			double amount, const char *name, long recvWindow);

		binanceError_t getDepositHistory(Json::Value &json_result,
			const char *asset, int status, long startTime, long endTime, long recvWindow);

		binanceError_t getWithdrawHistory(Json::Value &json_result,
			const char *asset, int status, long startTime, long endTime, long recvWindow);

		binanceError_t getDepositAddress(Json::Value &json_result, const char *asset, long recvWindow);
	};
}

#endif // BINANCE_H

