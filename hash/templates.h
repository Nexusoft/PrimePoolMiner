/*******************************************************************************************
 
			Hash(BEGIN(Satoshi[2010]), END(Sunny[2012])) == Videlicet[2014] ++
   
 [Learn and Create] Viz. http://www.opensource.org/licenses/mit-license.php
  
*******************************************************************************************/
#ifndef NEXUS_TEMPLATES_H
#define NEXUS_TEMPLATES_H

#include "uint1024.h"
#include "skein.h"

/** Ensure the c function names are not treated as mangled for cross platform support (Issue on OSX) **/
#if defined(__cplusplus)
extern "C" {
#endif

#include "KeccakHash.h"
	
#if defined(__cplusplus)
}
#endif

/** Hashing template for Checksums **/
template<typename T1>
inline uint64 SK64(const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1];
	
    uint64 skein;
	Skein_256_Ctxt_t ctx;
	Skein_256_Init  (&ctx, 64);
	Skein_256_Update(&ctx, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]));
	Skein_256_Final (&ctx, (unsigned char *)&skein);
	
    uint64 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize(&ctx_keccak, 1344, 256, 64, 0x06);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 64);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for Address Generation **/
inline uint64 SK64(const std::vector<unsigned char>& vch)
{
	static unsigned char pblank[1];
	
    uint64 skein;
	Skein_256_Ctxt_t ctx;
	Skein_256_Init(&ctx, 64);
	Skein_256_Update(&ctx, (unsigned char *)&vch[0], vch.size());
	Skein_256_Final(&ctx, (unsigned char *)&skein);
	
    uint64 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize(&ctx_keccak, 1344, 256, 64, 0x06);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 64);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for Address Generation **/
inline uint256 SK256(const std::vector<unsigned char>& vch)
{
	static unsigned char pblank[1];
	
    uint256 skein;
	Skein_256_Ctxt_t ctx;
	Skein_256_Init(&ctx, 256);
	Skein_256_Update(&ctx, (unsigned char *)&vch[0], vch.size());
	Skein_256_Final(&ctx, (unsigned char *)&skein);
	
    uint256 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize_SHA3_256(&ctx_keccak);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 256);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for Address Generation **/
template<typename T1>
inline uint256 SK256(const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1];
	
    uint256 skein;
	Skein_256_Ctxt_t ctx;
	Skein_256_Init  (&ctx, 256);
	Skein_256_Update(&ctx, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]));
	Skein_256_Final (&ctx, (unsigned char *)&skein);
	
    uint256 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize_SHA3_256(&ctx_keccak);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 256);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for Trust Key Hash **/
template<typename T1>
inline uint512 SK512(const std::vector<unsigned char>& vch, const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1];
	
    uint512 skein;
	Skein_512_Ctxt_t ctx;
	Skein_512_Init(&ctx, 512);
	Skein_512_Update(&ctx, (unsigned char *)&vch[0], vch.size());
	Skein_512_Update(&ctx, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]));
	Skein_512_Final(&ctx, (unsigned char *)&skein);
	
    uint512 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize_SHA3_512(&ctx_keccak);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 512);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for TX hash **/
template<typename T1>
inline uint512 SK512(const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1];
	
    uint512 skein;
	Skein_512_Ctxt_t ctx;
	Skein_512_Init  (&ctx, 512);
	Skein_512_Update(&ctx, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]));
	Skein_512_Final (&ctx, (unsigned char *)&skein);
	
    uint512 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize_SHA3_512(&ctx_keccak);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 512);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for TX hash **/
template<typename T1, typename T2>
inline uint512 SK512(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
	static unsigned char pblank[1];
	
    uint512 skein;
	Skein_512_Ctxt_t ctx;
	Skein_512_Init  (&ctx, 512);
	Skein_512_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    Skein_512_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
	Skein_512_Final (&ctx, (unsigned char *)&skein);
	
    uint512 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize_SHA3_512(&ctx_keccak);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 512);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template for TX hash **/
template<typename T1, typename T2, typename T3>
inline uint512 SK512(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end,
                    const T3 p3begin, const T3 p3end)
{
	static unsigned char pblank[1];
	
    uint512 skein;
	Skein_512_Ctxt_t ctx;
	Skein_512_Init  (&ctx, 512);
	Skein_512_Update(&ctx, (p1begin == p1end ? pblank : (unsigned char*)&p1begin[0]), (p1end - p1begin) * sizeof(p1begin[0]));
    Skein_512_Update(&ctx, (p2begin == p2end ? pblank : (unsigned char*)&p2begin[0]), (p2end - p2begin) * sizeof(p2begin[0]));
    Skein_512_Update(&ctx, (p3begin == p3end ? pblank : (unsigned char*)&p3begin[0]), (p3end - p3begin) * sizeof(p3begin[0]));
	Skein_512_Final (&ctx, (unsigned char *)&skein);
	
    uint512 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize_SHA3_512(&ctx_keccak);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 512);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template used for Private Keys **/
template<typename T1>
inline uint576 SK576(const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1];
	
    uint576 skein;
	Skein1024_Ctxt_t ctx;
	Skein1024_Init(&ctx, 576);
	Skein1024_Update(&ctx, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]));
	Skein1024_Final(&ctx, (unsigned char *)&skein);
	
    uint576 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize(&ctx_keccak, 1024, 576, 576, 0x06);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 576);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
	return keccak;
}

/** Hashing template used to build Block Hashes **/
template<typename T1>
inline uint1024 SK1024(const T1 pbegin, const T1 pend)
{
	static unsigned char pblank[1];
	
    uint1024 skein;
	Skein1024_Ctxt_t ctx;
	Skein1024_Init(&ctx, 1024);
	Skein1024_Update(&ctx, (pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]));
	Skein1024_Final(&ctx, (unsigned char *)&skein);
	
	uint1024 keccak;
	Keccak_HashInstance ctx_keccak;
	Keccak_HashInitialize(&ctx_keccak, 576, 1024, 1024, 0x05);
	Keccak_HashUpdate(&ctx_keccak, (unsigned char *)&skein, 1024);
	Keccak_HashFinal(&ctx_keccak, (unsigned char *)&keccak);
	
    return keccak;
}

#endif