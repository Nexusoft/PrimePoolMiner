#include "config.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>

/* 
	Adapted from @paulscreen's pool config code by HashToBeWild
*/


namespace pt = boost::property_tree;

namespace Core
{
	MinerConfig::MinerConfig()
	{
		strHost = "";
		nPort = "9549"; // 9325 for solo
		strNxsAddress = "2S4PPSznWfPVLtPJNpi8Ly46Wft3wGbayGkhaGzKLVcepmrhKTP";
		nMiningThreads = 0; // 0 = use all threads
		nTimeout = 10; 
		nBitArraySize = 37748736; // 1024*1024*36
		primeLimit = 71378571;
		nPrimeLimit = 4194304;
		nPrimorialEndPrime = 12;
	}

	void MinerConfig::PrintConfig()
	{
		printf("Configuration: \n");
		printf("-------------\n");
		printf("Host: %s \n", strHost.c_str());
		printf("Port: %s \n", nPort.c_str());
		printf("Address: %s \n", strNxsAddress.c_str());
		printf("Threads: %i \n", nMiningThreads);
		printf("Timeout: %i \n", nTimeout);
		printf("Bit Array Size: %u \n", nBitArraySize);
		printf("Prime Limit: %u \n", primeLimit);
		printf("N Prime Limit: %u}", nPrimeLimit);
		printf("Primorial End Prime: %u \n", nPrimorialEndPrime);
	}


	bool MinerConfig::ReadConfig()
	{
		printf("in read");
		bool bSuccess = true;
		try
		{
			printf("Reading config file miner.conf\n");
			std::ifstream lConfigFile("miner.conf");
			pt::ptree root;
			pt::read_json("miner.conf", root);
			
			// do not beak if a value is missing... 
			try
			{
				strHost = root.get<std::string>("host");
			}
			catch(...)
			{}
			try
			{
				nPort = root.get<std::string>("port");
			}
			catch(...)
			{}
			try
			{
				strNxsAddress = root.get<std::string>("nxs_address");
			}
			catch(...)
			{}
			try
			{
				nMiningThreads = root.get<int>("threads");
			}
			catch(...)
			{}
			try
			{
				nTimeout = root.get<int>("timeout");
			}
			catch(...)
			{}
			try
			{
				nBitArraySize = root.get<unsigned int>("bit_array_size");
			}
			catch(...)
			{}
			try
			{
				primeLimit = root.get<unsigned int>("prime_limit");
			}
			catch(...)
			{}
			try
			{
				nPrimeLimit = root.get<unsigned int>("n_prime_limit");
			}
			catch(...)
			{}
			try
			{
				nPrimorialEndPrime = root.get<unsigned int>("primorial_end_prime");
			}
			catch(...)
			{}
		}
		catch(...)
		{
			bSuccess = false;
		}

		return bSuccess;
	}


} // end namespace
