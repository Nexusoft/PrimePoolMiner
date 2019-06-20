#include "util.h"

/** Find out how many cores the CPU has. **/
int GetTotalCores()
{
	int nProcessors = boost::thread::hardware_concurrency();
	if (nProcessors < 1)
		nProcessors = 1;
			
	return nProcessors;
}


uint64_t mpz2uint64(mpz_t n)
{
    unsigned int lo, hi;
    mpz_t tmp;

    mpz_init( tmp );
    mpz_mod_2exp( tmp, n, 64 );

    lo = mpz_get_ui( tmp );
    mpz_div_2exp( tmp, tmp, 32 );
    hi = mpz_get_ui( tmp );

    mpz_clear( tmp );

    return (((uint64_t)hi) << 32) + lo;
}

/*int bignum2mpz(const BIGNUM *bn, mpz_t *g)
{
	//bn_check_top(bn);
	if((BN_num_bits(bn) == GMP_NUMB_BITS) && (BN_BITS2 == GMP_NUMB_BITS)) 
	{
        size_t len;
        void *p;
        
        mpz_init (*g);
        
        len = BN_num_bytes(bn);
        p = malloc(len);
        BN_bn2bin(bn, p);
        mpz_set_str(g, p, len);
        //mp_int_read_unsigned(g, p, len);
        free(p);
        
        return 1;
    }
	else
	{
		char *tmpchar = BN_bn2hex(bn);
		
		if(!tmpchar)
			return 0;
		
		OPENSSL_free(tmpchar);
		
		return 0;
	}
}
*/

void bignum2mpz(const BIGNUM *bn, mpz_t &g)
{
        uint8_t p[128] = {0};
     
        mpz_init(g);
        BN_bn2bin(bn, p);
        mpz_import(g, BN_num_bytes(bn), -1, sizeof(uint8_t), 0, 0, &p);
}

void SetCurrentThreadPriority(int priority)
{
#ifdef __CYGWIN__
	pthread_t thId = pthread_self();
	pthread_attr_t thAttr;
	int policy = 0;
	int max_prio_for_policy = 0;
	int min_prio_for_policy = 0;

	pthread_attr_init(&thAttr);
	pthread_attr_getschedpolicy(&thAttr, &policy);
	max_prio_for_policy = sched_get_priority_max(policy);
	min_prio_for_policy = sched_get_priority_min(policy);
	if (priority = INT32_MAX)
		priority = max_prio_for_policy;
	else if (priority = INT32_MIN)
		priority = min_prio_for_policy;

	printf("Setting thread proirity to  %u\r\n", priority);
	//printf("Max proirity: %u\r\n", max_prio_for_policy);
	//printf("Max proirity: %u\r\n", min_prio_for_policy);
	if (pthread_setschedprio(thId, priority) != 0)
		printf("Failed to set main thread prioroty");
	pthread_attr_destroy(&thAttr);
#endif
}

void SetThreadPriority(pthread_t threadID, int priority)
{

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
	//TODO: implment set thread priority in VS
#else
	int retcode;
	int policy;
	struct sched_param param;

	if ((retcode = pthread_getschedparam(threadID, &policy, &param)) != 0)
	{
		errno = retcode;
		perror("pthread_getschedparam");
		exit(EXIT_FAILURE);
	}

	policy = SCHED_FIFO;
	param.sched_priority = priority;

	if ((retcode = pthread_setschedparam(threadID, policy, &param)) != 0)
	{
		errno = retcode;
		perror("pthread_setschedparam");
		printf("\n!!! FAILED to set thread priority. For the best performance try executing with 'sudo ./nexus_cpuminer' !!!\n\n");
		//exit(EXIT_FAILURE);
		return;
	}

	std::cout << "Thred ("<< threadID << ") priority CHANGED: ";
	std::cout << "policy=" << ((policy == SCHED_FIFO) ? "SCHED_FIFO" :
		(policy == SCHED_RR) ? "SCHED_RR" :
		(policy == SCHED_OTHER) ? "SCHED_OTHER" :
		"???")
		<< ", priority=" << param.sched_priority << std::endl;
#endif
}
