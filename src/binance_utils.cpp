/*
	Author: tensaix2j
	Date  : 2017/10/15
	
	C++ library for Binance API.
*/

#include "binance_utils.h"

#include <cstring>
#include <sys/time.h>
#include <mbedtls/sha256.h>
#include <mbedtls/md.h>
#include <sstream>

using namespace std;

void binance::split_string( string &s, char delim, vector<string> &result)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim))
		result.push_back(item);
}

int binance::replace_string_once( string& str, const char *from, const char *to, int offset)
{
	size_t start_pos = str.find(from, offset);
	if ( start_pos == std::string::npos )
		return 0;
	str.replace(start_pos, strlen(from), to);
	return start_pos + strlen(to);
}

bool binance::replace_string( string& str, const char *from, const char *to)
{
	bool found = false;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, strlen( from ), to);
		found = true;
		start_pos += strlen(to);
	}
	return found;
}

void binance::string_toupper( string &src)
{
	for ( int i = 0 ; i < src.size() ; i++ )
		src[i] = toupper(src[i]);
}

string binance::string_toupper( const char *cstr )
{
	string ret;
	for ( int i = 0 ; i < strlen( cstr ) ; i++ ) {
		ret.push_back( toupper(cstr[i]) );
	}
	return ret;
}

string binance::b2a_hex( char *byte_arr, int n )
{
	const static std::string HexCodes = "0123456789abcdef";
	string HexString;
	for ( int i = 0; i < n ; ++i ) {
		unsigned char BinValue = byte_arr[i];
		HexString += HexCodes[( BinValue >> 4 ) & 0x0F];
		HexString += HexCodes[BinValue & 0x0F];
	}

	return HexString;
}

time_t binance::get_current_epoch( )
{
	struct timeval tv;
	gettimeofday(&tv, NULL); 

	return tv.tv_sec ;
}

unsigned long binance::get_current_ms_epoch( )
{
	struct timeval tv;
	gettimeofday(&tv, NULL); 

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

string binance::hmac_sha256( const char *key, const char *data)
{
	unsigned char* digest;
	mbedtls_md_hmac( mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),
		reinterpret_cast<const unsigned char*>(key), strlen(key),
		reinterpret_cast<const unsigned char*>(data), strlen(data),
		digest );
	return b2a_hex( (char *)digest, 32 );
}

string binance::sha256( const char *data )
{
	unsigned char digest[32];
	mbedtls_sha256_ret( reinterpret_cast<const unsigned char*>(data),
		strlen(data), digest, 0 );
	return b2a_hex( (char *)digest, 32 );	
}

