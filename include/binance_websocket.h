/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#ifndef BINANCE_WEBSOCKET_H
#define BINANCE_WEBSOCKET_H

#include <json/json.h>
#include <chrono>

#define BINANCE_WS_HOST "stream.binance.com"
#define BINANCE_WS_PORT 9443

namespace binance
{
	typedef int (*CB)( Json::Value &json_value );

	class Websocket
	{	
	public :
		static void connect_endpoint(CB user_cb, const char* path);
		static void init();
		static void enter_event_loop(std::chrono::hours hours = std::chrono::hours(24));
	};
}

#endif // BINANCE_WEBSOCKET_H

