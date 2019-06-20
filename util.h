#ifndef COINSHIELD_UTIL_H
#define COINSHIELD_UTIL_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include "hash/templates.h"
#include "bignum.h"

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
#include <conio.h>
#include <time.h>
typedef int pid_t;

//#ifdef HAVE__ALIGNED_MALLOC
#include <malloc.h>
#define aligned_alloc(a, s)		_aligned_malloc(s)
//#else
//#define aligned_alloc(a, s)		malloc(s)
//#endif

#define gmtime_r(now, tm_time)		_gmtime64_s(tm_time, now)
typedef unsigned long int pthread_t;
#elif !defined __MINGW32__
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <sys/resource.h>
#endif

#ifndef gmtime_r
inline struct tm* gmtime_r(const time_t* t, struct tm* r)
{
    // gmtime is threadsafe in windows because it uses TLS
    struct tm *theTm = gmtime(t);
    if (theTm) {
        *r = *theTm;
        return r;
    } else {
        return 0;
    }
}
#endif

#include <string>
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <gmp.h>


#define loop                for(;;)
#define BEGIN(a)            ((char*)&(a))
#define END(a)              ((char*)&((&(a))[1]))
#define Sleep(a)            boost::this_thread::sleep(boost::posix_time::milliseconds(a))
#define LOCK(a)             boost::lock_guard<boost::mutex> lock(a)


int GetTotalCores();

/** Convert an mpz into 64 bit unsigned integer. Used in replacement of mpz_get_ui. **/
uint64_t mpz2uint64(mpz_t n);

/** Convert a Bignum into Mpz integer for GMP. **/
void bignum2mpz(const BIGNUM *bn, mpz_t &g);

/** Convert a 32 bit Unsigned Integer to Byte Vector using Bitwise Shifts. **/
inline std::vector<unsigned char> uint2bytes(unsigned int UINT)
{
	std::vector<unsigned char> BYTES(4, 0);
	BYTES[0] = UINT >> 24;
	BYTES[1] = UINT >> 16;
	BYTES[2] = UINT >> 8;
	BYTES[3] = UINT;
				
	return BYTES;
}
			
			
/** Convert a byte stream into unsigned integer 32 bit. **/	
inline unsigned int bytes2uint(std::vector<unsigned char> BYTES, int nOffset = 0) { return (BYTES[0 + nOffset] << 24) + (BYTES[1 + nOffset] << 16) + (BYTES[2 + nOffset] << 8) + BYTES[3 + nOffset]; }
			
/** Convert a 64 bit Unsigned Integer to Byte Vector using Bitwise Shifts. **/
inline std::vector<unsigned char> uint2bytes64(uint64 UINT)
{
	std::vector<unsigned char> INTS[2];
	INTS[0] = uint2bytes((unsigned int) UINT);
	INTS[1] = uint2bytes((unsigned int) (UINT >> 32));
				
	std::vector<unsigned char> BYTES;
	BYTES.insert(BYTES.end(), INTS[0].begin(), INTS[0].end());
	BYTES.insert(BYTES.end(), INTS[1].begin(), INTS[1].end());
				
	return BYTES;
}

			
/** Convert a byte Vector into unsigned integer 64 bit. **/
inline uint64 bytes2uint64(std::vector<unsigned char> BYTES) { return (bytes2uint(BYTES) | ((uint64)bytes2uint(BYTES, 4) << 32)); }



/** Convert Standard String into Byte Vector. **/
inline std::vector<unsigned char> string2bytes(std::string STRING)
{
	std::vector<unsigned char> BYTES(STRING.begin(), STRING.end());
	return BYTES;
}


/** Convert Byte Vector into Standard String. **/
inline std::string bytes2string(std::vector<unsigned char> BYTES)
{
	std::string STRING(BYTES.begin(), BYTES.end());
	return STRING;
}

inline std::vector<unsigned char> double2bytes(double DOUBLE)
{
    union {
        double DOUBLE;
        uint64 UINT64;
    } u;
    u.DOUBLE = DOUBLE;

    return uint2bytes64(u.UINT64);
}

inline double bytes2double(std::vector<unsigned char> BYTES)
{
    uint64 n64 = bytes2uint64(BYTES);
    union {
        double DOUBLE;
        uint64 UINT64;
    } u;
    u.UINT64 = n64;
    return u.DOUBLE;
}

inline int64 GetTimeMicros()
{
	return (boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time()) - boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).total_microseconds();
}

void SetCurrentThreadPriority(int priority = INT32_MAX);

void SetThreadPriority(pthread_t threadID, int priority);


unsigned int popcount32(uint32_t v)
{
	unsigned int c;

	v = v - ((v >> 1) & 0x55555555U);
	v = (v & 0x33333333U) + ((v >> 2) & 0x33333333U);
	v = (v + (v >> 4)) & 0x0f0f0f0fU;
	c = (v * 0x01010101U) >> 24;
	/*
	* v = (v >> 16) + v;
	* v = (v >> 8) + v;
	* c = v & 255;
	*/

	return c;
}

unsigned long sqrtld(unsigned long N) 
{
	int                 b = 1;
	unsigned long       res, s;
	while (1 << b<N) b += 1;
	res = 1 << (b / 2 + 1);
	for (;;) {
		s = (N / res + res) / 2;
		if (s >= res) return res;
		res = s;
	}
}

static unsigned int int_invert(unsigned int a, unsigned int nPrime)
{
	// Extended Euclidean algorithm to calculate the inverse of a in finite field defined by nPrime
	int rem0 = nPrime, rem1 = a % nPrime, rem2;
	int aux0 = 0, aux1 = 1, aux2;
	int quotient, inverse;

	while (1)
	{
		if (rem1 <= 1)
		{
			inverse = aux1;
			break;
		}

		rem2 = rem0 % rem1;
		quotient = rem0 / rem1;
		aux2 = -quotient * aux1 + aux0;

		if (rem2 <= 1)
		{
			inverse = aux2;
			break;
		}

		rem0 = rem1 % rem2;
		quotient = rem1 / rem2;
		aux0 = -quotient * aux2 + aux1;

		if (rem0 <= 1)
		{
			inverse = aux0;
			break;
		}

		rem1 = rem2 % rem0;
		quotient = rem2 / rem0;
		aux1 = -quotient * aux0 + aux2;
	}

	return (inverse + nPrime) % nPrime;
}

#endif
