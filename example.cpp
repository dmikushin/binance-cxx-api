#include <iostream>
#include <json/json.h>
#include <map>
#include <string>
#include <vector>

#include "binance.h"
#include "binance_logger.h"
#include "binance_websocket.h"

using namespace binance;
using namespace std;

void print_depthCache(map < string, map <double,double> >  depthCache) {

	map < string, map <double,double> >::iterator it_i;

	for ( it_i = depthCache.begin() ; it_i != depthCache.end() ; it_i++ ) {

		string bid_or_ask = (*it_i).first ;
		cout << bid_or_ask << endl ;
		cout << "Price             Qty" << endl ;

		map <double,double>::reverse_iterator it_j;

		cout << "==================================" << endl;
		for ( it_j = depthCache[bid_or_ask].rbegin() ; it_j != depthCache[bid_or_ask].rend() ; it_j++ ) {

			double price = (*it_j).first;
			double qty   = (*it_j).second;
			printf("%.08f          %.08f\n", price, qty );
		}
		cout << "==================================" << endl;
	}
}

void print_klinesCache(map<long, map<string, double> > klinesCache)
{

	for (map<long, map<string, double> >::iterator it_i = klinesCache.begin();
		 it_i != klinesCache.end(); it_i++)
	{
		long start_of_candle = (*it_i).first;
		map <string,double> candle_obj 	= (*it_i).second;

		cout << "s:" << start_of_candle << ",";
		cout << "o:" << candle_obj["o"] << ",";
		cout << "h:" << candle_obj["h"] << ",";
		cout << "l:" << candle_obj["l"] << ",";
		cout << "c:" << candle_obj["c"] << ",";
		cout << "v:" << candle_obj["v"] ;
		cout << " " << endl;
	}
	cout << "==================================" << endl;
}

int ws_klines_onData(Json::Value& json_result)
{
	map<long, map<string, double> > klinesCache;
	map < string, map <double,double> >  depthCache;

	if (json_result["lastUpdateId"].isNumeric() && json_result["bids"].isArray() &&  json_result["asks"].isArray()) {
		int i;

		for ( i = 0 ; i < json_result["bids"].size() ; i++ ) {
			double price = atof( json_result["bids"][i][0].asString().c_str());
			double qty 	 = atof( json_result["bids"][i][1].asString().c_str());
			if ( qty == 0.0 ) {
				depthCache["bids"].erase(price);
			} else {
				depthCache["bids"][price] = qty;
			}
		}
		for ( i = 0 ; i < json_result["asks"].size() ; i++ ) {
			double price = atof( json_result["asks"][i][0].asString().c_str());
			double qty 	 = atof( json_result["asks"][i][1].asString().c_str());
			if ( qty == 0.0 ) {
				depthCache["asks"].erase(price);
			} else {
				depthCache["asks"][price] = qty;
			}
		}
		print_depthCache(depthCache);
	}else if(json_result["e"].asString() == "kline"){
		long start_of_candle = json_result["k"]["t"].asInt64();
		klinesCache[start_of_candle]["o"] = atof(json_result["k"]["o"].asString().c_str());
		klinesCache[start_of_candle]["h"] = atof(json_result["k"]["h"].asString().c_str());
		klinesCache[start_of_candle]["l"] = atof(json_result["k"]["l"].asString().c_str());
		klinesCache[start_of_candle]["c"] = atof(json_result["k"]["c"].asString().c_str());
		klinesCache[start_of_candle]["v"] = atof(json_result["k"]["v"].asString().c_str());

		cout << "=============="<< json_result["s"].asString() <<"====================" << endl;
		print_klinesCache(klinesCache);
	}else if(json_result["e"].asString() == "24hrMiniTicker"){
		cout << "=====24hrMiniTicker\\Begin========="<< json_result["s"].asString() <<"=======24hrMiniTicker\\Begin" << endl;
		cout << json_result << endl;
		cout << "=====24hrMiniTicker\\End========="<< json_result["s"].asString() <<"=======24hrMiniTicker\\End====" << endl;
	}

	return 0;
}


int main()
{
	Logger::set_debug_level(1);
	Logger::set_debug_logfp(stderr);

	Json::Value result;

	Server server;

	Market market(server);

	// Klines / CandleStick
	BINANCE_ERR_CHECK(market.getKlines(result, "BNBUSDT", "1w", 0, 0, 10));
	map<long, map<string, double> > klinesCache;
	for (Json::Value::ArrayIndex i = 0 ; i < result.size() ; i++)
	{

		long start_of_candle = result[i][0].asInt64();
		klinesCache[start_of_candle]["o"] = atof(result[i][1].asString().c_str());
		klinesCache[start_of_candle]["h"] = atof(result[i][2].asString().c_str());
		klinesCache[start_of_candle]["l"] = atof(result[i][3].asString().c_str());
		klinesCache[start_of_candle]["c"] = atof(result[i][4].asString().c_str());
		klinesCache[start_of_candle]["v"] = atof(result[i][5].asString().c_str());
	}

	print_klinesCache(klinesCache);

	while (true){
		Websocket::init();
		sleep (1);
		Websocket::connect_endpoint(ws_klines_onData, "/ws/poebtc@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/batbtc@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzusdt@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/bnbusdt@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/algobnb@depth20@1000ms");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/poebtc@kline_1m");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/batbtc@kline_1m");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzusdt@kline_1m");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/bnbusdt@kline_1m");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/algobnb@kline_1m");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzbtc@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzbtc@miniTicker");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbtc@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbtc@miniTicker");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/eosusdt@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/eosusdt@miniTicker");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/algousdt@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/algousdt@miniTicker");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbnb@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbnb@miniTicker");

		Websocket::connect_endpoint(ws_klines_onData, "/ws/ethusdt@depth20@1000ms");
		Websocket::connect_endpoint(ws_klines_onData, "/ws/ethusdt@miniTicker");


		Websocket::enter_event_loop();
		cout << "error exiting enter_event_loop and we will try again after 5sec" << endl;
		sleep(5);
	}


	return 0;
}

