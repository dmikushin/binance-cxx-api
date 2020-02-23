/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#ifndef BINANCE_WEBSOCKET_H
#define BINANCE_WEBSOCKET_H

#include <jsoncpp/json/json.h>

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
		static void enter_event_loop();
	};
}

#endif // BINANCE_WEBSOCKET_H

