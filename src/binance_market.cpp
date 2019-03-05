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

binance::Market::Market(const binance::Server& server_) : hostname(server_.getHostname()), server(server_) { }

// Get Latest price for all symbols.
// GET /api/v1/ticker/allPrices
binanceError_t binance::Market::getAllPrices(Json::Value &json_result)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_allPrices>");

	string url(hostname);
	url += "/api/v1/ticker/allPrices";

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_allPrices> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_allPrices> Done.");

	return status;
}

// Get Single Pair's Price
binanceError_t binance::Market::getPrice(const char *symbol, double& price)
{
	Logger::write_log("<get_price>");

	Json::Value alltickers;
	string str_symbol = string_toupper(symbol);
	binanceError_t status = getAllPrices(alltickers);

	if (status == binanceSuccess)
	{
		status = binanceErrorInvalidSymbol;
		for (int i = 0;i < alltickers.size();i++)
		{
			if (alltickers[i]["symbol"].asString() == str_symbol)
			{
				price = atof(alltickers[i]["price"].asString().c_str());
				status = binanceSuccess;
				break;
			}
		}
	}

	Logger::write_log("<get_price> Done.");

	return status;
}

// Get Best price/qty on the order book for all symbols.
// GET /api/v1/ticker/allBookTickers
binanceError_t binance::Market::getAllBookTickers(Json::Value &json_result)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_allBookTickers>");

	string url(hostname);
	url += "/api/v1/ticker/allBookTickers";

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_allBookTickers> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_allBookTickers> Done.");

	return status;
}

binanceError_t binance::Market::getBookTicker(Json::Value &json_result, const char *symbol)
{
	Logger::write_log("<get_BookTickers>");

	Json::Value alltickers;
	string str_symbol = string_toupper(symbol);
	binanceError_t status = getAllBookTickers(alltickers);

	if (status == binanceSuccess)
	{
		status = binanceErrorInvalidSymbol;
		for (int i = 0;i < alltickers.size();i++)
		{
			if (alltickers[i]["symbol"].asString() == str_symbol)
			{
				json_result = alltickers[i];
				status = binanceSuccess;
				break;
			}
		}
	}

	Logger::write_log("<get_BookTickers> Done.");

	return status;
}

// Get Market Depth
//
// GET /api/v1/depth
//
// Name	Type		Mandatory	Description
// symbol	STRING		YES
// limit	INT		NO		Default 100;max 100.
//
binanceError_t binance::Market::getDepth(Json::Value &json_result, const char *symbol, int limit)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_depth>");

	string url(hostname);
	url += "/api/v1/depth?";

	string querystring("symbol=");
	querystring.append(symbol);
	querystring.append("&limit=");
	querystring.append(to_string(limit));

	url.append(querystring);
	Logger::write_log("<get_depth> url = |%s|", url.c_str());

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_depth> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_depth> Done.");

	return status;
}

// Get Aggregated Trades list
//
// GET /api/v1/aggTrades
//
// Name		Type	Mandatory	Description
// symbol		STRING	YES
// fromId		LONG	NO		ID to get aggregate trades from INCLUSIVE.
// startTime	LONG	NO		Timestamp in ms to get aggregate trades from INCLUSIVE.
// endTime		LONG	NO		Timestamp in ms to get aggregate trades until INCLUSIVE.
// limit		INT	NO		Default 500;max 500.
//
binanceError_t binance::Market::getAggTrades(Json::Value &json_result, const char *symbol, int fromId, int limit)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_aggTrades>");

	string url(hostname);
	url += "/api/v1/aggTrades?";

	string querystring("symbol=");
	querystring.append(symbol);

	querystring.append("&fromId=");
	querystring.append(to_string(fromId));

	querystring.append("&limit=");
	querystring.append(to_string(limit));

	url.append(querystring);
	Logger::write_log("<get_aggTrades> url = |%s|", url.c_str());

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_aggTrades> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_aggTrades> Done.");

	return status;
}

// Get Aggregated Trades list
//
// GET /api/v1/aggTrades
//
// Name		Type	Mandatory	Description
// symbol		STRING	YES
// fromId		LONG	NO		ID to get aggregate trades from INCLUSIVE.
// startTime	LONG	NO		Timestamp in ms to get aggregate trades from INCLUSIVE.
// endTime		LONG	NO		Timestamp in ms to get aggregate trades until INCLUSIVE.
// limit		INT	NO		Default 500;max 500.
//
binanceError_t binance::Market::getAggTrades(Json::Value &json_result, const char *symbol, time_t startTime, time_t endTime, int limit)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_aggTrades>");

	string url(hostname);
	url += "/api/v1/aggTrades?";

	string querystring("symbol=");
	querystring.append(symbol);

	querystring.append("&startTime=");
	querystring.append(to_string(startTime));

	querystring.append("&endTime=");
	querystring.append(to_string(endTime));

	querystring.append("&limit=");
	querystring.append(to_string(limit));

	url.append(querystring);
	Logger::write_log("<get_aggTrades> url = |%s|", url.c_str());

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_aggTrades> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_aggTrades> Done.");

	return status;
}

// Get 24hr ticker price change statistics
//
// Name	Type	Mandatory	Description
// symbol	STRING	YES
//
binanceError_t binance::Market::get24hr(Json::Value &json_result, const char *symbol)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_24hr>");

	string url(hostname);
	url += "/api/v1/ticker/24hr?";

	string querystring("symbol=");
	querystring.append(symbol);

	url.append(querystring);
	Logger::write_log("<get_24hr> url = |%s|", url.c_str());

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_24hr> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_24hr> Done.");

	return status;
}

// Get KLines(Candle stick / OHLC)
//
// GET /api/v1/klines
//
// Name		Type	Mandatory	Description
// symbol		STRING	YES
// interval	ENUM	YES
// startTime	LONG	NO
// endTime		LONG	NO
// limit		INT		NO	Default 500;max 500.
//
binanceError_t binance::Market::getKlines(Json::Value &json_result, const char *symbol, const char *interval, time_t startTime, time_t endTime, int limit)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_klines>");

	string url(hostname);
	url += "/api/v1/klines?";

	string querystring("symbol=");
	querystring.append(symbol);

	querystring.append("&interval=");
	querystring.append(interval);

	if (startTime > 0 && endTime > 0)
	{
		querystring.append("&startTime=");
		querystring.append(to_string(startTime));

		querystring.append("&endTime=");
		querystring.append(to_string(endTime));

	}

	querystring.append("&limit=");
	querystring.append(to_string(limit));

	url.append(querystring);
	Logger::write_log("<get_klines> url = |%s|", url.c_str());

	string str_result;
	Server::getCurl(str_result, url);

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
		 	Logger::write_log("<get_klines> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_klines> Done.");

	return status;
}

// Get Exchange information
//GET /api/v1/exchangeInfo
binanceError_t binance::Market::getExchangeInfo(Json::Value &json_result)
{
	binanceError_t status = binanceSuccess;

	Logger::write_log("<get_exchangeInfo>");

	string url(hostname);
	url += "/api/v1/exchangeInfo";

	string str_result;
	Server::getCurl(str_result, url);

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
			Logger::write_log("<get_exchangeInfo> Error ! %s", e.what());
			status = binanceErrorParsingServerResponse;
		}
	}

	Logger::write_log("<get_exchangeInfo> Done.");

	return status;
}

//LOT_SIZE
//The LOT_SIZE filter defines the quantity (aka "lots" in auction terms) rules for a symbol. There are 3 parts:
//minQty defines the minimum quantity/icebergQty allowed.
//maxQty defines the maximum quantity/icebergQty allowed.
//stepSize defines the intervals that a quantity/icebergQty can be increased/decreased by.
//In order to pass the lot size, the following must be true for quantity/icebergQty:
//quantity >= minQty
//quantity <= maxQty
//(quantity-minQty) % stepSize == 0
binanceError_t binance::Market::getLotSize(const char *symbol, double& maxQty, double& minQty, double& stepSize)
{
	Logger::write_log("<get_lotSize>");

	Json::Value exchangeInfo;
	string str_symbol = string_toupper(symbol);
	binanceError_t status = getExchangeInfo(exchangeInfo);
	CHECK_SERVER_ERR(exchangeInfo);

	if (status == binanceSuccess)
	{
		status = binanceErrorInvalidSymbol;
		
		for (int i = 0;i < exchangeInfo["symbols"].size();i++)
		{
			if (exchangeInfo["symbols"][i]["symbol"].asString() == str_symbol)
			{
				for(int j=0;j < exchangeInfo["symbols"][i]["filters"].size(); j++) {
					if (exchangeInfo["symbols"][i]["filters"][j]["filterType"].asString() == "LOT_SIZE") {
						maxQty = atof(exchangeInfo["symbols"][i]["filters"][j]["maxQty"].asString().c_str());
						minQty = atof(exchangeInfo["symbols"][i]["filters"][j]["minQty"].asString().c_str());
						stepSize = atof(exchangeInfo["symbols"][i]["filters"][j]["stepSize"].asString().c_str());
						status = binanceSuccess;
						break;
					}
				}
			}
		}
	}

	Logger::write_log("<get_lotSize> Done.");

	return status;
}
