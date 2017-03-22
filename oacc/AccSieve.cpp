#include <stdio.h>
#include <string.h>
#include <fstream>
#include <stdlib.h>

using namespace std;

#include "AccSieve.h"


static unsigned int _offsets8Tuple1[8] = { 0, 2, 6, 8, 12, 18, 20, 26 };
static int nSieveTarget = 6;



void pgisieve(unsigned int * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, long * candidates)
{
	const int sieveSizeBytes = sieveSize / 32;

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



#pragma acc data copy(sieve[0:sieveSizeBytes])
	{
//#pragma acc data copy(candidates[0:1000])	
//#pragma acc data copyin(sieve[0:sieveSizeBytes])
#pragma acc data copyin( primes[0:nPrimeLimit], inverses[0:nPrimeLimit], base_remainders[0:nPrimeLimit], _offsets8Tuple1) 
#pragma acc parallel
#pragma acc loop independent
		for (unsigned int i = nPrimorialEndPrime; i < nPrimeLimit; i++)
		{
			unsigned long  p = primes[i];
			unsigned int inv = inverses[i];
			unsigned int base_remainder = base_remainders[i];
#pragma acc loop independent
			for (int pt = 0; pt < nSieveTarget; pt++)
			{
				unsigned int remainder = base_remainder + _offsets8Tuple1[pt];
				if (p < remainder)
					remainder -= p;
				unsigned long r = (p - remainder)*inv;
				unsigned int idx = r % p;

				const int silo = sieveSize / p;
#pragma acc loop worker
				for (unsigned int k = 0; k <= silo; k++)
				{
					if (idx < sieveSize)
						sieve[(idx) >> 5] |= (1UL << ((idx) & 31));
					idx += p;
				}
				//while (index < sieveSize)
				//{
				//	sieve[(index) >> 5] |= (1UL << ((index) & 31));
				//	index += p;
				//}
			}
		}
	}
//#pragma acc wait 

	int cx = 0;
	//#pragma acc parallel
	//#pragma acc loop independent
//#pragma acc loop worker
	for (int i = 0; i < sieveSize / 32; i++)
	{
		if (sieve[i] == 0xFFFFFFFF)
			continue;

		for (size_t p = 0; p < 32; p++)
		{
			if (sieve[i] & (1UL << p))
				continue;
			int idx = i * 32 + p;
			candidates[cx] = idx;
			if (cx < MAXCANDIDATESPERSIEVE) cx++;
		}
	}
	candidates[cx] = -1;

}
void  find_tuples(long * candidates, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * nonces)
{
	mpz_t zPrimeOriginOffset, zTempVar, zTempVar2;
	mpz_init(zPrimeOriginOffset);
	mpz_init(zTempVar);
	mpz_init(zTempVar2);
	int n1stPrimeSearchLimit = 2;
	int cx = 0;


	while (candidates[cx] != -1 && cx < MAXCANDIDATESPERSIEVE)
	{	
		mpz_mul_ui(zTempVar, zPrimorial, candidates[cx]);
		cx++;
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
			if (mpz_probab_prime_p(zTempVar, 0) > 0)
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
			nonces->push_back(nNonce);
		}
		
	}
	if (nonces->size() > 0)
		printf("Found %u %u+ tuples out of %u candidates\n", nonces->size(), nMinimumPrimeCount, cx);
}

void check_candidates(unsigned int * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * candidates)
{
	mpz_t zPrimeOriginOffset, zTempVar, zTempVar2;
	mpz_init(zPrimeOriginOffset);
	mpz_init(zTempVar);
	mpz_init(zTempVar2);
	int n1stPrimeSearchLimit = 0;
	unsigned long c[MAXCANDIDATESPERSIEVE];
	const int sieveSizeBytes = sieveSize / 32;

//

	int cx = 0;
	//#pragma acc kernels
	 //loop gang
//#pragma acc data copy(sieve[0:sieveSizeBytes], c) 
//#pragma acc parallel
	for (int i = 0; i < sieveSize / 32; i++)
	{
		if(sieve[i] == 0xFFFFFFFF)
			continue;

		for (size_t p = 0; p < 32; p++)
		{
			if (sieve[i] & (1UL << p))
				continue;
			int idx = i * 32 + p;
			c[cx] = idx;
			if (cx < MAXCANDIDATESPERSIEVE) cx++;
		}
	}
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

		mpz_add_ui(zTempVar2, zTempVar, 20);
		if (mpz_probab_prime_p(zTempVar2,0)>0)
			n1stPrimeSearchLimit = 12;
		else
		{
			mpz_add_ui(zTempVar2, zTempVar, 18);
			if (mpz_probab_prime_p(zTempVar2, 0)>0)
				n1stPrimeSearchLimit = 18;
			else
				continue;
		}

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
	if (candidates->size() > 0)
		printf("acc candidates %u out of  %u\n", candidates->size(), cx);

}

AccSieve::AccSieve()
{
}

AccSieve::~AccSieve()
{
}
