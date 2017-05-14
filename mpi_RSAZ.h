
#ifndef RSAZ_AVX2_H
#define RSAZ_AVX2_H


typedef unsigned long mp_digit;
#define MP_OKAY 0    /* no error, all is well */
#define MP_YES 0     /* yes (boolean result)  */
#define MP_NO -1     /* no (boolean result)   */
#define MP_MEM -2    /* out of memory         */
#define MP_RANGE -3  /* argument out of range */
#define MP_BADARG -4 /* invalid parameter     */
#define MP_UNDEF -5  /* answer is undefined   */
#define MP_LAST_CODE MP_UNDEF

#ifdef __cplusplus
extern "C" {
#endif 
	int RSAZ_redundant_mod_exponent(mp_digit result[16], mp_digit base_norm[16], mp_digit exponent[16], mp_digit m_norm[16], mp_digit RR[16], mp_digit k0, int exponent_bits, int mod_bits, int const_time);
#ifdef __cplusplus
}
#endif



void AMS(mp_digit res[36], mp_digit a[40], mp_digit m[40], mp_digit k0, int repeat);

void AMM(mp_digit res[36], mp_digit a[40], mp_digit b[40], mp_digit m[40], mp_digit k0);

void red2norm1024(mp_digit *res, mp_digit *in);

#ifdef __cplusplus
extern "C" {
#endif 
void freebl_cpuid(unsigned long op, unsigned long *eax,	unsigned long *ebx, unsigned long *ecx,	unsigned long *edx);
#ifdef __cplusplus
}
#endif
#endif