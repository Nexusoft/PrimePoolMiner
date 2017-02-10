#ifndef NEXUS_MINER_CONFIG_H
#define NEXUS_MINER_CONFIG_H
#include <string>

/* 
	Adapted from @paulscreen's pool config code by HashToBeWild
*/

namespace Core
{
/* 
	Class to get configuration from a config file
	Adapted from @paulscreen's pool config code by @hashtobewild
*/

    class MinerConfig
    {
    public:
        MinerConfig();
        bool ReadConfig();
	    void PrintConfig();

	// Standard config
        std::string  strHost;
        std::string  nPort;
        std::string  strNxsAddress;
        int          nMiningThreads;
        int 	     nTimeout;

	// Detailed hash related config
        unsigned int nBitArraySize;
        unsigned int primeLimit;
        unsigned int nPrimeLimit;
        unsigned int nPrimorialEndPrime;	



    };

}


#endif
