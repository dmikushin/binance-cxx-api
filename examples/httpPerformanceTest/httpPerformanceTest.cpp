//
// Created by mussonero on 25/10/20.
//

#include <iostream>
#include <json/json.h>
#include <string>
#include <vector>
#include <pthread.h>

#include "binance.h"
#include "binance_logger.h"
#include <chrono>

using namespace binance;

using std::map;
using std::vector;
using std::string;
using std::cout;
using std::endl;

#define NUM_THREADS 100

void print_depthCache(map < string, map <double,double> >  depthCache) {

	map < string, map <double,double> >::iterator it_i;

	for ( it_i = depthCache.begin() ; it_i != depthCache.end() ; it_i++ ) {

		string bid_or_ask = (*it_i).first ;
		cout << bid_or_ask <<" Price             " << bid_or_ask << " Qty  =" << endl ;

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
	}
	return 0;
}

void * getDepth(void*  id)
{
	Json::Value result;
	auto _server = new Server;
	auto _market = new Market(*_server);
	auto threadBeginTime = std::chrono::high_resolution_clock::now();
	auto beginTime = std::chrono::high_resolution_clock::now();

	_market->getDepth(result, "LINKBTC", 5);
	auto endTime = std::chrono::high_resolution_clock::now();
	cout << "LINKBTC:getDepth time duration->" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << ":ms" << endl;
	cout << "==================================" << endl;
	ws_klines_onData(result);

	beginTime = std::chrono::high_resolution_clock::now();
	_market->getDepth(result, "ETHTUSD", 5);
	endTime = std::chrono::high_resolution_clock::now();
	cout << "ETHTUSD:getDepth time duration->" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << ":ms" << endl;
	cout << "==================================" << endl;
	ws_klines_onData(result);

	beginTime = std::chrono::high_resolution_clock::now();
	_market->getDepth(result, "XRPBNB", 5);
	endTime = std::chrono::high_resolution_clock::now();
	cout << "XRPBNB:getDepth time duration->" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << ":ms" << endl;
	cout << "==================================" << endl;
	ws_klines_onData(result);

	beginTime = std::chrono::high_resolution_clock::now();
	_market->getDepth(result, "LINKETH", 5);
	endTime = std::chrono::high_resolution_clock::now();
	cout << "LINKETH:getDepth time duration->" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << ":ms" << endl;
	cout << "==================================" << endl;
	ws_klines_onData(result);

	beginTime = std::chrono::high_resolution_clock::now();
	_market->getDepth(result, "BNBUSDT", 5);
	endTime = std::chrono::high_resolution_clock::now();
	cout << "BNBUSDT:getDepth time duration->" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() << ":ms" << endl;
	cout << "==================================" << endl;
	ws_klines_onData(result);

	auto threadEndTime = std::chrono::high_resolution_clock::now();
	fprintf(stdout, "Thread %d finished in->%ld:ms  =\n", reinterpret_cast<int&>(id), (std::chrono::duration_cast<std::chrono::milliseconds>(threadEndTime - threadBeginTime).count()));
	cout << "==================================" << endl;
	cout << "==================================" << endl;
	cout << "==================================" << endl;
	return NULL;
}


int main()
{
	Logger::set_debug_level(0);
	Logger::set_debug_logfp(stderr);

	pthread_t inc_x_thread[NUM_THREADS];

	/* create all thread which are to be executes */
	for (int i = 0; i < NUM_THREADS; i++)
	{
		if(pthread_create(&inc_x_thread[i], NULL,getDepth, reinterpret_cast<void *>(i))) {

			fprintf(stderr, "Error creating thread\n");
			return 1;

		}
	}

	/* wait for all threads to finish */
	for (int i = 0; i < NUM_THREADS; i++)
	{
		if(pthread_join(inc_x_thread[i], NULL)) {

			fprintf(stderr, "Error joining thread\n");
			return 2;

		}
	}

}