#pragma once
#include "util.h"

class Sieve
{
private:
	uint16_t*	pSieve;
	uint32_t	nSieveSize;

public:
	Sieve(uint32_t siveveSize);
	~Sieve();
};

