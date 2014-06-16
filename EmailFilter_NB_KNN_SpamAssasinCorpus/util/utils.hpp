
#ifndef UTILS_H
#define UTILS_H

/*
	C headers
*/

#include<cmath>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<unistd.h>

/*
	STL
*/

#include<map>
#include<set>
#include<queue>
#include<stack>
#include<string>
#include<vector>
#include<bitset>
#include<algorithm>
#include<functional>

#include<fstream>
#include<sstream>
#include<iostream>

/**
 * C++11
 */

#include <unordered_map>
#include <unordered_set>

#include "rand.h"

#ifdef __linux__
        #include <dirent.h>
#endif

#ifdef SSE3
	#include <pmmintrin.h>
#endif

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

typedef unsigned int uint;
typedef unsigned char uchar;

const uint inf = 0x3f3f3f3f;

#define DBG_LEVEL 1
#define dbg( level, format, ...) if( level >= DBG_LEVEL ) fprintf (stderr, format, ## __VA_ARGS__)

#define _assert( condition, call_descr, ... )								    \
{																				\
	if( !(condition) )															\
	{																			\
	        perror("");                                                             \
                fprintf(stderr, "%s: %s()-> line %d\n", __FILE__, __func__, __LINE__);	\
		fprintf(stderr, call_descr, ## __VA_ARGS__ );				\
								 									    \
		exit( EXIT_FAILURE );													\
	}																			\
}

template< class T >
inline T sqr( T val ){
	return val * val;
}

template< class T >
inline void swap_ptr( T **p1, T **p2 ){
	T* t = *p1;
	*p1  = *p2;
	*p2  = t;
}

FILE* safe_fopen( const char* path, const char* mode );
void  safe_fclose( FILE* f, const char* file );

/* Memory management */
void allocate_memory( void** dst, size_t size, size_t type_size );
void reallocate_memory( void** dst, size_t new_size );
void free_memory( void* ptr );

#define for_each(i,v) \
	for(uint i = 0, sz = v.size(); i < sz; ++i)

#define _for(i,size) \
	for( int i = 0, sz = size; i < sz; ++i )

#define _forf(i,start,end) \
	for( int i = start; i < end; ++i )

#define _forb(i,start,end) \
	for( int i = start; i > end; --i )

#define ZERO_EPS 0.00000001
inline bool is_float_zero(float value)
{
	if( value > 0 )
	{
		if( value < ZERO_EPS ){
			return true;
		}

	} else if( value > -ZERO_EPS ){
		return true;
	}

	return false;
}


class InvalidQuery{
	
public:
	InvalidQuery( std::string error_msg ){
		std::cerr << "Invalid Query: " << error_msg << "\n";
	}
};

template< class T >
void check_in_range( T value, T range_start, T range_end )
{
	if( value < range_start || value > range_end )
	{
		std::cerr << value << " not in [ " << range_start;
		std::cerr << " , " << range_end << "\n";
		throw InvalidQuery("elem not in range");
	}
}


template < class T >
std::ostream& operator << (std::ostream& os, const typename std::vector<T>& v) {

	for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii){
		os << " " << *ii;	
	}
	os << "\n";
	return os;
}

template < class T >
std::ostream& operator << (std::ostream& os, const typename std::vector< std::pair< T, T > >& v) {

    for (typename std::vector< std::pair< T, T > >::const_iterator ii = v.begin(); ii != v.end(); ++ii){
        os << "(" << *ii->first << " , " << *ii->second << ") ";
    }
	os << "\n";
    return os;
}

const int MAX_PATH_SIZE = 256;
const int MAX_LINE_SIZE = 256;
const int MAX_FILE_NUM  = 10000;

#ifdef __linux__

template< class Fun >
void scan_dir( const char* dir_path, Fun& f )
{
        char file_path[MAX_PATH_SIZE];

        DIR* dp;
        struct dirent *ep;
        _assert( (dp = opendir( dir_path )) != NULL, "open_dir %s |", dir_path );

        int file_count = 0;
        while( (ep = readdir(dp) ) != NULL )
        {
                if( ep->d_name[0] == '.' )
                        continue;

                if( file_count++ == MAX_FILE_NUM ){
                        break;
                }

                sprintf( file_path, "%s/%s", dir_path, ep->d_name );
                f(file_path);

        }

        _assert( closedir(dp) == 0, "closedir" );
}

#endif

#endif
