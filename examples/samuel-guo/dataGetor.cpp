#include"dataGetor.h"
#include <iostream>
#include <unistd.h>

using namespace std;
dataGetor::dataGetor(binance::Server &server) :server(server), market(server)
{

}

dataGetor::~dataGetor()
{
}

std::vector<dataGetor::dataStru> dataGetor::getData()
{
    using std::vector;
    //vector<dataStru> *data = new vector<dataStru>();
    vector<dataStru> data;
    //if (startTime <= endTime)
    //     return std::vector<dataStru>();
    time_t range = endTime - startTime;
    time_t stime = startTime;
    time_t etime = time(0);
    Json::Value result;
    long difftime;
    while (1)
    {
        BINANCE_ERR_CHECK(market.getKlines(result, symbol.c_str(), interval.c_str(), stime*1000, etime*1000));
        //long start_of_candle;
        cout << "@@result.size()@@" << result.size() << endl;
        int percent = float(stime - startTime) / range * 100.0f;
        cout << percent << "%" << endl;
        sleep(1);
        for (Json::Value::ArrayIndex i = 0; i < result.size(); i++)
        {
            dataStru item;
            difftime = result[i][0].asInt64() / 1000 - stime;
            stime = result[i][0].asInt64()/1000;
            item.datatime = stime;
            item.open= atof(result[i][1].asString().c_str());
            item.high= atof(result[i][2].asString().c_str());
            item.low= atof(result[i][3].asString().c_str());
            item.close = atof(result[i][4].asString().c_str());
            item.volume= atof(result[i][5].asString().c_str());
            if (stime <= endTime)
                data.push_back(item);
            else
                break;
            //klinesCache[start_of_candle]["o"] = atof(result[i][1].asString().c_str());
            //klinesCache[start_of_candle]["h"] = atof(result[i][2].asString().c_str());
            //klinesCache[start_of_candle]["l"] = atof(result[i][3].asString().c_str());
            //klinesCache[start_of_candle]["c"] = atof(result[i][4].asString().c_str());
            //klinesCache[start_of_candle]["v"] = atof(result[i][5].asString().c_str());
        }
        cout << "@@data.size()@@" << data.size() << endl;

        if (stime > endTime)
        {
            break;
        }
        stime += difftime;
    }
    return data;
}
