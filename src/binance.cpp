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

class BinanceGlobalInit
{
public :

	BinanceGlobalInit()
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}
	
	~BinanceGlobalInit()
	{
		curl_global_cleanup();
	}
};

// Initialize globals.
static BinanceGlobalInit binanceGlobalInit;

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

	static const string str_binanceErrorUnknown = "binanceErrorUnknown";
	return str_binanceErrorUnknown.c_str();
}

std::string binance::toString(double val)
{
	std::ostringstream out;
	out.precision(8);
	out.setf(std::ios_base::fixed);
	out << val;
	return out.str();
}

