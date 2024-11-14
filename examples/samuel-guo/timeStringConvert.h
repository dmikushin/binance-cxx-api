#ifndef TIMESTRINGCONVERT_CPP
#define TIMESTRINGCONVERT_CPP

#include <time.h>
#include <sstream>
#include<string.h>
#include<iostream>
namespace timeConvertor 
{
    using namespace std;

    // time转格式化字符串=====================================

    std::string ShowDateTime(const tm& t, const string& format)
    {
        char s[100];
        strftime(s, sizeof(s), format.c_str(), &t);
        return string(s);
    }


    std::string ShowDateTime(const time_t& t, const string& format)
    {
        tm _tm;

        localtime_r(&t, &_tm);

        return ShowDateTime(_tm, format);
    }

    std::string ShowDateTime(const tm& t, const char& dateDiv = '-', const char& timeDiv = ':')
    {
        ostringstream format;
        format << "%Y" << dateDiv << "%m" << dateDiv << "%d" << ' ';
        format << "%H" << timeDiv << "%M" << timeDiv << "%S";

        return ShowDateTime(t, format.str());
    }

    std::string ShowDateTime(const time_t& t, const char& dateDiv = '-', const char& timeDiv = ':')
    {
        ostringstream format;
        format << "%Y" << dateDiv << "%m" << dateDiv << "%d" << ' ';
        format << "%H" << timeDiv << "%M" << timeDiv << "%S";

        return ShowDateTime(t, format.str());
    }

    std::string ShowYMD(const time_t& t, const char& dateDiv = '-')
    {
        ostringstream format;
        format << "%Y" << dateDiv << "%m" << dateDiv << "%d";

        return ShowDateTime(t, format.str());
    }

    std::string ShowHMS(const time_t& t, const char& timeDiv = ':')
    {
        ostringstream format;
        format << "%H" << timeDiv << "%M" << timeDiv << "%S";

        return ShowDateTime(t, format.str());
    }

    std::string ShowHM(const time_t& t, const char& timeDiv = ':')
    {
        ostringstream format;
        format << "%H" << timeDiv << "%M";

        return ShowDateTime(t, format.str());
    }

    // 格式化字符串转time=====================================

    time_t mkgmtime(tm* pTm)
    {
        unsigned int year = pTm->tm_year + 1900;
        unsigned int mon = pTm->tm_mon + 1;
        unsigned int day = pTm->tm_mday;
        unsigned int hour = pTm->tm_hour;
        unsigned int min = pTm->tm_min;
        unsigned int sec = pTm->tm_sec;

        if (0 >= (int)(mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
            mon += 12;      /* Puts Feb last since it has leap day */
            year -= 1;
        }

        return (((
            (unsigned long)(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
            year * 365 - 719499
            ) * 24 + hour /* now have hours */
            ) * 60 + min /* now have minutes */
            ) * 60 + sec; /* finally seconds */
    }

    time_t str2time(const string& dateStr, const string& format)
    {
        tm t;
        memset(&t, 0, sizeof(tm));

        ::strptime(dateStr.c_str(), format.c_str(), &t);// windows下用不了

        return mktime(&t);
    }

    time_t str2time(const string& dateStr, const char& dateDiv = '-', const char& timeDiv = ':')
    {
        string format = "%Y-%m-%d %H:%M:%S";
        if (dateDiv != '-')
        {
            format[2] = format[5] = dateDiv;
        }
        if (timeDiv != ':')
        {
            format[11] = format[14] = timeDiv;
        }

        return str2time(dateStr.c_str(), format);
    }

    time_t str2date(const string& dateStr, const char& dateDiv = '-')
    {
        string format = "%Y-%m-%d";
        if (dateDiv != '-')
        {
            format[2] = format[5] = dateDiv;
        }

        return str2time(dateStr.c_str(), format);
    }

    // 使用====================================

    //int main() {
    //    time_t now = time(0);
    //    cout << ShowDateTime(now) << endl;
    //    cout << str2date(ShowYMD(now)) << endl;
    //
    //    system("pause");// 暂停以显示终端窗口
    //    return 0;
    //}
}
#endif