#include <iostream>
#include <json/json.h>
#include <map>
#include <string>
#include <vector>

#include "binance.h"
#include "binance_logger.h"
#include "binance_websocket.h"
#include <pthread.h>
#include <iomanip>

using namespace binance;
using namespace std;

#define NUM_THREADS 2


void print_depthCache(map < string, map <double,double> >  depthCache, int64_t id) {

	map < string, map <double,double> >::iterator it_i;

	for ( it_i = depthCache.begin() ; it_i != depthCache.end() ; it_i++ ) {

		string bid_or_ask = (*it_i).first ;
		cout << bid_or_ask << endl ;
		cout << "Price         Qty    ID:" << id << endl ;

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
		print_depthCache(depthCache, json_result["lastUpdateId"].asInt64());
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

static int
ws_userStream_OnData(Json::Value &json_result)
{
  cout << "==================================" << endl;
  cout << "==================================" << endl;
  cout << json_result << endl;
  cout << "==================================" << endl;
  cout << "==================================" << endl;
  return 0;
}

string ws_1th_path = string("/ws/");

void * Thread_1(void * id) {
    cout << "Thread_1 loop" << endl;

    Server _server;
    Account account(_server);
    Json::Value json_result;
    account.startUserDataStream(json_result);
    cout << json_result << endl;
    ws_1th_path.append(json_result["listenKey"].asString());
    Websocket::init();
    Websocket::connect_endpoint(ws_userStream_OnData, ws_1th_path.c_str());
    cout << std::setprecision(10) << "\nWebsocket ws_userStream_OnData init" << std::endl;

    sleep(1);
    cout << "add another endpoint 1th before enter_event_loop" << endl;
    Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbtc@miniTicker");

    Websocket::enter_event_loop(std::chrono::hours(1));
    cout << "exiting Thread_1" << endl;
    return NULL;
};

void * Thread_2(void * id) {
  cout << "Thread_2 loop" << endl;
  sleep (10);
  cout << "add /ws/bnbusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/bnbusdt@miniTicker");

  sleep (10);
  cout << "add /ws/ethusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/ethusdt@miniTicker");

  sleep(20);
  cout << "remove /ws/bnbusdt@miniTicker" << endl;
  sleep(1);
  Websocket::disconnect_endpoint("/ws/bnbusdt@miniTicker");

  sleep(10);
  cout << "remove non existing /ws/xxxxxx@miniTicker ??" << endl;
  sleep(1);
  Websocket::disconnect_endpoint("/ws/xxxxxx@miniTicker");

  sleep(10);
  cout << "remove /ws/algobnb@depth5@1000ms before enter_event_loop" << endl;
  sleep(1);
  Websocket::disconnect_endpoint("/ws/algobnb@depth5@1000ms");

  sleep(10);
  cout << "remove "<<ws_1th_path<<" in init" << endl;
  sleep(1);
  Websocket::disconnect_endpoint(ws_1th_path.c_str());

  sleep (20);
  cout << "add /ws/bnbusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/bnbusdt@miniTicker");

  sleep(10);
  cout << "add /ws/xrpbtc@miniTicker" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbtc@miniTicker");

  sleep (20);
  cout << "add /ws/algobnb@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/algobnb@miniTicker");

  sleep(10);
  cout << "remove /ws/ethusdt@depth5@1000ms before enter_event_loop" << endl;
  sleep(1);
  Websocket::disconnect_endpoint("/ws/ethusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/xtzusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzusdt@miniTicker");

  sleep (20);
  cout << "add /ws/eosusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/eosusdt@miniTicker");

  sleep (20);
  cout << "add /ws/xrpbnb@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbnb@miniTicker");


  sleep (20);
  cout << "add /ws/xtzusdt@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/eosusdt@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/eosusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/xrpbnb@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbnb@depth5@1000ms");


  sleep (20);
  cout << "add /ws/ethusdt@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/ethusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/ethbusd@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/ethbusd@depth5@1000ms");

  sleep(60*30);
  cout << "add /ws/bnbusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/bnbusdt@miniTicker");

  sleep(10);
  cout << "add /ws/xrpbtc@miniTicker" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbtc@miniTicker");

  sleep (20);
  cout << "add /ws/algobnb@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/algobnb@miniTicker");

  sleep(10);
  cout << "remove /ws/ethusdt@depth5@1000ms before enter_event_loop" << endl;
  sleep(1);
  Websocket::disconnect_endpoint("/ws/ethusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/xtzusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzusdt@miniTicker");

  sleep (20);
  cout << "add /ws/eosusdt@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/eosusdt@miniTicker");

  sleep (20);
  cout << "add /ws/xrpbnb@miniTicker after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbnb@miniTicker");


  sleep (20);
  cout << "add /ws/xtzusdt@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xtzusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/eosusdt@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/eosusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/xrpbnb@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/xrpbnb@depth5@1000ms");


  sleep (20);
  cout << "add /ws/ethusdt@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/ethusdt@depth5@1000ms");

  sleep (20);
  cout << "add /ws/ethbusd@depth5@1000ms after enter_event_loop" << endl;
  sleep(1);
  Websocket::connect_endpoint(ws_klines_onData, "/ws/ethbusd@depth5@1000ms");

  sleep(60*30);
  cout << "kill_all" << endl;
  sleep(5);
  Websocket::kill_all();

  cout << "exiting Thread_2" << endl;
  return NULL;
};

int main()
{
	Logger::set_debug_level(1);
	Logger::set_debug_logfp(stderr);

  pthread_t inc_x_thread[NUM_THREADS];
  /* create all thread which are to be executes */
  for (int i = 0; i < NUM_THREADS; i++)
  {
    if(i == 0){
      sleep(1);
      cout << "pthread_create 1" << endl;
      if (pthread_create(&inc_x_thread[0], NULL, Thread_1, reinterpret_cast<void *>(0))) {

        fprintf(stderr, "Error creating thread\n");
        return 1;

      }
    }

    cout << "sleep for 10sec before adding other endpoints" << endl;
    sleep(10);
    
    if(i == 1){
      sleep(1);
      cout << "pthread_create 2" << endl;
      if(pthread_create(&inc_x_thread[1], NULL,Thread_2, reinterpret_cast<void *>(1))) {

        fprintf(stderr, "Error creating thread\n");
        return 1;

      }
    }
  }

  /* wait for the thread to finish */
  for (int i = 0; i < NUM_THREADS; i++)
  {
    
    if(pthread_join(inc_x_thread[i], NULL)) {

      fprintf(stderr, "Error joining thread\n");
      return -2;
    }
  }

	return 0;
}

