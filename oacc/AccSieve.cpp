#include <stdio.h>
#include <string.h>
#include <fstream>
#include <stdlib.h>

using namespace std;

#include "AccSieve.h"
#ifdef __PGI
#include <openacc.h>
#endif
#include <bitset>

// !!!!!!!!!!!
//#define PRIMELIMIT 16384*2
//#define SIEVESIZE 65536 * 6
//#define SIEVETARGET 9

// !!!!!!!!
//#define PRIMELIMIT 16384 * 18
//#define SIEVESIZE 65536 * 3 *32
//#define SIEVETARGET 7


#define PRIMELIMIT 16384 * 5
#define SIEVESIZE 65536 * 4
#define SIEVETARGET 8

static uint32_t _offsets8Tuple1[8] = { 0, 2, 6, 8, 12, 18, 20, 26 };
static uint32_t _offsets10Tuple1[10] = { 0, 2, 6, 8, 12, 18, 20, 26, 30, 32 };
static uint32_t _offsets14Tuple1[14] = { 0, 2, 6, 8, 12, 18, 20, 26, 30, 32, 36, 42, 48, 50 };
static uint32_t _offsets24Tuple1[24] = { 0, 4, 6, 10, 16, 18, 24, 28, 30, 34, 40, 48, 54, 58, 60, 66, 70, 76, 84, 88, 90, 94, 96, 100 };


void pgisieve_bool(unsigned int * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates)
{

	mpz_t zPrimorialMod, zTempVar;
	mpz_init(zPrimorialMod);
	mpz_init(zTempVar);

	sieveSize = SIEVESIZE * 32;

	//const int sieveSizeBytes = sieveSize / 32;

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
	unsigned int * base_remainders = (unsigned int *)malloc(PRIMELIMIT * sizeof(unsigned int));

	for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
	{
		unsigned long  p = primes[i];
		base_remainders[i] = mpz_tdiv_ui(zTempVar, p);
	}

	int cx = 0;


	
#pragma acc data //copyin(sieve[0:sieveSizeBytes])
#pragma acc data copyout(candidates[0:MAXCANDIDATESPERSIEVE],cx)
{
		unsigned int indexes[PRIMELIMIT * (SIEVETARGET + 1)];
		//unsigned int * indexes[SIEVETARGET + 1];
		///unsigned int sieve[SIEVESIZE];
		bool sieve[SIEVESIZE];
		unsigned int ssz = SIEVESIZE*sizeof(bool);

#pragma acc enter data create(sieve[0:ssz])  create(indexes[0:SIEVETARGET+1][0:PRIMELIMIT*(SIEVETARGET+1)])
		#pragma acc parallel loop present(sieve) gang num_gangs(256)  worker num_workers(8) vector_length(128)
		{
			#pragma acc loop independent gang worker
			for (size_t i = 0; i < SIEVESIZE; i++)
			{
				sieve[i] = 0;
			}
		}
#pragma acc data copyin( primes[0:PRIMELIMIT], inverses[0:PRIMELIMIT], base_remainders[0:PRIMELIMIT], _offsets14Tuple1) 
#pragma acc parallel loop present( indexes, primes,inverses,base_remainders) gang num_gangs(128) worker num_workers(32) vector_length(32)
		{
			#pragma acc loop independent gang worker
			for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
			{
				unsigned long p = primes[i]; 
				//#pragma acc cache (indexes[0:SIEVETARGET*PRIMELIMIT])
				indexes[i*(SIEVETARGET + 1)] = (unsigned int)p;
				unsigned int inv = inverses[i];
				unsigned int base_remainder = base_remainders[i];
				#pragma acc loop  independent vector
				for (int pt = 0; pt < SIEVETARGET; pt++)
				{
					unsigned int remainder = base_remainder + _offsets14Tuple1[pt];
					if (p < remainder)
						remainder -= p;

					unsigned long r = (p - remainder)*inv;
					unsigned int idx = r % p;
					indexes[i*(SIEVETARGET + 1) + pt + 1] = idx;
				}
			}

		}//ACC parallel

#pragma acc parallel loop present(sieve, indexes) gang num_gangs(256) worker num_workers(128) vector_length(8)
		{			
			#pragma acc loop independent gang worker			
			for (unsigned int i = nPrimorialEndPrime *(SIEVETARGET + 1); i < PRIMELIMIT*(SIEVETARGET + 1); i++)
			{
				if ((i % (SIEVETARGET + 1)) != 0)
				{
					unsigned int pIdx2 = i - i % (SIEVETARGET + 1);
					unsigned long  p = indexes[pIdx2];
					int lc = (SIEVESIZE / p) + 1;

					//unsigned int index = indexes[i];
					////#pragma acc for private (index)
					//while (index < SIEVESIZE)
					//{
					//	
					//	//sieve[(index) >> 5] |= (1 << ((index) & 31));
					//	sieve[index] = 1;
					//	index += p;
					//}
					
					#pragma acc loop independent vector	
					for (int l = 0; l < lc; l++)
					{
						//#pragma acc loop independent vector
						//for (int pt = 1; pt < SIEVETARGET + 1; pt++)
						{
								unsigned int idx = indexes[i] + l*p;
								if (idx < SIEVESIZE)
								{
									//#pragma acc atomic update
									sieve[idx] = 1;
								}
								
						}
					}
				}
			}
		} //ACC parallel

/*
#pragma acc parallel loop present(sieve, indexes) gang num_gangs(256)  worker num_workers(8) vector_length(128)
		{
			#pragma acc loop independent gang worker			
			for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
			{
				//unsigned int pIdx2 = i - i % (SIEVETARGET + 1);
				unsigned long  p = indexes[i*(SIEVETARGET + 1)];
				int lc = (sieveSize / p) + 1;
				#pragma acc loop independent vector	
				for (int l = 0; l < lc; l++)
				{
					//#pragma acc loop independent vector
					for (int pt = 1; pt < SIEVETARGET+1; pt++)
					{
						unsigned int idx = indexes[i*(SIEVETARGET + 1) + pt] + l*p;
						if (idx < sieveSize)
							sieve[(idx) >> 5] |= (1UL << ((idx) & 31));
					}
				}
			}
		} //ACC parallel
*/

		//for (size_t i = 0; i <= SIEVETARGET; i++)
		//{
		//	free(indexes[i]);
		//}

		#pragma acc parallel loop present (candidates, sieve) gang num_gangs(128)  worker num_workers(8)
		{
			cx = 0;
#pragma acc loop independent worker
			for (int i = 0; i < SIEVESIZE; i++)
			{
				if (sieve[i] == 1)
					continue;
				candidates[cx] = i;
				if (cx < MAXCANDIDATESPERSIEVE) cx++;

			}
			//for (int i = 0; i < SIEVESIZE; i++)
			//{
			//	if (sieve[i] == 0xFFFFFFFF)
			//		continue;

			//	for (size_t p = 0; p < 32; p++)
			//	{
			//		if (sieve[i] & (1UL << p))
			//			continue;
			//		int idx = i * 32 + p;
			//		candidates[cx] = idx;
			//		if (cx < MAXCANDIDATESPERSIEVE) cx++;
			//	}
			//}

		}  // ACC parallel
#pragma acc exit data
#pragma acc data copyout(cx)
		

	} // ACC data
	candidates[cx] = -1;
	
	free(base_remainders);
}


// CPU Sieve
#ifdef __PGI
void pgisieve_cpu(uint64_t * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates)
#else
void pgisieve(uint64_t * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates)
#endif
{
	mpz_t zPrimorialMod, zTempVar;
	mpz_init(zPrimorialMod);
	mpz_init(zTempVar);

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
	unsigned int * base_remainders = (unsigned int *)malloc(PRIMELIMIT * sizeof(unsigned int));

	for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
	{
		unsigned long  p = primes[i];
		base_remainders[i] = mpz_tdiv_ui(zTempVar, p);
	}

	int cx = 0;

	unsigned int indexes[SIEVETARGET + 1][PRIMELIMIT];

	// clear the bits
	memset((wchar_t *)sieve1, 0x0, (sieveSize) / 8);

	for (int pt = 0; pt < SIEVETARGET; pt++)
	{
		for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
		{
			unsigned long p = primes[i];
			indexes[0][i] = (unsigned int)p;
			unsigned int inv = inverses[i];
			unsigned int base_remainder = base_remainders[i];

			unsigned int remainder = base_remainder + _offsets14Tuple1[pt];
			if (p < remainder)
				remainder -= p;
			unsigned long r = (p - remainder)*inv;
			unsigned int idx = r % p;
			indexes[pt + 1][i] = idx;
		}
	}

	for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
	{
		unsigned long  p = indexes[0][i];
		int lc = (sieveSize / p) + 1;
		for (int l = 0; l < lc; l++)
		{
			for (int pt = SIEVETARGET; pt >= 1; pt--)
			{
				unsigned int idx = indexes[pt][i] + l*p;
				if (idx < sieveSize)
				{
					sieve1[(idx) >> 6] |= (1ULL << ((idx) & 63));					
				}
			}
		}
	}

	for (size_t i = 0; i < sieveSize / 64; i++)
	{
		if(sieve1[i] == 0xFFFFFFFFFFFFFFFF)
			continue;
		for (size_t p = 0; p < 64; p++)
		{
			if(sieve1[i] & (1ULL << p))
				continue;

			candidates[cx] = i * 64 + p;
			if (cx < MAXCANDIDATESPERSIEVE) cx++;
		}
	}
	candidates[cx] = -1;
	free(base_remainders);
}

#ifdef __PGI
void pgisieve(uint64_t * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates)
#else
void pgisieve_GPU(uint64_t * sieve1, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates)
#endif
{


	mpz_t zPrimorialMod, zTempVar;
	mpz_init(zPrimorialMod);
	mpz_init(zTempVar);

	sieveSize = SIEVESIZE * 32;

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
	unsigned int * base_remainders = (unsigned int *)malloc(PRIMELIMIT * sizeof(unsigned int));

	for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
	{
		unsigned long  p = primes[i];
		base_remainders[i] = mpz_tdiv_ui(zTempVar, p);
	}

	int cx = 0;


#pragma acc data //copyin(sieve[0:sieveSizeBytes])
#pragma acc data copyout(candidates[0:MAXCANDIDATESPERSIEVE],cx)
	{
		unsigned int indexes[SIEVETARGET + 1][PRIMELIMIT];

		uint8_t sieve[SIEVESIZE*4];

#pragma acc enter data create(sieve[0:SIEVESIZE*4])  create(indexes[0:SIEVETARGET+1][0:PRIMELIMIT])
#pragma acc parallel loop present(sieve) gang num_gangs(256)  worker num_workers(8) vector_length(128)
		for (size_t i = 0; i < SIEVESIZE*4; i++)
		{
			sieve[i] = 0x0;
		}

#pragma acc data copyin( primes[0:PRIMELIMIT], inverses[0:PRIMELIMIT], base_remainders[0:PRIMELIMIT], _offsets14Tuple1) 
#pragma acc parallel loop present( indexes, primes,inverses,base_remainders) gang num_gangs(SIEVETARGET) worker num_workers(256) //vector_length(32)
		{
#pragma acc loop independent gang
			for (int pt = 0; pt < SIEVETARGET; pt++)
			{
#pragma acc loop independent worker
				for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
				{
					unsigned long p = primes[i];
					indexes[0][i] = (unsigned int)p;
					unsigned int inv = inverses[i];
					unsigned int base_remainder = base_remainders[i];

					unsigned int remainder = base_remainder + _offsets14Tuple1[pt];
					if (p < remainder)
						remainder -= p;
					unsigned long r = (p - remainder)*inv;
					unsigned int idx = r % p;
					indexes[pt + 1][i] = idx;
				}
			}
		}//ACC parallel


#pragma acc parallel loop present(sieve, indexes) gang num_gangs(128)  worker num_workers(64) vector_length(16)
		{
			//#pragma acc loop independent gang
			for (int pt = SIEVETARGET; pt >= 1; pt--)
			{
				#pragma acc loop independent gang worker //cache (indexes[pt]) //collapse(2) 
				for (unsigned int i = nPrimorialEndPrime; i < PRIMELIMIT; i++)
				{
					unsigned long  p = indexes[0][i];
					int lc = (sieveSize / p) + 1;
					#pragma acc loop independent vector	
					for (int l = 0; l < lc; l++)
					{
						//for (int pt = SIEVETARGET; pt >= 1; pt--)
						{
							//#pragma acc loop independent seq
							unsigned int idx = indexes[pt][i] + l*p;
							if (idx < sieveSize)
							{
								sieve[(idx) >> 3] |= (1 << ((idx) & 7));
							}
						}
					}
				}
			}
		} //ACC parallel 

#pragma acc parallel loop present (candidates, sieve) gang num_gangs(128)  worker num_workers(8)
		{
			cx = 0;
#pragma acc loop independent worker
			for (size_t i = 0; i < SIEVESIZE*4; i++)
			{
				if (sieve[i] == 0xFF)
					continue;
				for (size_t p = 0; p < 8; p++)
				{
					if (sieve[i] & (1 << p))
						continue;
					candidates[cx] = i * 8 + p;
					if (cx < MAXCANDIDATESPERSIEVE) cx++;
				}
			}
		}  // ACC parallel
#pragma acc exit data
#pragma acc data copyout(cx)
	} // ACC data
	candidates[cx] = -1;

	free(base_remainders);
}


int find_tuples(unsigned long * candidates, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * nonces)
{	
	mpz_t zPrimeOriginOffset, zTempVar, zTempVar2;
	mpz_init(zPrimeOriginOffset);
	mpz_init(zTempVar);
	mpz_init(zTempVar2);
	int n1stPrimeSearchLimit = 0;
	int cx = 0;
	int nPrimes = 0;

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

		mpz_add_ui(zTempVar2, zTempVar, 18);
		if (mpz_probab_prime_p(zTempVar2, 0) > 0)
			n1stPrimeSearchLimit = 12;
		else
		{
			mpz_add_ui(zTempVar2, zTempVar, 20);
			if (mpz_probab_prime_p(zTempVar2, 0) > 0)
				n1stPrimeSearchLimit = 20;
			else
				//{
				//	mpz_add_ui(zTempVar2, zTempVar, 26);
				//	if (mpz_probab_prime_p(zTempVar2, 0) > 0)
				//		n1stPrimeSearchLimit = 26;
				//	else
				//		continue;
				//}
				continue;
				
				
		}

		nStop = 0; nPrimeCount = 0; nLastOffset = 0; firstPrimeAt = -1;
		double diff = 0;

		for (nStart = 0; nStart <= nStop + 12; nStart += 2)
		{
			if (mpz_probab_prime_p(zTempVar, 0) > 0)
			{
				nStop = nStart;
				nPrimeCount++;
				nPrimes++;
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
	//if (nonces->size() > 0)
	//	printf("Found %u %u+ tuples out of %u candidates\n", nonces->size(), nMinimumPrimeCount, cx);
	return nPrimes;
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
			n1stPrimeSearchLimit = 20;
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
		printf("acc candidates %lu out of %u\n", candidates->size(), cx);

}

AccSieve::AccSieve()
{
}

AccSieve::~AccSieve()
{
}
