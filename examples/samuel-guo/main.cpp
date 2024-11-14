#include <iostream>
#include <json/json.h>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
//#include <cctype>
#include "binance_logger.h"

#include "dataGetor.h"
#include "timeStringConvert.h"
const std::string verString =
"V1.3"
;

//Server serverUs("https://api.binance.us");
binance::Server serverCn;
//Server& server = serverCn;

using namespace binance;
using namespace std;


map<long, map<string, double> > klinesCache;

void testFuture()
{
    cout << "<>Server::getTime<>" << endl;

	binance::Server server("https://fapi.binance.com", "/fapi/v1/");

	Json::Value result;

	server.getTime(result);
	
	cout << result << endl;

	result.clear();
	Account acc(server);

	BINANCE_ERR_CHECK(acc.getBalance_f(result,5000));
	cout << result << endl;

	//Json::Value result;

	//server.getTime(result);

}


int main(int argc, char* argv[])
{

	bool b = getenv("FREEZE_ON_ERROR");

	Logger::set_debug_level(1);
	Logger::set_debug_logfp(stdout);

	srand(time(NULL));
	
	testFuture();

	time_t startTime = time(NULL);

	cout << "----start at----" << ctime(&startTime) << verString << endl;

	
	//Json::Value tl = result["serverTime"];
	//if (!tl.isNull())
	//{
	//	cout << "serverTime = " << result["serverTime"] << endl;

	//}
	//else
	//{
	//	cerr << "result =" << result << endl;
	//	cerr << "<>Server::getTime<> failed" << endl;
	//}
	string outfile;
	dataGetor getor(serverCn);
	if (argc < 8)
	{
		cout << "usage:" << endl;
		cout<< "- s BTCUSDT -st	20180101-020000"
			<<"-et 20200801-020000 -i 1d"<<endl;
		cout << "{3m 5m 15m 30m 1h 2h 4h 6h 8h 12h 1d 3d 1w 1M}" << endl;
		exit(0);
	}
	for (int i = 1; i < argc-1; ++i)
	{
		cout << argv[i] << " - " << argv[i + 1] << endl;
		if (strcasecmp(argv[i], "-s")==0)
		{
			getor.symbol =  argv[i + 1];
		}
		else if (strcasecmp(argv[i], "-st") == 0)
		{
			getor.startTime = timeConvertor::str2time(argv[i + 1],"%Y%m%d%H%M%S");
		}
		else if (strcasecmp(argv[i], "-et") == 0)
		{
			getor.endTime = timeConvertor::str2time(argv[i + 1], "%Y%m%d%H%M%S");
		}
		else if (strcasecmp(argv[i], "-i") == 0)
		{
			getor.interval = argv[i + 1];
		}
		else if (strcasecmp(argv[i], "-f") == 0)
		{
			outfile = argv[i + 1];
		}

	}

	cout << "start Time = " << timeConvertor::ShowDateTime(getor.startTime) << endl
		<< "end time = " << timeConvertor::ShowDateTime(getor.endTime) << endl;
	cout << "interval:" << getor.interval << endl;


	if (outfile == "")
	{
		outfile = "gData"+getor.symbol+"-" + timeConvertor::ShowDateTime(getor.startTime)
			+ "-" + timeConvertor::ShowDateTime(getor.endTime) + "-" + getor.interval + ".txt";
	}

	auto data =getor.getData();

	ofstream  myfile(outfile);

	cout << "@@data.size()@@" << data.size() << endl;
	int count = 1;
	for (auto& item : data)
	{
		ostringstream os;
		os << count <<"\t" << timeConvertor::ShowDateTime(item.datatime) << "\t" << item.datatime << "\t" 
			<< item.open << "\t" <<item.high<<"\t" <<item.low <<"\t" << item.close;
		cout << os.str() << endl;
		myfile << os.str() << endl;
		count++;

	}

	cout << endl<< "output:" << outfile << endl;

	return 0;
}

