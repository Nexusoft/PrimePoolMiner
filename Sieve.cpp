#include "Sieve.h"
#include "core.h"



Sieve::Sieve(uint32_t siveveSize)
{
	pSieve = (uint16_t *)aligned_alloc(64, (nSieveSize) * sizeof(uint16_t));
}


Sieve::~Sieve()
{
	free(pSieve);
}
