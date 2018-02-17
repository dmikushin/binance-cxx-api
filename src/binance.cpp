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

string binance::api_key = "";
string binance::secret_key = "";

void binance::init( string &api_key, string &secret_key ) 
{
	api_key = api_key;
	secret_key = secret_key;
}

// GET /api/v1/time
void binance::get_serverTime( Json::Value &json_result) 
{
	Logger::write_log( "<get_serverTime>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/time";

	string str_result;
	curl_api( url, str_result );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
	    		
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_serverTime> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_serverTime> Done." );
	}
	else
	{
		Logger::write_log( "<get_serverTime> Failed to get anything." );
	}
}

// Get Latest price for all symbols.
// GET /api/v1/ticker/allPrices
void binance::get_allPrices( Json::Value &json_result ) 
{	

	Logger::write_log( "<get_allPrices>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/ticker/allPrices";

	string str_result;
	curl_api( url, str_result );

	if ( str_result.size() > 0 ) {
		
		try {
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
	    		
		} catch ( exception &e ) {
		 	Logger::write_log( "<get_allPrices> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_allPrices> Done." );
	
	} else {
		Logger::write_log( "<get_allPrices> Failed to get anything." );
	}
}

// Get Single Pair's Price
double binance::get_price( const char *symbol )
{
	Logger::write_log( "<get_price>" );

	double ret = 0.0;
	Json::Value alltickers;
	string str_symbol = string_toupper(symbol);
	get_allPrices( alltickers );

	for ( int i = 0 ; i < alltickers.size(); i++ ) {
		if ( alltickers[i]["symbol"].asString() == str_symbol ) {
			ret = atof( alltickers[i]["price"].asString().c_str() );
			break;
		}
		
	}	
	return ret;
}

// Get Best price/qty on the order book for all symbols.
// GET /api/v1/ticker/allBookTickers
void binance::get_allBookTickers(  Json::Value &json_result ) 
{	
	Logger::write_log( "<get_allBookTickers>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/ticker/allBookTickers";

	string str_result;
	curl_api( url, str_result );

	if ( str_result.size() > 0 ) {
		
		try {
			Json::Reader reader;
			json_result.clear();	
	    		reader.parse( str_result , json_result );
	    		
	    	} catch ( exception &e ) {
		 	Logger::write_log( "<get_allBookTickers> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_allBookTickers> Done." );
	
	} else {
		Logger::write_log( "<get_allBookTickers> Failed to get anything." );
	}
}

void binance::get_bookTicker( const char *symbol, Json::Value &json_result ) 
{
	Logger::write_log( "<get_BookTickers>" );

	Json::Value alltickers;
	string str_symbol = string_toupper(symbol);
	get_allBookTickers( alltickers );

	for ( int i = 0 ; i < alltickers.size(); i++ ) {
		if ( alltickers[i]["symbol"].asString() == str_symbol ) {
			
			json_result = alltickers[i];
			
			break;
		}
		
	}		
}

//--------------------
// Get Market Depth
//
// GET /api/v1/depth
// 
// Name	Type		Mandatory	Description
// symbol	STRING		YES	
// limit	INT		NO		Default 100; max 100.
//
void binance::get_depth( const char *symbol, int limit, Json::Value &json_result ) 
{	
	Logger::write_log( "<get_depth>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/depth?";

	string querystring("symbol=");
	querystring.append( symbol );
	querystring.append("&limit=");
	querystring.append( to_string( limit ) );

	url.append( querystring );
	Logger::write_log( "<get_depth> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api( url, str_result );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );	
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_depth> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_depth> Done." );
	}
	else
	{
		Logger::write_log( "<get_depth> Failed to get anything." );
	}
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
// limit		INT	NO		Default 500; max 500.
//
void binance::get_aggTrades( const char *symbol, int fromId, time_t startTime, time_t endTime, int limit, Json::Value &json_result ) 
{
	Logger::write_log( "<get_aggTrades>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/aggTrades?";

	string querystring("symbol=");
	querystring.append( symbol );

	if ( startTime != 0 && endTime != 0 )
	{
		querystring.append("&startTime=");
		querystring.append( to_string( startTime ) );

		querystring.append("&endTime=");
		querystring.append( to_string( endTime ) );
	}
	else
	{
		querystring.append("&fromId=");
		querystring.append( to_string( fromId ) );

		querystring.append("&limit=");
		querystring.append( to_string( limit ) );
	}

	url.append( querystring );
	Logger::write_log( "<get_aggTrades> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api( url, str_result );
	
	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );	
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_aggTrades> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_aggTrades> Done." );
	}
	else
	{
		Logger::write_log( "<get_aggTrades> Failed to get anything." );
	}
}

// Get 24hr ticker price change statistics
//
// Name	Type	Mandatory	Description
// symbol	STRING	YES	
//
void binance::get_24hr( const char *symbol, Json::Value &json_result ) 
{	
	Logger::write_log( "<get_24hr>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/ticker/24hr?";

	string querystring("symbol=");
	querystring.append( symbol );

	url.append( querystring );
	Logger::write_log( "<get_24hr> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api( url, str_result );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );	
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_24hr> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_24hr> Done." );
	}
	else
	{
		Logger::write_log( "<get_24hr> Failed to get anything." );
	}
}

// Get KLines( Candle stick / OHLC )
// 
// GET /api/v1/klines
// 
// Name		Type	Mandatory	Description
// symbol		STRING	YES	
// interval	ENUM	YES	
// limit		INT		NO	Default 500; max 500.
// startTime	LONG	NO	
// endTime		LONG	NO	
//
void binance::get_klines( const char *symbol, const char *interval, int limit, time_t startTime, time_t endTime, Json::Value &json_result ) 
{		
	Logger::write_log( "<get_klines>" );

	string url(BINANCE_HOST);  
	url += "/api/v1/klines?";

	string querystring("symbol=");
	querystring.append( symbol );

	querystring.append( "&interval=" );
	querystring.append( interval );

	if ( startTime > 0 && endTime > 0 )
	{
		querystring.append("&startTime=");
		querystring.append( to_string( startTime ) );

		querystring.append("&endTime=");
		querystring.append( to_string( endTime ) );
	
	}
	else if ( limit > 0 )
	{
		querystring.append("&limit=");
		querystring.append( to_string( limit ) );
	}
	
	url.append( querystring );
	Logger::write_log( "<get_klines> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api( url, str_result );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
	    		
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_klines> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_klines> Done." );
	}
	else
	{
		Logger::write_log( "<get_klines> Failed to get anything." );
	}
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
void binance::get_account( long recvWindow,  Json::Value &json_result ) 
{	
	Logger::write_log( "<get_account>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_account> API Key and Secret Key has not been set." );
		return ;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/account?";
	string action = "GET";

	string querystring("timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow ) );
	}

	string signature =  hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	Logger::write_log( "<get_account> url = |%s|" , url.c_str() );
	
	string post_data = "";
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header , post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );	
    	}
    	catch ( exception &e )
    	{
		 	Logger::write_log( "<get_account> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_account> Done." );
	}
	else
	{
		Logger::write_log( "<get_account> Failed to get anything." );
	}

	Logger::write_log( "<get_account> Done.\n" );
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
void binance::get_myTrades( const char *symbol, int limit, long fromId, long recvWindow, Json::Value &json_result )
{	
	Logger::write_log( "<get_myTrades>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_myTrades> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/myTrades?";

	string querystring("symbol=");
	querystring.append( symbol );

	if ( limit > 0 )
	{
		querystring.append("&limit=");
		querystring.append( to_string( limit ) );
	}

	if ( fromId > 0 )
	{
		querystring.append("&fromId=");
		querystring.append( to_string( fromId ) );
	}

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow ) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str() , querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	Logger::write_log( "<get_myTrades> url = |%s|" , url.c_str() );
	
	string action = "GET";
	string post_data = "";

	string str_result;
	curl_api_with_header( url, str_result , extra_http_header , post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_myTrades> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_myTrades> Done." );
	}
	else
	{
		Logger::write_log( "<get_myTrades> Failed to get anything." );
	}

	Logger::write_log( "<get_myTrades> Done.\n" );
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
void binance::get_openOrders( const char *symbol, long recvWindow, Json::Value &json_result ) 
{	
	Logger::write_log( "<get_openOrders>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_openOrders> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/openOrders?";

	string querystring("symbol=");
	querystring.append( symbol );

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);
	
	string action = "GET";
	string post_data ="";
		
	Logger::write_log( "<get_openOrders> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );		
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_openOrders> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_openOrders> Done." );
	}
	else
	{
		Logger::write_log( "<get_openOrders> Failed to get anything." );
	}
	
	Logger::write_log( "<get_openOrders> Done.\n" );
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
void binance::get_allOrders( const char *symbol, long orderId, int limit, long recvWindow, Json::Value &json_result ) 
{
	Logger::write_log( "<get_allOrders>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_allOrders> API Key and Secret Key has not been set." );
		return ;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/allOrders?";

	string querystring("symbol=");
	querystring.append( symbol );

	if ( orderId > 0 )
	{
		querystring.append("&orderId=");
		querystring.append( to_string( orderId ) );
	}

	if ( limit > 0 )
	{
		querystring.append("&limit=");
		querystring.append( to_string( limit ) );
	}

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow ) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);
	
	string action = "GET";
	string post_data ="";
		
	Logger::write_log( "<get_allOrders> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_allOrders> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_allOrders> Done." );
	}
	else
	{
		Logger::write_log( "<get_allOrders> Failed to get anything." );
	}
	
	Logger::write_log( "<get_allOrders> Done.\n" );
}

// Send order (SIGNED)
//
// POST /api/v3/order
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
void binance::send_order( const char *symbol, const char *side, const char *type, const char *timeInForce, double quantity,
	double price, const char *newClientOrderId, double stopPrice, double icebergQty, long recvWindow, Json::Value &json_result ) 
{	
	Logger::write_log( "<send_order>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<send_order> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/order?";

	string action = "POST";
	
	string post_data("symbol=");
	post_data.append( symbol );
	
	post_data.append("&side=");
	post_data.append( side );

	post_data.append("&type=");
	post_data.append( type );

	post_data.append("&timeInForce=");
	post_data.append( timeInForce );

	post_data.append("&quantity=");
	post_data.append( to_string( quantity) );

	post_data.append("&price=");
	post_data.append( to_string( price) );

	if ( strlen( newClientOrderId ) > 0 )
	{
		post_data.append("&newClientOrderId=");
		post_data.append( newClientOrderId );
	}

	if ( stopPrice > 0.0 )
	{
		post_data.append("&stopPrice=");
		post_data.append( to_string( stopPrice ) );
	}

	if ( icebergQty > 0.0 )
	{
		post_data.append("&icebergQty=");
		post_data.append( to_string( icebergQty ) );
	}

	if ( recvWindow > 0 )
	{
		post_data.append("&recvWindow=");
		post_data.append( to_string( recvWindow) );
	}

	post_data.append("&timestamp=");
	post_data.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), post_data.c_str() );
	post_data.append( "&signature=");
	post_data.append( signature );

	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	Logger::write_log( "<send_order> url = |%s|, post_data = |%s|" , url.c_str(), post_data.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data, action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<send_order> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<send_order> Done." );
	}
	else
	{
		Logger::write_log( "<send_order> Failed to get anything." );
	}
	
	Logger::write_log( "<send_order> Done.\n" );
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
void binance::get_order( const char *symbol, long orderId, const char *origClientOrderId, long recvWindow, Json::Value &json_result )
{	
	Logger::write_log( "<get_order>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_order> API Key and Secret Key has not been set." );
		return ;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/order?";
	string action = "GET";

	string querystring("symbol=");
	querystring.append( symbol );
	
	if ( orderId > 0 )
	{
		querystring.append("&orderId=");
		querystring.append( to_string( orderId ) );
	}

	if ( strlen( origClientOrderId ) > 0 )
	{
		querystring.append("&origClientOrderId=");
		querystring.append( origClientOrderId );
	}

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	string post_data = "";
	
	Logger::write_log( "<get_order> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
	    }
	    catch ( exception &e )
	    {
		 	Logger::write_log( "<get_order> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_order> Done." );
	}
	else
	{
		Logger::write_log( "<get_order> Failed to get anything." );
	}
	
	Logger::write_log( "<get_order> Done.\n" );
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
void binance::cancel_order( const char *symbol, long orderId, const char *origClientOrderId,
	const char *newClientOrderId, long recvWindow, Json::Value &json_result )
{
	Logger::write_log( "<send_order>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<send_order> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/api/v3/order?";

	string action = "DELETE";
	
	string post_data("symbol=");
	post_data.append( symbol );

	if ( orderId > 0 )
	{
		post_data.append("&orderId=");
		post_data.append( to_string( orderId ) );
	}

	if ( strlen( origClientOrderId ) > 0 )
	{
		post_data.append("&origClientOrderId=");
		post_data.append( origClientOrderId );
	}

	if ( strlen( newClientOrderId ) > 0 )
	{
		post_data.append("&newClientOrderId=");
		post_data.append( newClientOrderId );
	}

	if ( recvWindow > 0 )
	{
		post_data.append("&recvWindow=");
		post_data.append( to_string( recvWindow) );
	}

	post_data.append("&timestamp=");
	post_data.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), post_data.c_str() );
	post_data.append( "&signature=");
	post_data.append( signature );

	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	Logger::write_log( "<send_order> url = |%s|, post_data = |%s|" , url.c_str(), post_data.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data, action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<send_order> Error ! %s", e.what() ); 
		}   

		Logger::write_log( "<send_order> Done." );
	}
	else
	{
		Logger::write_log( "<send_order> Failed to get anything." );
	}
	
	Logger::write_log( "<send_order> Done.\n" );
}

// Start user data stream (API-KEY)
void binance::start_userDataStream( Json::Value &json_result ) 
{	
	Logger::write_log( "<start_userDataStream>" );

	if ( api_key.size() == 0 )
	{
		Logger::write_log( "<start_userDataStream> API Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/api/v1/userDataStream";

	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");

	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	Logger::write_log( "<start_userDataStream> url = |%s|" , url.c_str() );
	
	string action = "POST";
	string post_data = "";

	string str_result;
	curl_api_with_header( url, str_result , extra_http_header , post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
	    		
	    	} catch ( exception &e ) {
		 	Logger::write_log( "<start_userDataStream> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<start_userDataStream> Done." );
	}
	else
	{
		Logger::write_log( "<start_userDataStream> Failed to get anything." );
	}

	Logger::write_log( "<start_userDataStream> Done.\n" );
}

// Keepalive user data stream (API-KEY)
void binance::keep_userDataStream( const char *listenKey ) 
{	
	Logger::write_log( "<keep_userDataStream>" );

	if ( api_key.size() == 0 )
	{
		Logger::write_log( "<keep_userDataStream> API Key has not been set." );
		return ;
	}

	string url(BINANCE_HOST);
	url += "/api/v1/userDataStream";

	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	

	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	string action = "PUT";
	string post_data("listenKey=");
	post_data.append( listenKey );

	Logger::write_log( "<keep_userDataStream> url = |%s|, post_data = |%s|" , url.c_str() , post_data.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result, extra_http_header, post_data, action );

	if ( str_result.size() > 0 )
	{
		Logger::write_log( "<keep_userDataStream> Done." );
	}
	else
	{
		Logger::write_log( "<keep_userDataStream> Failed to get anything." );
	}

	Logger::write_log( "<keep_userDataStream> Done.\n" );
}

// Keepalive user data stream (API-KEY)
void binance::close_userDataStream( const char *listenKey ) 
{	
	Logger::write_log( "<close_userDataStream>" );

	if ( api_key.size() == 0 )
	{
		Logger::write_log( "<close_userDataStream> API Key has not been set." );
		return ;
	}

	string url(BINANCE_HOST);
	url += "/api/v1/userDataStream";

	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");

	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	string action = "DELETE";
	string post_data("listenKey=");
	post_data.append( listenKey );

	Logger::write_log( "<close_userDataStream> url = |%s|, post_data = |%s|" , url.c_str() , post_data.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header , post_data , action );

	if ( str_result.size() > 0 ) {
		
		Logger::write_log( "<close_userDataStream> Done." );
	
	} else {
		Logger::write_log( "<close_userDataStream> Failed to get anything." );
	}

	Logger::write_log( "<close_userDataStream> Done.\n" );

}

// Submit a withdraw request.
// 
// POST /wapi/v3/withdraw.html
//
// Name		Type	Mandatory	Description
// asset		STRING	YES	
// address		STRING	YES	
// addressTag	STRING	NO	Secondary address identifier for coins like XRP,XMR etc.
// amount		DECIMAL	YES	
// name		STRING	NO	Description of the address
// recvWindow	LONG	NO	
// timestamp	LONG	YES
//
void binance::withdraw( const char *asset, const char *address, const char *addressTag,
	double amount, const char *name, long recvWindow, Json::Value &json_result ) 
{	
	Logger::write_log( "<withdraw>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<send_order> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/wapi/v3/withdraw.html";

	string action = "POST";
	
	string post_data("asset=");
	post_data.append( asset );
	
	post_data.append("&address=" );
	post_data.append( address );

	if ( strlen(addressTag) > 0 )
	{
		post_data.append( "&addressTag=");
		post_data.append( addressTag );
	}

	post_data.append( "&amount=");
	post_data.append( to_string( amount ));	

	if ( strlen( name ) > 0 )
	{
		post_data.append("&name=");
		post_data.append(name);
	}

	if ( recvWindow > 0 )
	{
		post_data.append("&recvWindow=");
		post_data.append( to_string( recvWindow) );
	}

	post_data.append("&timestamp=");
	post_data.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), post_data.c_str() );
	post_data.append( "&signature=");
	post_data.append( signature );

	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	Logger::write_log( "<withdraw> url = |%s|, post_data = |%s|" , url.c_str(), post_data.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data, action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<withdraw> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<withdraw> Done." );
	}
	else
	{
		Logger::write_log( "<withdraw> Failed to get anything." );
	}
	
	Logger::write_log( "<withdraw> Done.\n" );
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
void binance::get_depositHistory(
	const char *asset, int status, long startTime, long endTime, long recvWindow, Json::Value &json_result ) 
{	
	Logger::write_log( "<get_depostHistory>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_depostHistory> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/wapi/v3/depositHistory.html?";
	string action = "GET";
	
	string querystring("");

	if ( strlen( asset ) > 0 )
	{
		querystring.append( "asset=" );
		querystring.append( asset );
	}

	if ( status > 0 )
	{
		querystring.append("&status=");
		querystring.append( to_string( status ) );
	}

	if ( startTime > 0 )
	{
		querystring.append("&startTime=");
		querystring.append( to_string( startTime ) );
	}

	if ( endTime > 0 )
	{
		querystring.append("&endTime=");
		querystring.append( to_string( endTime ) );
	}

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature = hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	string post_data = "";
	
	Logger::write_log( "<get_depostHistory> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );	
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_depostHistory> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_depostHistory> Done." );
	}
	else
	{
		Logger::write_log( "<get_depostHistory> Failed to get anything." );
	}
	
	Logger::write_log( "<get_depostHistory> Done.\n" );
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
void binance::get_withdrawHistory(
	const char *asset, int status, long startTime, long endTime, long recvWindow, Json::Value &json_result ) 
{
	Logger::write_log( "<get_withdrawHistory>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_withdrawHistory> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/wapi/v3/withdrawHistory.html?";
	string action = "GET";
	
	string querystring("");

	if ( strlen( asset ) > 0 )
	{
		querystring.append( "asset=" );
		querystring.append( asset );
	}

	if ( status > 0 )
	{
		querystring.append("&status=");
		querystring.append( to_string( status ) );
	}

	if ( startTime > 0 )
	{
		querystring.append("&startTime=");
		querystring.append( to_string( startTime ) );
	}

	if ( endTime > 0 )
	{
		querystring.append("&endTime=");
		querystring.append( to_string( endTime ) );
	}

	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	string post_data = "";
	
	Logger::write_log( "<get_withdrawHistory> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_withdrawHistory> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_withdrawHistory> Done." );
	}
	else
	{
		Logger::write_log( "<get_withdrawHistory> Failed to get anything." );
	}
	
	Logger::write_log( "<get_withdrawHistory> Done.\n" );
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
void binance::get_depositAddress( const char *asset, long recvWindow, Json::Value &json_result ) 
{	
	Logger::write_log( "<get_depositAddress>" );

	if ( api_key.size() == 0 || secret_key.size() == 0 )
	{
		Logger::write_log( "<get_depositAddress> API Key and Secret Key has not been set." );
		return;
	}

	string url(BINANCE_HOST);
	url += "/wapi/v3/depositAddress.html?";
	string action = "GET";
	
	string querystring("asset=");
	querystring.append( asset );
	
	if ( recvWindow > 0 )
	{
		querystring.append("&recvWindow=");
		querystring.append( to_string( recvWindow) );
	}

	querystring.append("&timestamp=");
	querystring.append( to_string( get_current_ms_epoch() ) );

	string signature =  hmac_sha256( secret_key.c_str(), querystring.c_str() );
	querystring.append( "&signature=");
	querystring.append( signature );

	url.append( querystring );
	
	vector <string> extra_http_header;
	string header_chunk("X-MBX-APIKEY: ");
	header_chunk.append( api_key );
	extra_http_header.push_back(header_chunk);

	string post_data = "";
	
	Logger::write_log( "<get_depositAddress> url = |%s|" , url.c_str() );
	
	string str_result;
	curl_api_with_header( url, str_result , extra_http_header, post_data , action );

	if ( str_result.size() > 0 )
	{
		try
		{
			Json::Reader reader;
			json_result.clear();	
			reader.parse( str_result , json_result );
		}
		catch ( exception &e )
		{
		 	Logger::write_log( "<get_depositAddress> Error ! %s", e.what() ); 
		}   
		Logger::write_log( "<get_depositAddress> Done." );	
	}
	else
	{
		Logger::write_log( "<get_depositAddress> Failed to get anything." );
	}
	
	Logger::write_log( "<get_depositAddress> Done.\n" );
}

// Curl's callback
size_t binance::curl_cb( void *content, size_t size, size_t nmemb, std::string *buffer ) 
{	
	Logger::write_log( "<curl_cb> " );

	size_t newLength = size*nmemb;
	size_t oldLength = buffer->size();
	try
	{
		buffer->resize(oldLength + newLength);
	}
	catch(std::bad_alloc &e)
	{
		// TODO handle memory problem
		return 0;
	}

	std::copy((char*)content,(char*)content + newLength, buffer->begin() + oldLength);
	Logger::write_log( "<curl_cb> done" );

	return size * nmemb;
}

void binance::curl_api( string &url, string &result_json )
{
	vector <string> v;
	string action = "GET";
	string post_data = "";
	curl_api_with_header( url , result_json , v, post_data , action );	
} 

// Do the curl
void binance::curl_api_with_header(
	string &url, string &str_result, vector <string> &extra_http_header, string &post_data , string &action ) 
{
	Logger::write_log( "<curl_api>" );

	CURL *curl;
	CURLcode res;
	
	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	
	if ( curl )
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str_result );
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

		if ( extra_http_header.size() > 0 )
		{
			struct curl_slist *chunk = NULL;
			for ( int i = 0 ; i < extra_http_header.size();i++ )
				chunk = curl_slist_append(chunk, extra_http_header[i].c_str() );

 			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
 		}

 		if ( post_data.size() > 0 || action == "POST" || action == "PUT" || action == "DELETE" )
 		{
 			if ( action == "PUT" || action == "DELETE" ) {
 				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, action.c_str() );
 			}
 			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str() );
 		}

		res = curl_easy_perform(curl);

		/* Check for errors */ 
		if ( res != CURLE_OK )
		{
			Logger::write_log( "<curl_api> curl_easy_perform() failed: %s" , curl_easy_strerror(res) );
		} 	

		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	Logger::write_log( "<curl_api> done" );
}

