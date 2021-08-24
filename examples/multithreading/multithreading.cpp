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

void * ThredME(void * id) {
    cout << "ThredME loop" << endl;
    Websocket::init();
    sleep (1);
    Server _server;
    Account account(_server);
    Json::Value json_result;
    account.startUserDataStream(json_result);
    cout << json_result << endl;
    string ws_path = string("/ws/");
    ws_path.append(json_result["listenKey"].asString());

    Websocket::connect_endpoint( ws_userStream_OnData , ws_path.c_str() );
    cout << std::setprecision(10) << "\nWebsocket ws_userStream_OnData init" << std::endl;
    Websocket::enter_event_loop();
    cout << "error exiting ThredME" << endl;
    return NULL;
};

void * ThredMER(void * id) {
  cout << "ThredMER loop" << endl;
  sleep (10);
  cout << "add another endpoint 2" << endl;
  Websocket::connect_endpoint(ws_klines_onData, "/ws/bnbusdt@miniTicker");

  sleep (10);
  cout << "add another endpoint 3" << endl;
  Websocket::connect_endpoint(ws_klines_onData, "/ws/ethusdt@miniTicker");

  sleep(20);
  cout << "remove endpoint 2" << endl;
  Websocket::disconnect_endpoint("/ws/bnbusdt@miniTicker");

  sleep(5);
  cout << "remove non existing endpoint" << endl;
  Websocket::disconnect_endpoint("/ws/xxxxxx@miniTicker");

  sleep(5);
  cout << "remove endpoint 3" << endl;
  Websocket::disconnect_endpoint("/ws/ethusdt@miniTicker");

  sleep(5);
  cout << "remove endpoint ??" << endl;
  Websocket::disconnect_endpoint("/ws/xrpbtc@miniTicker");
  cout << "error exiting ThredMER" << endl;
  return NULL;
};

int main()
{
	Logger::set_debug_level(1);
	Logger::set_debug_logfp(stderr);

  pthread_t inc_x_thread[NUM_THREADS];
  /* create all thread which are to be executes */
  //for (int i = 0; i < NUM_THREADS; i++)
  {
    cout << "enter_event_loop 1" << endl;
    if(pthread_create(&inc_x_thread[0], NULL,ThredME, reinterpret_cast<void *>(0))) {

      fprintf(stderr, "Error creating thread\n");
      return 1;

    }
    //pthread_detach(inc_x_thread[0]);
  }

  {
    cout << "enter_event_loop 2" << endl;
    if(pthread_create(&inc_x_thread[1], NULL,ThredMER, reinterpret_cast<void *>(1))) {

      fprintf(stderr, "Error creating thread\n");
      return 1;

    }
    //pthread_detach(inc_x_thread[1]);
  }

  sleep(10);

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

