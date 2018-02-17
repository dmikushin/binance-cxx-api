/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#ifndef BINANCE_LOGGER_H
#define BINANCE_LOGGER_H

#include <cstdio>
#include <string>

namespace binance
{
	class Logger
	{
		static int debug_level;
		static std::string debug_log_file;
		static int debug_log_file_enable ;
		static FILE *log_fp;

		static void open_logfp_if_not_opened();

	public :
		static void write_log( const char *fmt, ... );
		static void write_log_clean( const char *fmt, ... ); 
		static void set_debug_level( int level );
		static void set_debug_logfile( std::string &pDebug_log_file );
		static void enable_logfile( int pDebug_log_file_enable );
	};
}

#endif // BINANCE_LOGGER_H

