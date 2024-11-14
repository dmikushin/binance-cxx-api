#pragma once
#include <ctime>
#include <string>
#include <binance.h>
#include <vector>


class dataGetor
{

public:
	dataGetor(binance::Server& server);
	~dataGetor();
	
	struct dataStru
	{
		float open;
		float high;
		float low;
		float close;
		float volume;
		time_t datatime;

	};
	binance::Server server;

	binance::Market market;

	std::string symbol;
	time_t startTime = 0;
	time_t endTime = 0;
	std::string interval;



	std::vector<dataStru> getData();


	
	//
	//enum Interval
	//{
	//	E1m = 0,
	//	E3m,E15m,E30m,
	//	E1h,E2h,E4h,E6h,E8h,E12h,
	//	E1d,E3d,
	//	E1w,E1M
	//};

private:
};
