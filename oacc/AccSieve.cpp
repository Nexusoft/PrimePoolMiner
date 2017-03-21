#include <stdio.h>
#include <string.h>
#include <fstream>
using namespace std;

#include "AccSieve.h"


static unsigned int _offsets8Tuple1[8] = { 0, 2, 6, 8, 12, 18, 20, 26 };
static int nSieveTarget = 5;

mpz_t * pgisieve(unsigned char * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement)
{
	unsigned long c[10000];
	const int sieveSizeBytes = sieveSize / 8;

	mpz_t zPrimorialMod, zTempVar;// , zFirstSieveElement;
	mpz_init(zPrimorialMod);
	mpz_init(zTempVar);
	//mpz_init(zFirstSieveElement);

	memset(sieve, 0x00, (sieveSize) / 8);
	mpz_mod(zPrimorialMod, zPrimeOrigin, zPrimorial);
	mpz_sub(zPrimorialMod, zPrimorial, zPrimorialMod);

	mpz_mod(zPrimorialMod, zPrimorialMod, zPrimorial);

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
	mpz_import(zOctuplet, 1, 1, sizeof(octuplet_origins[j]), 0, 0, &ktuple_origin);
	mpz_add(zPrimorialMod, zPrimorialMod, zOctuplet);
#else
	mpz_add_ui(zPrimorialMod, zPrimorialMod, ktuple_origin);
#endif


	mpz_add(zTempVar, zPrimeOrigin, zPrimorialMod);
	mpz_set(*zFirstSieveElement, zTempVar);
	unsigned int * base_remainders = (unsigned int *)malloc(nPrimeLimit * sizeof(unsigned long));

	for (unsigned int i = nPrimorialEndPrime; i < nPrimeLimit; i++)
	{
		unsigned long  p = primes[i];
		base_remainders[i] = mpz_tdiv_ui(zTempVar, p);
	}

#pragma acc data copy(sieve[0:sieveSizeBytes], primes[0:nPrimeLimit], inverses[0:nPrimeLimit], base_remainders[0:nPrimeLimit], _offsets8Tuple1) 
//#pragma acc kernels
//#pragma acc parallel 

	for (unsigned int i = nPrimorialEndPrime; i < nPrimeLimit; i++)
	{
		unsigned long  p = primes[i];
		unsigned int inv = inverses[i];
		unsigned int base_remainder = base_remainders[i];


		for (int pt = 0; pt < nSieveTarget; pt++)
		{
			unsigned int remainder = base_remainder + _offsets8Tuple1[pt];
			if (p<remainder)
				remainder -= p;
			unsigned long r = (p - remainder)*inv;
			unsigned int index = r % p;
#pragma acc kernels
			while (index < sieveSize)
			{
				sieve[(index) >> 3] |= (1 << ((index) & 7));
				index += p;
			}
		}
	}
	
	/*
	int cx = 0;
//#pragma acc data copy(sieve[0:sieveSizeBytes], c) 
//#pragma acc parallel
	for (int i = 0; i < sieveSize / 32; i++)
	{
		if (sieve[i] == 0xFFFFFFFF)
			continue;

		for (size_t p = 0; p < 32; p++)
		{
			if (sieve[i] & (1U << p))
				continue;
			int idx = i * 32 + p;
			c[cx] = idx;
			if (cx < 10000) cx++;
		}
	}
	printf("CX %u\n", cx);
	*/
}

void check_candidates(unsigned char * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * candidates)
{
	mpz_t zPrimeOriginOffset, zTempVar;
	mpz_init(zPrimeOriginOffset);
	mpz_init(zTempVar);
	int n1stPrimeSearchLimit = 0;
	unsigned long c[10000];
	const int sieveSizeBytes = sieveSize / 8;

//

	int cx = 0;
	//#pragma acc kernels
	 //loop gang
//#pragma acc data copy(sieve[0:sieveSizeBytes], c) 
//#pragma acc parallel
	for (int i = 0; i < sieveSize / 8; i++)
	{
		if(sieve[i] == 0xFF)
			continue;

		for (size_t p = 0; p < 8; p++)
		{
			if (sieve[i] & (1 << p))
				continue;
			int idx = i * 8 + p;
			c[cx] = idx;
			if (cx < 10000) cx++;
		}
	}
	printf("CX %u\n", cx);
//#pragma acc data copyout(c)		

	for (int j = 0; j < cx; j++)
	{
		mpz_mul_ui(zTempVar, zPrimorial, c[j]);
		mpz_add(zTempVar, zFirstSieveElement, zTempVar);
		mpz_set(zPrimeOriginOffset, zTempVar);

		unsigned long long nNonce = 0;
		unsigned int nPrimeCount = 0;
		unsigned int nSieveDifficulty = 0;
		unsigned long nStart = 0;
		unsigned long nStop = 0;
		unsigned long nLastOffset = 0;
		int firstPrimeAt = -1;

		nStop = 0; nPrimeCount = 0; nLastOffset = 0; firstPrimeAt = -1;
		double diff = 0;

		for (nStart = 0; nStart <= nStop + 12; nStart += 2)
		{
			if (mpz_probab_prime_p( zTempVar, 0) > 0 )
			{
				nStop = nStart;
				nPrimeCount++;
			}
			if (nPrimeCount == 0 && nStart >= n1stPrimeSearchLimit)
				break;

			if ((firstPrimeAt == -1 && nPrimeCount == 1))
			{
				mpz_set(zPrimeOriginOffset, zTempVar); // zPrimeOriginOffset = zTempVar
				firstPrimeAt = nStart;
			}

			mpz_add_ui(zTempVar, zTempVar, 2);
			nLastOffset += 2;
		}

		if (nPrimeCount >= nMinimumPrimeCount)
		{
			mpz_sub(zTempVar, zPrimeOriginOffset, zPrimeOrigin);

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
			nNonce = mpz2uint64(zTempVar);
#else
			nNonce = mpz_get_ui(zTempVar);
#endif
			candidates->push_back(nNonce);
		}
	}

}

AccSieve::AccSieve()
{
}

AccSieve::~AccSieve()
{
}
