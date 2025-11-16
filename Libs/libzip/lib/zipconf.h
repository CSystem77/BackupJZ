#ifndef _HAD_ZIPCONF_H
#define _HAD_ZIPCONF_H

/*
   zipconf.h -- platform specific include file
   Configuration for Windows/MSVC
*/

#ifdef _MSC_VER
// MSVC: utiliser cstdint pour C++ ou stdint.h pour C
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif
#else
#include <stdint.h>
#endif

#define LIBZIP_VERSION "1.10.1"
#define LIBZIP_VERSION_MAJOR 1
#define LIBZIP_VERSION_MINOR 10
#define LIBZIP_VERSION_MICRO 1

// Use standard C99 types
#ifdef _MSC_VER
// MSVC specific types
typedef signed char zip_int8_t;
typedef unsigned char zip_uint8_t;
typedef short zip_int16_t;
typedef unsigned short zip_uint16_t;
typedef int zip_int32_t;
typedef unsigned int zip_uint32_t;
typedef __int64 zip_int64_t;
typedef unsigned __int64 zip_uint64_t;
#else
// Standard C99 types
typedef int8_t zip_int8_t;
typedef uint8_t zip_uint8_t;
typedef int16_t zip_int16_t;
typedef uint16_t zip_uint16_t;
typedef int32_t zip_int32_t;
typedef uint32_t zip_uint32_t;
typedef int64_t zip_int64_t;
typedef uint64_t zip_uint64_t;
#endif

#define ZIP_INT8_MIN	 (-ZIP_INT8_MAX-1)
#define ZIP_INT8_MAX	 0x7f
#define ZIP_UINT8_MAX	 0xff

#define ZIP_INT16_MIN	 (-ZIP_INT16_MAX-1)
#define ZIP_INT16_MAX	 0x7fff
#define ZIP_UINT16_MAX	 0xffff

#define ZIP_INT32_MIN	 (-ZIP_INT32_MAX-1L)
#define ZIP_INT32_MAX	 0x7fffffffL
#define ZIP_UINT32_MAX	 0xffffffffLU

#define ZIP_INT64_MIN	 (-ZIP_INT64_MAX-1LL)
#define ZIP_INT64_MAX	 0x7fffffffffffffffLL
#define ZIP_UINT64_MAX	 0xffffffffffffffffULL

#endif /* zipconf.h */

