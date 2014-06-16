/*
 * utils.cpp
 *
 *  Created on: Apr 16, 2014
 *      Author: alexei
 */

#include "utils.hpp"

void allocate_memory( void** dst, size_t size, size_t type_size )
{
	*dst = calloc( size, type_size );
	_assert( dst != NULL, "Memory allocation failed\n" );
}

void reallocate_memory( void** dst, size_t new_size )
{
	*dst = realloc( *dst, new_size );
	_assert( dst != NULL, "Failure re-allocating memory\n");
}

void free_memory( void* ptr )
{
	if( ptr != NULL )
		free( ptr );
}

FILE* safe_fopen( const char* file, const char* mode )
{
	FILE* f = fopen( file, mode );
	_assert( f != NULL, "Failure while opening file %s for %s\n", file, mode );
	return f;
}

void safe_fclose( FILE* f, const char* file )
{
	_assert( f != NULL && fclose(f) != EOF, "Failure while closing file %s\n", file );
}


