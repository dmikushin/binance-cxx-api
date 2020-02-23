/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#ifndef BINANCE_UTILS_H
#define BINANCE_UTILS_H

#include <unistd.h>
#include <string>
#include <vector>

namespace binance
{
	void split_string( std::string &s, char delim, std::vector<std::string> &result);
	bool replace_string( std::string& str, const char *from, const char *to);
	int replace_string_once( std::string& str, const char *from, const char *to , int offset);

	std::string b2a_hex( char *byte_arr, int n );
	time_t get_current_epoch();
	unsigned long get_current_ms_epoch();

	inline bool file_exists (const std::string& name)
	{
	 	 return ( access( name.c_str(), F_OK ) != -1 );
	}

	std::string hmac_sha256( const char *key, const char *data);
	std::string sha256( const char *data );
	void string_toupper( std::string &src);
	std::string string_toupper( const char *cstr );
}

#endif // BINANCE_UTILS_H

