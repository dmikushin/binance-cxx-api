
#include "binacpp_logger.h"

int    BinaCPP_logger::debug_level   = 1;
string BinaCPP_logger::debug_log_file = "/tmp/binawatch.log";
int    BinaCPP_logger::debug_log_file_enable = 0;
FILE  *BinaCPP_logger::log_fp = NULL;



//-----------------------------------------------
void 
BinaCPP_logger::write_log( const char *fmt, ... ) 
{
    if ( debug_level == 0 ) {
        return;
    }
    if ( debug_log_file_enable == 1 ) {
        open_logfp_if_not_opened();
    }

    va_list arg;
    
    char new_fmt[1024];
    
    struct timeval tv;
    gettimeofday(&tv, NULL); 
    time_t t = tv.tv_sec;
    struct tm * now = localtime( &t );


    sprintf( new_fmt , "%04d-%02d-%02d %02d:%02d:%02d %06ld :%s\n" , now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec , tv.tv_usec , fmt );

    va_start (arg, fmt);
    
    if ( debug_log_file_enable && log_fp ) {
        vfprintf( log_fp , new_fmt ,arg );
        fflush(log_fp);
    } else {
        vfprintf ( stdout, new_fmt, arg);
        fflush(stdout);
    }   

    va_end (arg);

    
}



//-----------------------------------------------
// Write log to channel without any timestamp nor new line
void 
BinaCPP_logger::write_log_clean( const char *fmt, ... ) 
{
    if ( debug_level == 0 ) {
        return;
    }
    if ( debug_log_file_enable == 1 ) {
        open_logfp_if_not_opened();
    }

    va_list arg;
    va_start (arg, fmt);
    
    if ( debug_log_file_enable && log_fp ) {
        vfprintf( log_fp , fmt ,arg );
        fflush(log_fp);
    } else {
        vfprintf ( stdout, fmt, arg);
        fflush(stdout);
    }
    va_end (arg);

}


//---------------------
void 
BinaCPP_logger::open_logfp_if_not_opened() {

    if ( debug_log_file_enable && log_fp == NULL ) {

        log_fp = fopen( debug_log_file.c_str() , "a" );

        if ( log_fp ) {
            printf("log file in %s\n", debug_log_file.c_str());
        } else {
            printf("Failed to open log file.\n" );
        }
    }

}


//---------------------
void 
BinaCPP_logger::set_debug_level( int level ) 
{
    debug_level = level;
}

//--------------------
void 
BinaCPP_logger::set_debug_logfile( string &pDebug_log_file ) 
{
    debug_log_file = pDebug_log_file;
}

//--------------------
void 
BinaCPP_logger::enable_logfile( int pDebug_log_file_enable ) 
{
    debug_log_file_enable = pDebug_log_file_enable;
}




