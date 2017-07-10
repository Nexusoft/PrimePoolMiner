#include "PrimeTest.h"



CPrimeTest::CPrimeTest(uint64_t primorial)
{
	nPrimorial = primorial;

	const char zTmpPrimeHex[] = "b6122b34ba26087abc80aaa2f75c48d772ec4f8e377ced162efc9a56c167f42ddec5ddcac936f3a0e4ae928b8f61ce451221bd6e71291c0717a667a1418a6bfdb5b1aba05b4d3d5a170e50e05a0d11c4d40075d5cc84625c0bd378f361ed8c438c47b2731dd93f7dfa26ca0f582fca850dafe98bd5c64e8c127462b202ac1bb7";

	mpz_init_set_str(zN, zTmpPrimeHex, 16);
	mpz_init_set(zTwo, zN);
	mpz_set_ui(zTwo, 2);
	mpz_init(zNm1);
	mpz_sub_ui(zNm1, zN, 1L);
	mpz_init(zR);
	//mpz_pow_ui(zR, zN, 2); /* mpz_powm_ui needs excessive memory so preallocate 2x more!!! */
	mpz_realloc(zR, zN->_mp_size * 2);

#if defined(_MSC_VER) || defined(__CYGWIN__)
	use_avx2 = 0;
#else

	unsigned long eax, ebx, ecx, edx;
	freebl_cpuid(1, &eax, &ebx, &ecx, &edx);
	has_avx = (ecx & (1 << 28)) != 0 ? 1 : -1;
	freebl_cpuid(7, &eax, &ebx, &ecx, &edx);
	has_avx2 = (ebx & (1 << 5)) != 0 ? 1 : -1;
	use_avx2 = (has_avx > 0) && (has_avx2 > 0);
#endif
}

bool CPrimeTest::FermatTest(bool useTrialDivision)
{
#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
	useTrialDivision = false;
	use_avx2 = false;
#endif

	if (useTrialDivision)
	{
		mp_limb_t r = mpn_preinv_mod_1(zN->_mp_d, (mp_size_t)zN->_mp_size, (mp_limb_t)PP, (mp_limb_t)PP_INVERTED);
		if (r % 3 == 0 || r % 5 == 0 || r % 7 == 0 || r % 11 == 0 || r % 13 == 0 || r % 17 == 0 || r % 19 == 0 || r % 23 == 0 || r % 29 == 0 || r % 31 == 0 || r % 37 == 0 || r % 41 == 0 || r % 43 == 0 || r % 47 == 0 || r % 53 == 0)
			return false;
		else
		{
			r = mpn_mod_1(zN->_mp_d, (mp_size_t)zN->_mp_size, (mp_limb_t)PP2);
			/* 61 x 67 x 71 x 73 x 79 x 83 x 89 x 97 x 101 */
			if (r % 61 == 0 || r % 67 == 0 || r % 71 == 0 || r % 73 == 0 || r % 79 == 0 || r % 83 == 0 || r % 89 == 0 || r % 97 == 0 || r % 101 == 0)
				return false;
			else
			{
				r = mpn_mod_1(zN->_mp_d, (mp_size_t)zN->_mp_size, (mp_limb_t)PP3);
				/* 107 x 109 x 113 x 127 x 131 x 137 x 139 x 149 */
				if (r % 107 == 0 || r % 109 == 0 || r % 113 == 0 || r % 127 == 0 || r % 131 == 0 || r % 137 == 0 || r % 139 == 0 || r % 149 == 0)
					return false;
				else
				{
					r = mpn_mod_1(zN->_mp_d, (mp_size_t)zN->_mp_size, (mp_limb_t)PP4);
					/* 157 x 163 x 167 x 173 x 179 x 181 x 191 */
					if (r % 157 == 0 || r % 163 == 0 || r % 167 == 0 || r % 173 == 0 || r % 179 == 0 || r % 181 == 0 || r % 191 == 0)
						return false;
				}
			}
		}
	}

	zNm1->_mp_d[0] = zN->_mp_d[0] - 1;
	zNm1->_mp_d[1] = zN->_mp_d[1];
#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
	zNm1->_mp_d[2] = zN->_mp_d[2];
	zNm1->_mp_d[3] = zN->_mp_d[3];
#endif

	if (use_avx2)
		mp_exptmod(zTwo, zNm1, zN, zR);
	else
		mpz_powm(zR, zTwo, zNm1, zN);

	if (mpz_cmp_ui(zR, 1) != 0)
		return false;
	return true;
}

bool CPrimeTest::CheckOffsetUsingCandidateMask(uint16_t candidateMask, uint32_t * tupleOffsets, uint16_t offset)
{
	bool bIngnore = false;
	bool bTrialDiv = false;
	for (int i = 0; i < nSieveTarget; i++)
	{
		if (tupleOffsets[i] == offset)
		{
			bTrialDiv = true;
			uint16_t mask = 1U << i;
			if ((candidateMask & mask) == mask)
				bIngnore = true;
			break;
		}
		else if (tupleOffsets[i] > offset)
			break;
	}
	return true;
}


int CPrimeTest::FindTuples(uint64_t * candidates, uint32_t * candidateMasks, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, uint32_t * tupleOffsets, std::vector<std::pair<uint64_t, uint16_t>> * resultNonces)
{
	int n1stPrimeSearchLimit = 18;
	int cx = 0;
	int nPrimes = 0;

	mpz_sub(zN, zFirstSieveElement, zPrimeOrigin);

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
	uint64_t nonce;
	size_t size = 1;
	mpz_export(&nonce, &size, 1, sizeof(nonce), 0, 0, zN);
#else
	uint64_t nonce = zN->_mp_d[0];
#endif

	mpz_set(zN, zFirstSieveElement);
	mpz_set(zNm1, zFirstSieveElement);
	const mp_limb_t d0 = zN->_mp_d[0];
	const mp_limb_t d1 = zN->_mp_d[1];
	const mp_limb_t d2 = zN->_mp_d[2];
	int firstPrimeAt;

	uint16_t nStart, nStop, nPrimeCount, nLastOffset;
	while (cx < MAXCANDIDATESPERSIEVE)
	{
		uint64_t candidate = candidates[cx];
		if (candidate == UINT64_MAX)
			break;
		uint16_t candMask = candidateMasks[cx];
		zN->_mp_d[0] = d0;
		zN->_mp_d[1] = d1;
		zN->_mp_d[2] = d2;
		const uint64_t n = nPrimorial * candidate;
		cx++;
#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
		mpz_t zTmp1;
		mpz_init(zTmp1);
		mpz_import(zTmp1, 1, 1, sizeof(n), 0, 0, &n);
		mpz_add(zN, zN, zTmp1);
#else
		mpz_add_ui(zN, zN, n);
#endif

		const uint64_t tmp = zN->_mp_d[0];
		const uint16_t tuppleIndexA = 4;
		const uint16_t tuppleIndexB = 5;
		mpz_add_ui(zN, zN, tupleOffsets[tuppleIndexA]);
		const uint32_t bitMaskA = (1UL << tuppleIndexA);

		if (((candMask & bitMaskA) != bitMaskA) && FermatTest())
		{
			n1stPrimeSearchLimit = tupleOffsets[tuppleIndexA];
		}
		else
		{
			//mark previous position as not prime
			candMask = candMask | bitMaskA;

			mpz_add_ui(zN, zN, tupleOffsets[tuppleIndexB] - tupleOffsets[tuppleIndexA]);
			const uint32_t bitMaskB = (1UL << tuppleIndexB);
			if (((candMask & bitMaskB) != bitMaskB) && FermatTest())
			{
				candidateHit2Count++;
				n1stPrimeSearchLimit = tupleOffsets[tuppleIndexB];
			}
			else
				//{ 					
				//	mpz_add_ui(zN, zN, 6);
				//	if (((candMask & bit7Mask) != bit7Mask) && FermatTest())
				//		n1stPrimeSearchLimit = tupleOffsets[7];
				//	else
				//		continue;
				//}
				continue;
		}
		nPrimes++;
		candidateHitCount++;


		zN->_mp_d[0] = tmp;
		//zN->_mp_d[1] = d1;
		nStop = 0; nPrimeCount = 0; nLastOffset = 0; firstPrimeAt = -1;
		double diff = 0;
		bool bTrialDiv = false;
		bool inTupple = false;
		for (nStart = 0; nStart <= nStop + 12; nStart += 2)
		{
			bool bIngnore = false;
			for (int i = 0; i < nSieveTarget; i++)
			{
				if (tupleOffsets[i] == nLastOffset)
				{
					bTrialDiv = true;
					uint16_t mask = 1U << i;
					if ((candMask & mask) == mask)
						bIngnore = true;
					break;
				}
				else if (tupleOffsets[i] > nLastOffset)
					break;
			}
			if (bIngnore)
			{
				mpz_add_ui(zN, zN, 2);
				nLastOffset += 2;
				continue;
			}

			if (FermatTest(bTrialDiv))
			{
				nStop = nStart;
				nPrimeCount++;
			}
			if (nPrimeCount == 0 && nStart >= n1stPrimeSearchLimit)
				break;

			if ((firstPrimeAt == -1 && nPrimeCount == 1))
			{
				//mpz_set(zPrimeOriginOffset, zTempVar); // zPrimeOriginOffset = zTempVar
				firstPrimeAt = nStart;
			}

			mpz_add_ui(zN, zN, 2);
			nLastOffset += 2;
			if (nStart >= (nStop + 12) && nLastOffset < n1stPrimeSearchLimit)
			{
				//printf("Stopped at %u before touching search limit %u\n", nLastOffset, n1stPrimeSearchLimit);
				firstPrimeAt = n1stPrimeSearchLimit;
				nLastOffset = n1stPrimeSearchLimit;
				nStop = n1stPrimeSearchLimit;
				nStart = n1stPrimeSearchLimit;
				zN->_mp_d[0] = tmp;
				mpz_add_ui(zN, zN, n1stPrimeSearchLimit);
			}

		}
		if (nPrimeCount >= 2)
		{
			uint64_t nNonce = nonce + n + firstPrimeAt;

			if (firstPrimeAt <= 10)
			{
				int nStop = 12;
				for (int i = firstPrimeAt + 2 ; i <= nStop; i += 2)
				{
					zN->_mp_d[0] = firstPrimeAt + tmp - i;
					if (FermatTest())
					{
						//printf("\n!!! Prime before N (%u - fpa:%u) wiht diff %u \n\n", i , firstPrimeAt, nPrimeCount);
						nNonce = nonce + n + firstPrimeAt - i ;
						nStop += 12;
						nPrimeCount++;
					}
				}
			}
			if (nPrimeCount >= 3)
				resultNonces->push_back(std::pair<uint64_t, uint16_t>(nNonce, nPrimeCount));
		}
	}
	candidateCount += cx;
	//if (nonces->size() > 0)
	//	printf("Found %u %u+ tuples out of %u candidates\n", nonces->size(), nMinimumPrimeCount, cx);
	return nPrimes;
}


/* Compute T = (P ** -1) mod MP_RADIX.  Also works for 16-bit mp_digits.
** This technique from the paper "Fast Modular Reciprocals" (unpublished)
** by Richard Schroeppel (a.k.a. Captain Nemo).
*/
mp_digit s_mp_invmod_radix(mp_digit P)
{
	mp_digit T = P;
	T *= 2 - (P * T);
	T *= 2 - (P * T);
	T *= 2 - (P * T);
	T *= 2 - (P * T);
#if !defined(MP_USE_UINT_DIGIT)
	T *= 2 - (P * T);
	T *= 2 - (P * T);
#endif
	return T;
}


int CPrimeTest::mp_exptmod(const mpz_ptr inBase, const mpz_ptr exponent, mpz_ptr modulus, mpz_ptr result)
{
#if defined(_MSC_VER) || defined(__CYGWIN__)
	return MP_UNDEF;
#else
	mp_digit n0;
	mpz_t RR;
	mp_digit a_locl[32] = { 0 };

	mpz_init(RR);
	mpz_set_ui(RR, 1);
	mpz_mul_2exp(RR, RR, modulus->_mp_size * 2 * 64);
	mpz_mod(RR, RR, modulus);
	n0 = 0 - s_mp_invmod_radix(MPZ_DIGIT(modulus, 0));

	memcpy(a_locl, inBase->_mp_d, inBase->_mp_size * 8);

	if (!RSAZ_redundant_mod_exponent((mp_digit*)result->_mp_d, a_locl, (mp_digit*)exponent->_mp_d, (mp_digit *)modulus->_mp_d, (mp_digit*)RR->_mp_d, n0, 1024, 1024, 0))
	{
		return MP_MEM;
	}
	result->_mp_size = modulus->_mp_size;
	while ((result->_mp_d[result->_mp_size - 1] == 0) && result->_mp_size > 0) result->_mp_size--;

	mpz_clear(RR);

	return MP_OKAY;
#endif
}


CPrimeTest::~CPrimeTest()
{
}
