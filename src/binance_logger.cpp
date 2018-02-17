/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#include "binance_logger.h"

#include <cstdarg>
#include <sys/time.h>

using namespace std;

int	binance::Logger::debug_level = 0;
string binance::Logger::debug_log_file = "/tmp/binawatch.log";
int	binance::Logger::debug_log_file_enable = 0;
FILE *binance::Logger::log_fp = NULL;

void binance::Logger::write_log( const char *fmt, ... ) 
{
	if ( debug_level == 0 )
		return;

	if ( debug_log_file_enable == 1 )
		open_logfp_if_not_opened();

	va_list arg;
	
	char new_fmt[1024];
	
	struct timeval tv;
	gettimeofday(&tv, NULL); 
	time_t t = tv.tv_sec;
	struct tm * now = localtime( &t );


	sprintf( new_fmt, "%04d-%02d-%02d %02d:%02d:%02d %06ld :%s\n",
		now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, tv.tv_usec, fmt );

	va_start (arg, fmt);
	
	if ( debug_log_file_enable && log_fp )
	{
		vfprintf( log_fp , new_fmt ,arg );
		fflush(log_fp);
	}
	else
	{
		vfprintf ( stdout, new_fmt, arg);
		fflush(stdout);
	}   

	va_end (arg);
}

// Write log to channel without any timestamp nor new line
void binance::Logger::write_log_clean( const char *fmt, ... ) 
{
	if ( debug_level == 0 )
		return;

	if ( debug_log_file_enable == 1 )
		open_logfp_if_not_opened();

	va_list arg;
	va_start (arg, fmt);
	
	if ( debug_log_file_enable && log_fp )
	{
		vfprintf( log_fp , fmt ,arg );
		fflush(log_fp);
	}
	else
	{
		vfprintf ( stdout, fmt, arg);
		fflush(stdout);
	}

	va_end (arg);
}

void binance::Logger::open_logfp_if_not_opened()
{
	if ( debug_log_file_enable && log_fp == NULL )
	{
		log_fp = fopen( debug_log_file.c_str() , "a" );

		if ( log_fp )
			printf("log file in %s\n", debug_log_file.c_str());
		else
			printf("Failed to open log file.\n" );
	}
}

void binance::Logger::set_debug_level( int level ) 
{
	debug_level = level;
}

void binance::Logger::set_debug_logfile( string &pDebug_log_file ) 
{
	debug_log_file = pDebug_log_file;
}

void binance::Logger::enable_logfile( int pDebug_log_file_enable ) 
{
	debug_log_file_enable = pDebug_log_file_enable;
}

