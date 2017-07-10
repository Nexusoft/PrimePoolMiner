#pragma once
#include "core.h"
#include "mpi_RSAZ.h"
			 
#define PP 0xE221F97C30E94E1DL	/* 3 x 5 x 7 x 11 x ... x 53 */
#define PP2	0x6329899EA9F2714BL /* 61 x 67 x 71 x 73 x 79 x 83 x 89 x 97 x 101 */
#define PP3 0x58EDCB4C9ED39C8BL /* 107 x 109 x 113 x 127 x 131 x 137 x 139 x 149 */
#define PP4 0x9966FF94FD516FBL  /* 157 x 163 x 167 x 173 x 179 x 181 x 191 */
#define PP_INVERTED 0x21CFE6CFC938B36BL
#define PP_FIRST_OMITTED 59
#define MPZ_DIGIT(MP, N) (MP)->_mp_d[(N)]
#define MAXCANDIDATESPERSIEVE 5000

class CPrimeTest
{
	static constexpr uint32_t bit4Mask = 1UL << 4;
	static constexpr uint32_t bit5Mask = 1UL << 5;
	static constexpr uint32_t bit6Mask = 1UL << 6;
	static constexpr uint32_t bit7Mask = 1UL << 7;
	static constexpr uint32_t bit8Mask = 1UL << 8;
	static constexpr uint32_t bit9Mask = 1UL << 9;
	static constexpr uint32_t bit10Mask = 1UL << 10;

public:
	mpz_t zTwo;
	mpz_t zNm1; /* "zNm1" = "zP minus one" */
	mpz_t zR;
	mpz_t zN;
	int has_avx;
	int has_avx2;
	int use_avx2;
	uint64_t nPrimorial;
	int nSieveTarget;

	CPrimeTest(uint64_t primorial);

	~CPrimeTest();

	bool FermatTest(bool useTrialDivision = false);
	bool CheckOffsetUsingCandidateMask(uint16_t candidateMask, uint32_t * tupleOffsets, uint16_t offset);

	int FindTuples(uint64_t * candidates, uint32_t * candidateMasks, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, uint32_t * tupleOffsets, std::vector<std::pair<uint64_t, uint16_t>> * resultNonces);

	int mp_exptmod(const mpz_ptr inBase, const mpz_ptr exponent, mpz_ptr modulus, mpz_ptr result);
};
