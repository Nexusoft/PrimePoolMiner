#pragma once
#ifndef ACCSIEVE_H__
#define ACCSIEVE_H__

#include <gmp.h>
#include <vector>
#include <stdint.h>

#define MAXCANDIDATESPERSIEVE 1000

#ifdef __cplusplus
extern "C" {
#endif 
	void pgisieve(unsigned int * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates);
	
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif 
	void check_candidates(unsigned int * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * candidates);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif 
	int find_tuples(unsigned long * candidates, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<unsigned long> * nonces);
#ifdef __cplusplus
}
#endif

class  AccSieve {

public:
	AccSieve();
	~AccSieve();
};

#endif //ACCSIEVE
