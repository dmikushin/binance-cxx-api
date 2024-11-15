/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#include "binance.h"
#include "binance_logger.h"
#include "binance_utils.h"

#include <fstream>
#ifndef _WIN32
#include <wordexp.h>
#endif

using namespace binance;
using namespace std;

const string binance::Account::default_api_key_path = "$HOME/.bitrader/key";
const string binance::Account::default_secret_key_path = "$HOME/.bitrader/secret";

binance::Account::Account(const binance::Server& server_, const string api_key_, const string secret_key_) :

hostname(server_.getHostname()), server(server_), api_key(api_key_), secret_key(secret_key_),prefix(server_.prefix)

{
	if (api_key == "")
	{
#ifndef _WIN32
		wordexp_t p;
		char** w;
		wordexp(default_api_key_path.c_str(), &p, 0);
		w = p.we_wordv;
		ifstream binanceapi(w[0]);
#else
		ifstream binanceapi(default_api_key_path.c_str());
#endif
		if (binanceapi.is_open())
		{
			binanceapi >> api_key;
			binanceapi.close();
		}
#ifndef _WIN32
		wordfree(&p);
#endif
	}

	if (secret_key == "")
	{
#ifndef _WIN32
		wordexp_t p;
		char** w;
		wordexp(default_secret_key_path.c_str(), &p, 0);
		w = p.we_wordv;
		ifstream binanceapi(w[0]);
#else
		ifstream binanceapi(default_secret_key_path.c_str());
#endif
		if (binanceapi.is_open())
		{
			binanceapi >> secret_key;
			binanceapi.close();
		}
#ifndef _WIN32
		wordfree(&p);
#endif
	}
}

bool binance::Account::keysAreSet() const
{
	return ((api_key != "") && (secret_key != ""));
}

// Get current account information. (SIGNED)
//
// GET /api/v3/account
//
// Parameters:
// Name		Type	Mandatory	Description
// recvWindow	LONG	NO	
// timestamp	LONG	YES
//
binanceError_t binance::Account::getInfo(Json::Value &json_result, long recvWindow) 
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_account>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/account?";
		string action = "GET";

		string querystring("timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<get_account> url = |%s|", url.c_str());
	
		string post_data = "";
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_account> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_account> Error ! %s", e.what()); 
				status = binanceErrorParsingServerResponse;
			}   
		}
	}

	Logger::write_log("<get_account> Done.\n");
	
	return status;
}

// Get current account information. (SIGNED)
//
// GET /api/v3/account
//
// Parameters:
// Name		Type	Mandatory	Description
// recvWindow	LONG	NO	
// timestamp	LONG	YES
//
binanceError_t binance::Account::getBalance_f(Json::Value& json_result, long recvWindow)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_balance>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url = url+ "/fapi/v2/balance?";
		string action = "GET";

		string querystring("timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		string signature = hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<get_balance> url = |%s|", url.c_str());

		string post_data = "";

		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_balance> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception& e)
			{
				Logger::write_log("<get_balance> Error ! %s", e.what());
				status = binanceErrorParsingServerResponse;
			}
		}
	}

	Logger::write_log("<get_balance> Done.\n");

	return status;
}


// Recent trades list
//
// GET /api/v3/trades
// Get recent trades (up to last 500).
//
// Name	Type	Mandatory	Description
// symbol	STRING	YES	
// limit	INT	NO	Default 500; max 500.
//
binanceError_t binance::Account::getTrades(Json::Value &json_result, const char *symbol, int limit)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_trades>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/trades?";

		string querystring("symbol=");
		querystring.append(symbol);
	
		querystring.append("&limit=");
		querystring.append(to_string(limit));

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<get_trades> url = |%s|", url.c_str());

		string action = "GET";
		string post_data = "";

		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_trades> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_trades> Error ! %s", e.what());
				status = binanceErrorParsingServerResponse;
			}
		}

		Logger::write_log("<get_trades> Done.");
	}

	return status;
}

// Get trades for a specific account and symbol. (SIGNED)
//
// GET /api/v3/myTrades
// Name		Type	Mandatory	Description
// symbol		STRING	YES	
// limit		INT		NO	Default 500; max 500.
// fromId		LONG	NO	TradeId to fetch from. Default gets most recent trades.
// recvWindow	LONG	NO	
// timestamp	LONG	YES
//
binanceError_t binance::Account::getTradesSigned(Json::Value &json_result, const char *symbol, long fromId, long recvWindow, int limit)
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_myTrades>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/myTrades?";

		string querystring("symbol=");
		querystring.append(symbol);

		if (fromId != -1)
		{
			querystring.append("&fromId=");
			querystring.append(to_string(fromId));
		}

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&limit=");
		querystring.append(to_string(limit));

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<get_myTrades> url = |%s|", url.c_str());
	
		string action = "GET";
		string post_data = "";

		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_myTrades> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_myTrades> Error ! %s", e.what()); 
			}   
		}
	}

	Logger::write_log("<get_myTrades> Done.\n");

	return status;
}

// Old trade lookup (MARKET_DATA)
//
// GET /api/v3/historicalTrades
//
// Name	Type	Mandatory	Description
// symbol	STRING	YES	
// limit	INT	NO	Default 500; max 500.
// fromId	LONG	NO	TradeId to fetch from. Default gets most recent trades.
//
binanceError_t binance::Account::getHistoricalTrades(Json::Value &json_result, const char *symbol, long fromId, int limit)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_historicalTrades>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/historicalTrades?";

		string querystring("symbol=");
		querystring.append(symbol);
	
		if (fromId != -1)
		{
			querystring.append("&fromId=");
			querystring.append(to_string(fromId));
		}

		querystring.append("&limit=");
		querystring.append(to_string(limit));

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<get_historicalTrades> url = |%s|", url.c_str());

		string action = "GET";
		string post_data = "";

		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_historicalTrades> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_historicalTrades> Error ! %s", e.what());
				status = binanceErrorParsingServerResponse;
			}
		}

		Logger::write_log("<get_historicalTrades> Done.");
	}

	return status;
}

// Open Orders (SIGNED)
//
// GET /api/v3/openOrders
//
// Name		Type	Mandatory	Description
// symbol		STRING	YES	
// recvWindow	LONG	NO	
// timestamp	LONG	YES	
//
binanceError_t binance::Account::getOpenOrders(Json::Value &json_result, long recvWindow) 
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_openOrders>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/openOrders?";

		string querystring("");
		querystring.append("timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);
	
		string action = "GET";
		string post_data ="";
		
		Logger::write_log("<get_openOrders> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_openOrders> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_openOrders> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_openOrders> Done.\n");

	return status;
}

// Open Orders (SIGNED)
//
// GET /api/v3/openOrders
//
// Name		Type	Mandatory	Description
// symbol		STRING	YES	
// recvWindow	LONG	NO	
// timestamp	LONG	YES	
//
binanceError_t binance::Account::getOpenOrders(Json::Value &json_result, const char *symbol, long recvWindow) 
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_openOrders>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/openOrders?";

		string querystring("symbol=");
		querystring.append(symbol);

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);
	
		string action = "GET";
		string post_data ="";
		
		Logger::write_log("<get_openOrders> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_openOrders> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_openOrders> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_openOrders> Done.\n");

	return status;
}

// All Orders (SIGNED)
//
// GET /api/v3/allOrders
//
// Name		Type	Mandatory	Description
// symbol		STRING	YES	
// orderId		LONG	NO	
// limit		INT		NO		Default 500; max 500.
// recvWindow	LONG	NO	
// timestamp	LONG	YES	
//
binanceError_t binance::Account::getAllOrders(Json::Value &json_result, const char *symbol,
	long orderId, int limit, long recvWindow) 
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_allOrders>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/allOrders?";

		string querystring("symbol=");
		querystring.append(symbol);

		if (orderId > 0)
		{
			querystring.append("&orderId=");
			querystring.append(to_string(orderId));
		}

		if (limit > 0)
		{
			querystring.append("&limit=");
			querystring.append(to_string(limit));
		}

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);
	
		string action = "GET";
		string post_data ="";
		
		Logger::write_log("<get_allOrders> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_allOrders> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_allOrders> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_allOrders> Done.\n");

	return status;
}

// Send order (SIGNED)
//
// POST /api/v3/order
//
// Name              Type        Mandatory   Description
// symbol            STRING      YES
// side              ENUM        YES
// type              ENUM        YES
// timeInForce       ENUM        NO
// quantity          DECIMAL     NO
// quoteOrderQty     DECIMAL     NO
// price             DECIMAL     NO
// newClientOrderId  STRING      NO          A unique id for the order. Automatically generated by default.
// stopPrice         DECIMAL     NO          Used with STOP orders
// icebergQty        DECIMAL     NO          Used with icebergOrders
// newOrderRespType  ENUM NO                 Set the response JSON. ACK, RESULT, or FULL; MARKET and LIMIT order types default to FULL
// recvWindow        LONG        NO          The value cannot be greater than 60000
// timestamp         LONG        YES
//
binanceError_t binance::Account::sendOrder(Json::Value &json_result, const char *symbol,
	const char *side, const char *type, const char *timeInForce, double quantity, double price,
	const char *newClientOrderId, double stopPrice, double icebergQty, long recvWindow, int baseAssetPrecision)
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<send_order>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/order?";

		string action = "POST";
	
		string post_data("symbol=");
		post_data.append(symbol);
	
		post_data.append("&side=");
		post_data.append(side);

		post_data.append("&type=");
		post_data.append(type);

		if (strlen(timeInForce) > 0)
		{
			post_data.append("&timeInForce=");
			post_data.append(timeInForce);
		}

		post_data.append("&quantity=");
		post_data.append(toString(quantity,baseAssetPrecision));

		if (price > 0.0)
		{
			post_data.append("&price=");
			post_data.append(toString(price));
		}

		if (strlen(newClientOrderId) > 0)
		{
			post_data.append("&newClientOrderId=");
			post_data.append(newClientOrderId);
		}

		if (stopPrice > 0.0)
		{
			post_data.append("&stopPrice=");
			post_data.append(toString(stopPrice));
		}

		if (icebergQty > 0.0)
		{
			post_data.append("&icebergQty=");
			post_data.append(toString(icebergQty));
		}

        if(strncmp("MARKET", type, sizeof("MARKET")) != 0){
          post_data.append("&newOrderRespType=");
          post_data.append("RESULT");
        }

		if (recvWindow > 0)
		{
			post_data.append("&recvWindow=");
			post_data.append(to_string(recvWindow));
		}

		post_data.append("&timestamp=");
		post_data.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), post_data.c_str());
		post_data.append("&signature=");
		post_data.append(signature);

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<send_order> url = |%s|, post_data = |%s|", url.c_str(), post_data.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<send_order> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<send_order> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<send_order> Done.\n");

	return status;
}

// Send test order (SIGNED)
//
// POST /api/v3/order/test
//
// Name				Type		Mandatory	Description
// symbol				STRING		YES	
// side				ENUM		YES	
// type				ENUM		YES	
// timeInForce			ENUM		YES	
// quantity			DECIMAL		YES	
// price				DECIMAL		YES	
// newClientOrderId		STRING		NO		A unique id for the order. Automatically generated by default.
// stopPrice			DECIMAL		NO		Used with STOP orders
// icebergQty			DECIMAL		NO		Used with icebergOrders
// recvWindow			LONG		NO	
// timestamp			LONG		YES	
//
binanceError_t binance::Account::sendTestOrder(Json::Value &json_result, const char *symbol,
	const char *side, const char *type, const char *timeInForce, double quantity, double price,
	const char *newClientOrderId, double stopPrice, double icebergQty, long recvWindow) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<send_order>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/order/test?";

		string action = "POST";
	
		string post_data("symbol=");
		post_data.append(symbol);
	
		post_data.append("&side=");
		post_data.append(side);

		post_data.append("&type=");
		post_data.append(type);

		if (strlen(timeInForce) > 0)
		{
			post_data.append("&timeInForce=");
			post_data.append(timeInForce);
		}

		post_data.append("&quantity=");
		post_data.append(toString(quantity));

		if (price > 0.0)
		{
			post_data.append("&price=");
			post_data.append(toString(price));
		}

		if (strlen(newClientOrderId) > 0)
		{
			post_data.append("&newClientOrderId=");
			post_data.append(newClientOrderId);
		}

		if (stopPrice > 0.0)
		{
			post_data.append("&stopPrice=");
			post_data.append(toString(stopPrice));
		}

		if (icebergQty > 0.0)
		{
			post_data.append("&icebergQty=");
			post_data.append(toString(icebergQty));
		}

		if (recvWindow > 0)
		{
			post_data.append("&recvWindow=");
			post_data.append(to_string(recvWindow));
		}

		post_data.append("&timestamp=");
		post_data.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), post_data.c_str());
		post_data.append("&signature=");
		post_data.append(signature);

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<send_order> url = |%s|, post_data = |%s|", url.c_str(), post_data.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<send_order> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<send_order> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<send_order> Done.\n");

	return status;
}

// Get order (SIGNED)
//
// GET /api/v3/order
//
// Name				Type	Mandatory	Description
// symbol				STRING	YES	
// orderId				LONG	NO	
// origClientOrderId		STRING	NO	
// recvWindow			LONG	NO	
// timestamp			LONG	YES	
//
binanceError_t binance::Account::getOrder(Json::Value &json_result, const char *symbol,
	long orderId, const char *origClientOrderId, long recvWindow)
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_order>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/order?";
		string action = "GET";

		string querystring("symbol=");
		querystring.append(symbol);
	
		if (orderId > 0)
		{
			querystring.append("&orderId=");
			querystring.append(to_string(orderId));
		}

		if (strlen(origClientOrderId) > 0)
		{
			querystring.append("&origClientOrderId=");
			querystring.append(origClientOrderId);
		}

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
	
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string post_data = "";
	
		Logger::write_log("<get_order> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_order> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_order> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_order> Done.\n");

	return status;
}

// DELETE /api/v3/order
//
// cancel order (SIGNED)
//
// symbol				STRING	YES	
// orderId				LONG	NO	
// origClientOrderId		STRING	NO	
// newClientOrderId		STRING	NO	Used to uniquely identify this cancel. Automatically generated by default.
// recvWindow			LONG	NO	
// timestamp			LONG	YES	
//
binanceError_t binance::Account::cancelOrder(Json::Value &json_result, const char *symbol,
	long orderId, const char *origClientOrderId, const char *newClientOrderId, long recvWindow)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<send_order>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/order?";

		string action = "DELETE";
	
		string post_data("symbol=");
		post_data.append(symbol);

		if (orderId > 0)
		{
			post_data.append("&orderId=");
			post_data.append(to_string(orderId));
		}

		if (strlen(origClientOrderId) > 0)
		{
			post_data.append("&origClientOrderId=");
			post_data.append(origClientOrderId);
		}

		if (strlen(newClientOrderId) > 0)
		{
			post_data.append("&newClientOrderId=");
			post_data.append(newClientOrderId);
		}

		if (recvWindow > 0)
		{
			post_data.append("&recvWindow=");
			post_data.append(to_string(recvWindow));
		}

		post_data.append("&timestamp=");
		post_data.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), post_data.c_str());
		post_data.append("&signature=");
		post_data.append(signature);

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<send_order> url = |%s|, post_data = |%s|", url.c_str(), post_data.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<send_order> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<send_order> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<send_order> Done.\n");

	return status;
}

// Start user data stream (API-KEY)
binanceError_t binance::Account::startUserDataStream(Json::Value &json_result) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<start_userDataStream>");

	if (api_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/userDataStream";

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");

		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<start_userDataStream> url = |%s|", url.c_str());
	
		string action = "POST";
		string post_data = "";

		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<start_userDataStream> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<start_userDataStream> Error ! %s", e.what()); 
			}   
		}
	}

	Logger::write_log("<start_userDataStream> Done.\n");

	return status;
}

// Keepalive user data stream (API-KEY)
binanceError_t binance::Account::keepUserDataStream(const char *listenKey) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<keep_userDataStream>");

	if (api_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/userDataStream";

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
	

		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string action = "PUT";
		string post_data("listenKey=");
		post_data.append(listenKey);

		Logger::write_log("<keep_userDataStream> url = |%s|, post_data = |%s|", url.c_str(), post_data.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
	}

	Logger::write_log("<keep_userDataStream> Done.\n");

	return status;
}

// close user data stream (API-KEY)
binanceError_t binance::Account::closeUserDataStream(const char *listenKey) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<close_userDataStream>");

	if (api_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/api/v3/userDataStream";

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");

		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string action = "DELETE";
		string post_data("listenKey=");
		post_data.append(listenKey);

		Logger::write_log("<close_userDataStream> url = |%s|, post_data = |%s|", url.c_str(), post_data.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
	}

	Logger::write_log("<close_userDataStream> Done.\n");

	return status;
}

// Submit a withdraw request.
// 
// POST /wapi/v3/withdraw.html
//
// Name		Type	Mandatory	Description
// asset	STRING	YES
// network	STRING	NO
// address		STRING	YES	
// addressTag	STRING	NO	Secondary address identifier for coins like XRP,XMR etc.
// amount		DECIMAL	YES	
// name		STRING	NO	Description of the address
// recvWindow	LONG	NO	
// timestamp	LONG	YES
//
binanceError_t binance::Account::withdraw(Json::Value &json_result,
	const char *asset, const char *network, const char *address, const char *addressTag,
	double amount, const char *name, long recvWindow) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<withdraw>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/wapi/v3/withdraw.html";

		string action = "POST";
	
		string post_data("asset=");
		post_data.append(asset);

		if (strlen(network) > 0)
		{
			post_data.append("&network=");
			post_data.append(network);
		}

		post_data.append("&address=");
		post_data.append(address);

		if (strlen(addressTag) > 0)
		{
			post_data.append("&addressTag=");
			post_data.append(addressTag);
		}

		post_data.append("&amount=");
		post_data.append(toString(amount));	

		if (strlen(name) > 0)
		{
			post_data.append("&name=");
			post_data.append(name);
		}

		if (recvWindow > 0)
		{
			post_data.append("&recvWindow=");
			post_data.append(to_string(recvWindow));
		}

		post_data.append("&timestamp=");
		post_data.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), post_data.c_str());
		post_data.append("&signature=");
		post_data.append(signature);

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		Logger::write_log("<withdraw> url = |%s|, post_data = |%s|", url.c_str(), post_data.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<withdraw> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<withdraw> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<withdraw> Done.\n");

	return status;
}

// Fetch deposit history.
//
// GET /wapi/v3/depositHistory.html
// 
// Name		Type	Mandatory	Description
// asset		STRING	NO	
// status		INT	NO	0(0:pending,1:success)
// startTime	LONG	NO	
// endTime	LONG		NO	
// recvWindow	LONG	NO	
// timestamp	LONG	YES	
//
binanceError_t binance::Account::getDepositHistory(Json::Value &json_result,
	const char *asset, int istatus, long startTime, long endTime, long recvWindow) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_depostHistory>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/wapi/v3/depositHistory.html?";
		string action = "GET";
	
		string querystring("");

		if (strlen(asset) > 0)
		{
			querystring.append("asset=");
			querystring.append(asset);
		}

		if (istatus > 0)
		{
			querystring.append("&status=");
			querystring.append(to_string(istatus));
		}

		if (startTime > 0)
		{
			querystring.append("&startTime=");
			querystring.append(to_string(startTime));
		}

		if (endTime > 0)
		{
			querystring.append("&endTime=");
			querystring.append(to_string(endTime));
		}

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature = hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
	
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string post_data = "";
	
		Logger::write_log("<get_depostHistory> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_depostHistory> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_depostHistory> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_depostHistory> Done.\n");

	return status;
}

// Fetch withdraw history.
// 
// GET /wapi/v3/withdrawHistory.html
// 
// Name		Type	Mandatory	Description
// asset		STRING	NO	
// status		INT	NO	0(0:Email Sent,1:Cancelled 2:Awaiting Approval 3:Rejected 4:Processing 5:Failure 6Completed)
// startTime	LONG	NO	
// endTime	LONG		NO	
// recvWindow	LONG	NO	
// timestamp	LONG	YES	
//
binanceError_t binance::Account::getWithdrawHistory(Json::Value &json_result,
	const char *asset, int istatus, long startTime, long endTime, long recvWindow) 
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_withdrawHistory>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/wapi/v3/withdrawHistory.html?";
		string action = "GET";
	
		string querystring("");

		if (strlen(asset) > 0)
		{
			querystring.append("asset=");
			querystring.append(asset);
		}

		if (istatus > 0)
		{
			querystring.append("&status=");
			querystring.append(to_string(istatus));
		}

		if (startTime > 0)
		{
			querystring.append("&startTime=");
			querystring.append(to_string(startTime));
		}

		if (endTime > 0)
		{
			querystring.append("&endTime=");
			querystring.append(to_string(endTime));
		}

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
	
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string post_data = "";
	
		Logger::write_log("<get_withdrawHistory> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_withdrawHistory> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_withdrawHistory> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_withdrawHistory> Done.\n");

	return status;
}

// Fetch deposit address.
//
// GET /wapi/v3/depositAddress.html
//
// Name		Type	Mandatory	Description
// asset		STRING	YES	
// recvWindow	LONG	NO	
// timestamp	LONG	YES	
//
binanceError_t binance::Account::getDepositAddress(Json::Value &json_result, const char *asset, long recvWindow) 
{	
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_depositAddress>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/wapi/v3/depositAddress.html?";
		string action = "GET";
	
		string querystring("asset=");
		querystring.append(asset);
	
		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		querystring.append("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);
	
		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string post_data = "";
	
		Logger::write_log("<get_depositAddress> url = |%s|", url.c_str());
	
		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_depositAddress> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
			 	Logger::write_log("<get_depositAddress> Error ! %s", e.what()); 
			}   
		}
	}
	
	Logger::write_log("<get_depositAddress> Done.\n");

	return status;
}

binanceError_t binance::Account::getWalletData(Json::Value &json_result, long recvWindow)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_walletData>");

	if (api_key.size() == 0 || secret_key.size() == 0)
		status = binanceErrorMissingAccountKeys;
	else
	{
		string url(hostname);
		url += "/sapi/v1/capital/config/getall?";
		string action = "GET";

		string querystring("&timestamp=");
		querystring.append(to_string(get_current_ms_epoch()));

		if (recvWindow > 0)
		{
			querystring.append("&recvWindow=");
			querystring.append(to_string(recvWindow));
		}

		string signature =  hmac_sha256(secret_key.c_str(), querystring.c_str());
		querystring.append("&signature=");
		querystring.append(signature);

		url.append(querystring);

		vector <string> extra_http_header;
		string header_chunk("X-MBX-APIKEY: ");
		header_chunk.append(api_key);
		extra_http_header.push_back(header_chunk);

		string post_data = "";

		Logger::write_log("<get_walletData> url = |%s|", url.c_str());

		string str_result;
		Server::getCurlWithHeader(str_result, url, extra_http_header, post_data, action);

		if (str_result.size() == 0)
			status = binanceErrorEmptyServerResponse;
		else
		{
			try
			{
				json_result.clear();
				JSONCPP_STRING err;
				Json::CharReaderBuilder builder;
				const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
				if (!reader->parse(str_result.c_str(), str_result.c_str() + str_result.length(), &json_result,
								   &err)) {
					Logger::write_log("<get_walletData> Error ! %s", err.c_str());
					status = binanceErrorParsingServerResponse;
					return status;
				}
				CHECK_SERVER_ERR(json_result);
			}
			catch (exception &e)
			{
				Logger::write_log("<get_walletData> Error ! %s", e.what());
			}
		}
	}
	
  return status;
}
