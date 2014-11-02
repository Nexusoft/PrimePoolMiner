#ifndef COINSHIELD_UTIL_H
#define COINSHIELD_UTIL_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include "hash/templates.h"
#include "bignum.h"

#ifdef WIN32
	typedef int pid_t;
#else
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/resource.h>
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
uint64 mpz2uint64(mpz_t z);


/** Convert a Bignum into Mpz integer for GMP. **/
int bignum2mpz(const BIGNUM *bn, mpz_t g);


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



#endif
