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
		strHost = "127.0.0.1";
		nPort = "9549"; // 9325 for solo
		strNxsAddress = "2RjohSoM34UazsV8iHxaFrJ5f6eeeohLWE83QkLYbHKNnBz1cND";
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
		printf("Host: {%s} \n", strHost);
		printf("Port: {%s}", nPort);
		printf("Address: {%s} \n", strNxsAddress);
		printf("Threads: {%n}", nMiningThreads);
		printf("Timeout: {%n}", nTimeout);
		printf("Bit Array Size: {%n}", nBitArraySize);
		printf("Prime Limit: {%n}", primeLimit);
		printf("N Prime Limit: {%n}", nPrimeLimit);
		printf("Primorial End Prime: {%n}", nPrimorialEndPrime);
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
