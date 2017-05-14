/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**********************************************************************************/
/* Implementation of 1024/2048-bit Modular Exponent, using the AVX2 instruction   */
/* set.                                                                           */
/* Reference:                                                                     */
/* [1] S. Gueron, V. Krasnov: "Software implementation of modular exponentiation, */
/*     using advanced vector instructions architectures", WAIFI'12 Proceedings of */
/*     the 4th international conference on Arithmetic of Finite Fields Pages      */
/*     119-135                                                                    */
/* [2] S. Gueron: “Efficient Software Implementations of Modular Exponentiation”, */
/*     Journal of Cryptographic Engineering 2:31-43 (2012).                       */
/* [4] S. Gueron, V. Krasnov: “[PATCH] Efficient and side channel analysis        */
/*     resistant 512-bit and 1024-bit modular exponentiation for optimizing       */
/*     RSA1024 and RSA2048 on x86_64 platforms",                                  */
/*     http://rt.openssl.org/Ticket/Display.html?id=2582&user=guest&pass=guest    */
/**********************************************************************************/

#include "mpi_RSAZ.h"
#include <string.h>
#include <malloc.h>


void scatter(unsigned short *out, unsigned short *in, int wsize)
{
    int i;
    int step = 1<<wsize;
    int period = step*18;

    for(i=0; i<4; i++)
    {
        out[step*0] = in[0];
        out[step*1] = in[1];

        out[step*2] = in[4];
        out[step*3] = in[5];

        out[step*4] = in[8];
        out[step*5] = in[9];

        out[step*6] = in[12];
        out[step*7] = in[13];

        out[step*8] = in[16];
        out[step*9] = in[17];

        out[step*10] = in[20];
        out[step*11] = in[21];

        out[step*12] = in[24];
        out[step*13] = in[25];

        out[step*14] = in[28];
        out[step*15] = in[29];

        out[step*16] = in[32];
        out[step*17] = in[33];
        
        out+=period;
        in+=36;
    }
}

void gather(unsigned short *in, unsigned short *out, int wsize)
{
    int i;
    int step = 1<<wsize;
    int period = step*18;

    for(i=0; i<4; i++)
    {
        out[0]=in[step*0];
        out[1]=in[step*1];

        out[4]=in[step*2];
        out[5]=in[step*3];

        out[8]=in[step*4];
        out[9]=in[step*5];

        out[12]=in[step*6]; 
        out[13]=in[step*7]; 

        out[16]=in[step*8]; 
        out[17]=in[step*9]; 

        out[20]=in[step*10]; 
        out[21]=in[step*11]; 

        out[24]=in[step*12]; 
        out[25]=in[step*13]; 

        out[28]=in[step*14]; 
        out[29]=in[step*15]; 

        out[32]=in[step*16]; 
        out[33]=in[step*17];

        out+=36;
        in+=period;
    }
}

void norm2red(unsigned long out[40], unsigned long *in)
{
   out[0] = in[0] & 0x1fffffff;
   out[1] = (in[0] >> 29) & 0x1fffffff;
   out[2] = ((in[0] >> 58) ^ (in[1] << 6)) & 0x1fffffff;
   out[3] = (in[1] >> 23) & 0x1fffffff;   
   out[4] = ((in[1] >> 52) ^ (in[2] << 12)) & 0x1fffffff;
   out[5] = (in[2] >> 17) & 0x1fffffff;
   out[6] = ((in[2] >> 46) ^ (in[3] << 18)) & 0x1fffffff;
   out[7] = (in[3] >> 11) & 0x1fffffff;
   out[8] = ((in[3] >> 40) ^ (in[4] << 24)) & 0x1fffffff;
   out[9] = (in[4] >> 5) & 0x1fffffff;

   out[10] = (in[4] >> 34) & 0x1fffffff;
   out[11] = ((in[4] >> 63) ^ (in[5] << 1)) & 0x1fffffff;
   out[12] = (in[5] >> 28) & 0x1fffffff;
   out[13] = ((in[5] >> 57) ^ (in[6] << 7)) & 0x1fffffff;
   out[14] = (in[6] >> 22) & 0x1fffffff;
   out[15] = ((in[6] >> 51) ^ (in[7] << 13)) & 0x1fffffff;
   out[16] = (in[7] >> 16) & 0x1fffffff;
   out[17] = ((in[7] >> 45) ^ (in[8] << 19)) & 0x1fffffff;
   out[18] = (in[8] >> 10) & 0x1fffffff;
   out[19] = ((in[8] >> 39) ^ (in[9] << 25)) & 0x1fffffff;
   out[20] = (in[9] >> 4) & 0x1fffffff;

   out[21] = (in[9] >> 33) & 0x1fffffff;
   out[22] = ((in[9] >> 62) ^ (in[10] << 2)) & 0x1fffffff;
   out[23] = (in[10] >> 27) & 0x1fffffff;
   out[24] = ((in[10] >> 56) ^ (in[11] << 8)) & 0x1fffffff;
   out[25] = (in[11] >> 21) & 0x1fffffff;
   out[26] = ((in[11] >> 50) ^ (in[12] << 14)) & 0x1fffffff;
   out[27] = (in[12] >> 15) & 0x1fffffff;
   out[28] = ((in[12] >> 44) ^ (in[13] << 20)) & 0x1fffffff;
   out[29] = (in[13] >> 9) & 0x1fffffff;
   out[30] = ((in[13] >> 38) ^ (in[14] << 26)) & 0x1fffffff;
   out[31] = (in[14] >> 3) & 0x1fffffff;

   out[32] = (in[14] >> 32)& 0x1fffffff;
   out[33] = ((in[14] >> 61) ^ (in[15] << 3)) & 0x1fffffff;
   out[34] = (in[15] >> 26) & 0x1fffffff;
   out[35] = (in[15] >> 55) & 0x1fffffff;

   out[36] = 0;
   out[37] = 0;
   out[38] = 0;
   out[39] = 0;
}

void scatter_2048(unsigned short *out, unsigned short *in, int wsize)
{
    int i;
    int step = 1<<wsize;
    int period = step*18;
    for(i=0; i<8; i++)
    {
        out[step*0] = in[0];
        out[step*1] = in[1];

        out[step*2] = in[4];
        out[step*3] = in[5];

        out[step*4] = in[8];
        out[step*5] = in[9];

        out[step*6] = in[12];
        out[step*7] = in[13];

        out[step*8] = in[16];
        out[step*9] = in[17];

        out[step*10] = in[20];
        out[step*11] = in[21];

        out[step*12] = in[24];
        out[step*13] = in[25];

        out[step*14] = in[28];
        out[step*15] = in[29];

        out[step*16] = in[32];
        out[step*17] = in[33];
        
        out+=period;
        in+=36;
    }
    out[step*0] = in[0];
    out[step*1] = in[1];

    out[step*2] = in[4];
    out[step*3] = in[5];
}

void gather_2048(unsigned short *in, unsigned short *out, int wsize)
{
    int i;
    int step = 1<<wsize;
    int period = step*18;

    for(i=0; i<8; i++)
    {
        out[0]=in[step*0];
        out[1]=in[step*1];

        out[4]=in[step*2];
        out[5]=in[step*3];

        out[8]=in[step*4];
        out[9]=in[step*5];

        out[12]=in[step*6]; 
        out[13]=in[step*7]; 

        out[16]=in[step*8]; 
        out[17]=in[step*9]; 

        out[20]=in[step*10]; 
        out[21]=in[step*11]; 

        out[24]=in[step*12]; 
        out[25]=in[step*13]; 

        out[28]=in[step*14]; 
        out[29]=in[step*15]; 

        out[32]=in[step*16]; 
        out[33]=in[step*17];

        out+=36;
        in+=period;
    }
    out[0]=in[step*0];
    out[1]=in[step*1];

    out[4]=in[step*2];
    out[5]=in[step*3];
}

void norm2red2048(unsigned long out[80], unsigned long *in)
{
    int i, j;
    const unsigned long m = 0xfffffff;
    
    for(i=0, j=0; i<4*7; i+=7, j+=16)
    {
        out[j+0] = in[i+0] & m;
        out[j+1] = (in[i+0] >> 28) & m;
        out[j+2] = ((in[i+0] >> 56) ^ (in[i+1] << 8)) & m;

        out[j+3] = (in[i+1] >> 20) & m;
        out[j+4] = ((in[i+1] >> 48) ^ (in[i+2] << 16)) & m;

        out[j+5] = (in[i+2] >> 12) & m;   
        out[j+6] = ((in[i+2] >> 40) ^ (in[i+3] << 24)) & m;

        out[j+7] = (in[i+3] >> 4) & m;
        out[j+8] = (in[i+3] >> 32) & m;    
        out[j+9] = ((in[i+3] >> 60) ^ (in[i+4] << 4)) & m;
        
        out[j+10] = (in[i+4] >> 24) & m;
        out[j+11] = ((in[i+4] >> 52) ^ (in[i+5] << 12)) & m;
        
        out[j+12] = (in[i+5] >> 16) & m;
        out[j+13] = ((in[i+5] >> 44) ^ (in[i+6] << 20)) & m;
        
        out[j+14] = (in[i+6] >> 8) & m;
        out[j+15] = (in[i+6] >> 36) & m;
    }
    
    out[j+0] = in[i+0] & m;
    out[j+1] = (in[i+0] >> 28) & m;
    out[j+2] = ((in[i+0] >> 56) ^ (in[i+1] << 8)) & m;

    out[j+3] = (in[i+1] >> 20) & m;
    out[j+4] = ((in[i+1] >> 48) ^ (in[i+2] << 16)) & m;

    out[j+5] = (in[i+2] >> 12) & m;   
    out[j+6] = ((in[i+2] >> 40) ^ (in[i+3] << 24)) & m;

    out[j+7] = (in[i+3] >> 4) & m;
    out[j+8] = (in[i+3] >> 32) & m;    
    out[j+9] = (in[i+3] >> 60);
    
    out[74] = 0;
    out[75] = 0;
}

unsigned long LZCNT(unsigned long in)
{
    unsigned long res;
    asm("lzcnt %1, %0\n\t" : "=r"(res): "r"(in));
    
    return res;
}

unsigned long BZHI(unsigned long in, unsigned long idx)
{
    unsigned long res;
    asm("bzhi %2, %1, %0\n\t" : "=r"(res): "r"(in), "r"(idx));
    
    return res;
}

unsigned long one[74] = {1};
unsigned long conv2048[74] = {0,0,0,1<<12};
unsigned long conv1024[36] = {0,0,1<<22};

int RSAZ_redundant_mod_exponent(mp_digit result[16], mp_digit base_norm[16], mp_digit exponent[16], mp_digit m_norm[16], mp_digit RR[16], mp_digit k0, int exponent_bits, int mod_bits, int const_time)
{
    //void (*scatter)(unsigned short *, unsigned short *, int) = NULL;
    //void (*gather)(unsigned short *, unsigned short *, int) = NULL;
    //void (*norm2red)(unsigned long *, unsigned long *) = NULL;
    //void (*red2norm)(unsigned long *, unsigned long *) = NULL;
    //void (*AMM)(mp_digit*, mp_digit*, mp_digit*, mp_digit*, mp_digit k0) = NULL;
    //void (*AMS)(mp_digit*, mp_digit*, mp_digit*, mp_digit k0, int) = NULL;
    
    unsigned long *m = 0, *mod_mul_result = 0, *temp = 0, *R2 = 0, *a_tag = 0, *base = 0;
    unsigned long *conv_help = 0;
    
    int window_size;
    int operand_words = 0;
    
    unsigned short *table_s = NULL;
    unsigned char *space = NULL;
    
    int ret = 1;
    
    if((!const_time) && (exponent_bits <= 64))
    {
        window_size = 0;
    }
    else if(exponent_bits <= 20)
    {
        window_size = 1;
    }
    else if(exponent_bits <= 512)
    {
        window_size = 4;
    }
    else
    {
        window_size = 5;
    }
    
    //scatter = scatter_1024;
    //gather = gather_1024;
    //norm2red = norm2red1024;
    //red2norm = red2norm1024;
    //AMM = AMM_WW_1024;
    //AMS = AMS_WW_1024;
        
    operand_words = 36;
        
    space = (unsigned char *)memalign(4096, (1<<window_size) * operand_words * 4 + 36*8*4 + 40*8*5);
        
    if(NULL == space)
    {
        ret = 0;
        goto bail;
    }
        
    memset(space, 0, 36*8*4 + 40*8*5);
    m = (unsigned long *)&space[40*8*0];
    mod_mul_result = (unsigned long *)&space[36*8*4];
    temp = (unsigned long *)&space[36*8*4 + 40*8*1];
    a_tag = (unsigned long *)&space[36*8*4 + 40*8*2];
    R2 = (unsigned long *)&space[36*8*4 + 40*8*3];
    base = (unsigned long *)&space[36*8*4 + 40*8*4];
    table_s = (unsigned short *)&space[36*8*4 + 40*8*5];
        
    conv_help = conv1024;
        
    norm2red(m, m_norm);
    norm2red(base, base_norm);
    norm2red(R2, RR);

    // Create 4 "shifted" copies of m, to minimize split-loads.
    if(mod_bits == 1024)
    {
        memcpy(&m[36*1], &m[3], 8*36-3*8);
        memcpy(&m[36*2], &m[2], 8*36-2*8);
        memcpy(&m[36*3], &m[1], 8*36-1*8);
    }    
    else
    {
        memcpy(&m[76*1], &m[3], 8*76-3*8);
        memcpy(&m[76*2], &m[2], 8*76-2*8);
        memcpy(&m[76*3], &m[1], 8*76-1*8);
    }
    
    AMS(R2, R2, m, k0, 1);
    AMM(R2, R2, conv_help, m, k0);
    
	if (!window_size)
	{
		mp_digit exp = exponent[0];
		int pos = LZCNT(exp);

		AMM(a_tag, R2, base, m, k0);
		memcpy(mod_mul_result, a_tag, operand_words * 8);

		if (exp > 1)
		{
			exp = BZHI(exp, 63 - pos);
			do
			{
				if (exp)
				{
					AMS(mod_mul_result, mod_mul_result, m, k0, LZCNT(exp) - pos);
					AMM(mod_mul_result, mod_mul_result, a_tag, m, k0);
				}
				else
				{
					AMS(mod_mul_result, mod_mul_result, m, k0, 63 - pos);
				}

				pos = LZCNT(exp);
				exp = BZHI(exp, 63 - pos);

			} while (exp || pos < 63);
		}
		AMM(mod_mul_result, mod_mul_result, one, m, k0);
		red2norm1024(result, mod_mul_result);
	}
	else
	{
#define AMM_gather(res, a, b, m, k0) gather(b, (unsigned short*)temp, window_size); AMM(res, a, temp, m, k0);
		if (window_size == 1)
		{
			// table[0]
			AMM(mod_mul_result, R2, one, m, k0);
			scatter(&table_s[0], (unsigned short*)mod_mul_result, window_size);
			// table[1]
			AMM(a_tag, R2, base, m, k0);
			scatter(&table_s[1], (unsigned short*)a_tag, window_size);
		}
		else
		{
			// Prepare table
			// table[0]
			AMM(mod_mul_result, R2, one, m, k0);
			scatter(&table_s[0], (unsigned short*)mod_mul_result, window_size);
			// table[1]
#pragma GCC diagnostic ignored "-Wuninitialized"
			AMM(a_tag, R2, base, m, k0);
			scatter(&table_s[1], (unsigned short*)a_tag, window_size);
			// table[2]
			AMS(mod_mul_result, a_tag, m, k0, 1);
			scatter(&table_s[2], (unsigned short*)mod_mul_result, window_size);
			// table[4]
			AMS(mod_mul_result, mod_mul_result, m, k0, 1);
			scatter(&table_s[4], (unsigned short*)mod_mul_result, window_size);
			// table[8]
			AMS(mod_mul_result, mod_mul_result, m, k0, 1);
			scatter(&table_s[8], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[16]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[16], (unsigned short*)mod_mul_result, window_size);
			}
			// table[3]
			AMM_gather(mod_mul_result, a_tag, &table_s[2], m, k0);
			scatter(&table_s[3], (unsigned short*)mod_mul_result, window_size);
			// table[6]
			AMS(mod_mul_result, mod_mul_result, m, k0, 1);
			scatter(&table_s[6], (unsigned short*)mod_mul_result, window_size);
			// table[12]
			AMS(mod_mul_result, mod_mul_result, m, k0, 1);
			scatter(&table_s[12], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[24]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[24], (unsigned short*)mod_mul_result, window_size);
			}
			// table[5]
			AMM_gather(mod_mul_result, a_tag, &table_s[4], m, k0);
			scatter(&table_s[5], (unsigned short*)mod_mul_result, window_size);
			// table[10]
			AMS(mod_mul_result, mod_mul_result, m, k0, 1);
			scatter(&table_s[10], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[20]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[20], (unsigned short*)mod_mul_result, window_size);
			}
			// table[7]
			AMM_gather(mod_mul_result, a_tag, &table_s[6], m, k0);
			scatter(&table_s[7], (unsigned short*)mod_mul_result, window_size);
			// table[14]
			AMS(mod_mul_result, mod_mul_result, m, k0, 1);
			scatter(&table_s[14], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[28]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[28], (unsigned short*)mod_mul_result, window_size);
			}
			// table[9]
			AMM_gather(mod_mul_result, a_tag, &table_s[8], m, k0);
			scatter(&table_s[9], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[18]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[18], (unsigned short*)mod_mul_result, window_size);
			}
			// table[11]
			AMM_gather(mod_mul_result, a_tag, &table_s[10], m, k0);
			scatter(&table_s[11], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[22]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[22], (unsigned short*)mod_mul_result, window_size);
			}
			// table[13]
			AMM_gather(mod_mul_result, a_tag, &table_s[12], m, k0);
			scatter(&table_s[13], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[26]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[26], (unsigned short*)mod_mul_result, window_size);
			}
			// table[15]
			AMM_gather(mod_mul_result, a_tag, &table_s[14], m, k0);
			scatter(&table_s[15], (unsigned short*)mod_mul_result, window_size);
			if (window_size > 4)
			{
				// table[30]
				AMS(mod_mul_result, mod_mul_result, m, k0, 1);
				scatter(&table_s[30], (unsigned short*)mod_mul_result, window_size);
				// table[17]
				AMM_gather(mod_mul_result, a_tag, &table_s[16], m, k0);
				scatter(&table_s[17], (unsigned short*)mod_mul_result, window_size);
				// table[19]
				AMM_gather(mod_mul_result, a_tag, &table_s[18], m, k0);
				scatter(&table_s[19], (unsigned short*)mod_mul_result, window_size);
				// table[21]
				AMM_gather(mod_mul_result, a_tag, &table_s[20], m, k0);
				scatter(&table_s[21], (unsigned short*)mod_mul_result, window_size);
				// table[23]
				AMM_gather(mod_mul_result, a_tag, &table_s[22], m, k0);
				scatter(&table_s[23], (unsigned short*)mod_mul_result, window_size);
				// table[25]
				AMM_gather(mod_mul_result, a_tag, &table_s[24], m, k0);
				scatter(&table_s[25], (unsigned short*)mod_mul_result, window_size);
				// table[27]
				AMM_gather(mod_mul_result, a_tag, &table_s[26], m, k0);
				scatter(&table_s[27], (unsigned short*)mod_mul_result, window_size);
				// table[29]
				AMM_gather(mod_mul_result, a_tag, &table_s[28], m, k0);
				scatter(&table_s[29], (unsigned short*)mod_mul_result, window_size);
				// table[31]
				AMM_gather(mod_mul_result, a_tag, &table_s[30], m, k0);
				scatter(&table_s[31], (unsigned short*)mod_mul_result, window_size);
			}
		}
		// load first window
		unsigned char *p_str = (unsigned char*)exponent;
		int index = exponent_bits - window_size;
		int mask = (1 << window_size) - 1;
		int wvalue = *((unsigned short*)&p_str[index / 8]);
		wvalue = (wvalue >> (index % 8)) & mask;
		index -= window_size;

		gather(&table_s[wvalue], (unsigned short*)mod_mul_result, window_size);

		while (index >= 0)   // loop for the remaining windows
		{
			AMS(mod_mul_result, mod_mul_result, m, k0, window_size);

			wvalue = *((unsigned short*)&p_str[index / 8]);
			wvalue = (wvalue >> (index % 8)) & mask;
			index -= window_size;

			AMM_gather(mod_mul_result, mod_mul_result, &table_s[wvalue], m, k0);
		}
		if (index > -window_size) // The last window
		{
			int last_window_mask = (1 << (exponent_bits%window_size)) - 1;
			AMS(mod_mul_result, mod_mul_result, m, k0, window_size + index);
			wvalue = p_str[0] & last_window_mask;
			AMM_gather(mod_mul_result, mod_mul_result, &table_s[wvalue], m, k0);
		}
		AMM(mod_mul_result, mod_mul_result, one, m, k0);
		red2norm1024(result, mod_mul_result);
	}
    
    ret = 1;
bail:
    if(space)
    {   
        free(space);
    }
    return ret;
}

void freebl_cpuid(unsigned long op, unsigned long *eax,	unsigned long *ebx, unsigned long *ecx,	unsigned long *edx)
{
	/* 0 is passed to ecx, in order to allow avx2 detection, with op = 7 */
	__asm__("cpuid\n\t"
		: "=a"(*eax),
		"=b"(*ebx),
		"=c"(*ecx),
		"=d"(*edx)
		: "0"(op), "2"(0));
}
