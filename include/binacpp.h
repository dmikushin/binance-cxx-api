
/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/


#ifndef BINACPP_H
#define BINACPP_H


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <exception>

#include <curl/curl.h>
#include <jsoncpp/json/json.h>



#define BINANCE_HOST "https://api.binance.com"


using namespace std;

class BinaCPP {

	static string api_key;
	static string secret_key;


	

	public:

		template<typename T> static std::string to_string(T val)
		{
			std::ostringstream out;
			out << val;
			return out.str();
		}

		static std::string to_string(double val)
		{
			std::ostringstream out;
			out.precision(8);
			out.setf(std::ios_base::fixed);
			out << val;
			return out.str();
		}

		static void curl_api( string &url, string &result_json );
		static void curl_api_with_header( string &url, string &result_json , vector <string> &extra_http_header, string &post_data, string &action );
		static size_t curl_cb( void *content, size_t size, size_t nmemb, string *buffer ) ;
		
		static void init( string &api_key, string &secret_key);


		// Public API
		static void get_serverTime( Json::Value &json_result); 	

		static void get_allPrices( Json::Value &json_result );
		static double get_price( const char *symbol );

		static void get_allBookTickers( Json::Value &json_result );
		static void get_bookTicker( const char *symbol, Json::Value &json_result ) ;

		static void get_depth( const char *symbol, int limit, Json::Value &json_result );
		static void get_aggTrades( const char *symbol, int fromId, time_t startTime, time_t endTime, int limit, Json::Value &json_result ); 
		static void get_24hr( const char *symbol, Json::Value &json_result ); 
		static void get_klines( const char *symbol, const char *interval, int limit, time_t startTime, time_t endTime,  Json::Value &json_result );


		// API + Secret keys required
		static void get_account( long recvWindow , Json::Value &json_result );
		
		static void get_myTrades( 
			const char *symbol, 
			int limit,
			long fromId,
			long recvWindow, 
			Json::Value &json_result 
		);
		
		static void get_openOrders(  
			const char *symbol, 
			long recvWindow,   
			Json::Value &json_result 
		) ;
		

		static void get_allOrders(   
			const char *symbol, 
			long orderId,
			int limit,
			long recvWindow,
			Json::Value &json_result 
		);


		static void send_order( 
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


		static void get_order( 
			const char *symbol, 
			long orderId,
			const char *origClientOrderId,
			long recvWindow,
			Json::Value &json_result ); 


		static void cancel_order( 
			const char *symbol, 
			long orderId,
			const char *origClientOrderId,
			const char *newClientOrderId,
			long recvWindow,
			Json::Value &json_result 
		);

		// API key required
		static void start_userDataStream( Json::Value &json_result );
		static void keep_userDataStream( const char *listenKey  );
		static void close_userDataStream( const char *listenKey );


		// WAPI
		static void withdraw( 
			const char *asset,
			const char *address,
			const char *addressTag,
			double amount, 
			const char *name,
			long recvWindow,
			Json::Value &json_result );

		static void get_depositHistory( 
			const char *asset,
			int  status,
			long startTime,
			long endTime, 
			long recvWindow,
			Json::Value &json_result );

		static void get_withdrawHistory( 
			const char *asset,
			int  status,
			long startTime,
			long endTime, 
			long recvWindow,
			Json::Value &json_result ); 

		static void get_depositAddress( 
			const char *asset,
			long recvWindow,
			Json::Value &json_result );


};


#endif
