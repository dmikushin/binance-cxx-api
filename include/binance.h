/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#ifndef BINANCE_H
#define BINANCE_H

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <jsoncpp/json/json.h>

#define BINANCE_HOST "https://api.binance.com"

namespace binance
{
	extern std::string api_key;
	extern std::string secret_key;

	template<typename T> std::string to_string( const T& val )
	{
		std::ostringstream out;
		out << val;
		return out.str();
	}

	std::string to_string( double val )
	{
		std::ostringstream out;
		out.precision(8);
		out.setf(std::ios_base::fixed);
		out << val;
		return out.str();
	}

	void curl_api( std::string &url, std::string &result_json );

	void curl_api_with_header( std::string &url, std::string &result_json,
		std::vector <std::string> &extra_http_header, std::string &post_data, std::string &action );

	size_t curl_cb( void *content, size_t size, size_t nmemb, std::string *buffer ) ;
	
	void init( std::string &api_key, std::string &secret_key );

	// Public API
	void get_serverTime( Json::Value &json_result); 	

	void get_allPrices( Json::Value &json_result );
	double get_price( const char *symbol );

	void get_allBookTickers( Json::Value &json_result );
	void get_bookTicker( const char *symbol, Json::Value &json_result ) ;

	void get_depth( const char *symbol, int limit, Json::Value &json_result );
	void get_aggTrades( const char *symbol, int fromId, time_t startTime, time_t endTime, int limit, Json::Value &json_result ); 
	void get_24hr( const char *symbol, Json::Value &json_result ); 
	void get_klines( const char *symbol, const char *interval, int limit, time_t startTime, time_t endTime,  Json::Value &json_result );

	// API + Secret keys required
	void get_account( long recvWindow , Json::Value &json_result );
	
	void get_myTrades( 
		const char *symbol, 
		int limit,
		long fromId,
		long recvWindow, 
		Json::Value &json_result 
	);
	
	void get_openOrders(  
		const char *symbol, 
		long recvWindow,   
		Json::Value &json_result 
	) ;
	
	void get_allOrders(   
		const char *symbol, 
		long orderId,
		int limit,
		long recvWindow,
		Json::Value &json_result 
	);

	void send_order( 
		const char *symbol, 
		const char *side,
		const char *type,
		const char *timeInForce,
		double quantity,
		double price,
		const char *newClientOrderId,
		double stopPrice,
		double icebergQty,
		long recvWindow,
		Json::Value &json_result ) ;

	void get_order( 
		const char *symbol, 
		long orderId,
		const char *origClientOrderId,
		long recvWindow,
		Json::Value &json_result ); 

	void cancel_order( 
		const char *symbol, 
		long orderId,
		const char *origClientOrderId,
		const char *newClientOrderId,
		long recvWindow,
		Json::Value &json_result 
	);

	// API key required
	void start_userDataStream( Json::Value &json_result );
	void keep_userDataStream( const char *listenKey  );
	void close_userDataStream( const char *listenKey );

	// WAPI
	void withdraw( 
		const char *asset,
		const char *address,
		const char *addressTag,
		double amount, 
		const char *name,
		long recvWindow,
		Json::Value &json_result );

	void get_depositHistory( 
		const char *asset,
		int  status,
		long startTime,
		long endTime, 
		long recvWindow,
		Json::Value &json_result );

	void get_withdrawHistory( 
		const char *asset,
		int  status,
		long startTime,
		long endTime, 
		long recvWindow,
		Json::Value &json_result ); 

	void get_depositAddress( 
		const char *asset,
		long recvWindow,
		Json::Value &json_result );
};

#endif // BINANCE_H

