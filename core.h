#ifndef COINSHIELD_LLP_CORE_H
#define COINSHIELD_LLP_CORE_H

#include <stdlib.h>
#include "types.h"
#include <queue>
#include "config.h"
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>

namespace Core
{
	class CBlock
	{
	public:

		/** Begin of Header.   BEGIN(nVersion) **/
		unsigned int  nVersion;
		uint1024 hashPrevBlock;
		uint512 hashMerkleRoot;
		unsigned int  nChannel;
		unsigned int   nHeight;
		unsigned int     nBits;
		uint64          nNonce;
		/** End of Header.     END(nNonce).
		All the components to build an SK1024 Block Hash. **/

		CBlock()
		{
			nVersion = 0;
			hashPrevBlock = 0;
			hashMerkleRoot = 0;
			nChannel = 0;
			nHeight = 0;
			nBits = 0;
			nNonce = 0;
		}

		uint1024 GetHash() const
		{
			return SK1024(BEGIN(nVersion), END(nBits));
		}

		CBigNum GetPrime() const
		{
			return CBigNum(GetHash() + nNonce);
		}

		void Set(CBlock block)
		{
			nVersion = block.nVersion;
			hashPrevBlock = block.hashPrevBlock;
			hashMerkleRoot = block.hashMerkleRoot;
			nChannel = block.nChannel;
			nHeight = block.nHeight;
			nBits = block.nBits;
			nNonce = block.nNonce;
		}

		void Set(CBlock * block)
		{
			nVersion = block->nVersion;
			hashPrevBlock = block->hashPrevBlock;
			hashMerkleRoot = block->hashMerkleRoot;
			nChannel = block->nChannel;
			nHeight = block->nHeight;
			nBits = block->nBits;
			nNonce = block->nNonce;
		}

	};
}

namespace LLP
{
	class Outbound : public Connection
	{
		Service_t IO_SERVICE;
		std::string IP, PORT;
		
	public:
		Packet ReadNextPacket(int nTimeout = 10)
		{
			Packet NULL_PACKET;
			while(!PacketComplete())
			{
				if(Timeout(nTimeout) || Errors())
					return NULL_PACKET;
				
				ReadPacket();
			
				Sleep(1);
			}
			
			return this->INCOMING;
		}
		
	public:
		/** Outgoing Client Connection Constructor **/
		Outbound(std::string ip, std::string port) : IP(ip), PORT(port), Connection() { }
		
		bool Connect()
		{
			try
			{
				using boost::asio::ip::tcp;
				
				tcp::resolver 			  RESOLVER(IO_SERVICE);
				tcp::resolver::query      QUERY   (tcp::v4(), IP.c_str(), PORT.c_str());
				tcp::resolver::iterator   ADDRESS = RESOLVER.resolve(QUERY);
				
				this->SOCKET = Socket_t(new tcp::socket(IO_SERVICE));
				this->SOCKET -> connect(*ADDRESS, this->ERROR_HANDLE);
				
				if(Errors())
				{
					this->Disconnect();
					
					printf("Failed to Connect to Mining LLP Server...\n");
					return false;
				}
				
				this->CONNECTED = true;
				this->TIMER.Start();
				
				printf("Connected to %s:%s...\n", IP.c_str(), PORT.c_str());

				return true;
			}
			catch(...){ }
			
			this->CONNECTED = false;
			return false;
		}
		
	};
	
	
	class Miner : public Outbound
	{
	private:
		bool DeserializeSoloBlock(std::vector<unsigned char> DATA, Core::CBlock* BLOCK)
		{
			try
			{
				BLOCK->nVersion = bytes2uint(std::vector<unsigned char>(DATA.begin(), DATA.begin() + 4));

				BLOCK->hashPrevBlock.SetBytes(std::vector<unsigned char>(DATA.begin() + 4, DATA.begin() + 132));
				BLOCK->hashMerkleRoot.SetBytes(std::vector<unsigned char>(DATA.begin() + 132, DATA.end() - 20));

				BLOCK->nChannel = bytes2uint(std::vector<unsigned char>(DATA.end() - 20, DATA.end() - 16));
				BLOCK->nHeight = bytes2uint(std::vector<unsigned char>(DATA.end() - 16, DATA.end() - 12));
				BLOCK->nBits = bytes2uint(std::vector<unsigned char>(DATA.end() - 12, DATA.end() - 8));
				BLOCK->nNonce = bytes2uint64(std::vector<unsigned char>(DATA.end() - 8, DATA.end()));
			}
			catch (const std::exception&)
			{
				return false;
			}
			
			return true;
		}

		Core::CBlock* DeserializeSoloBlock(std::vector<unsigned char> DATA)
		{
			Core::CBlock* BLOCK = new Core::CBlock();
			if (DeserializeSoloBlock(DATA, BLOCK))
				return BLOCK;
			else
				return NULL;
		}

		int nTimeout;

	public:
		Miner(std::string ip, std::string port, int timeout = 10) : Outbound(ip, port) { nTimeout = timeout; }
		
		enum
		{
			/** DATA PACKETS **/
			LOGIN            = 0,
			BLOCK_DATA       = 1,
			SUBMIT_SHARE     = 2,
			ACCOUNT_BALANCE  = 3,
			PENDING_PAYOUT   = 4,
			SUBMIT_PPS     	= 5,
			
					
			/** REQUEST PACKETS **/
			GET_BLOCK    = 129,
			NEW_BLOCK    = 130,
			GET_BALANCE  = 131,
			GET_PAYOUT   = 132,
			
			
			/** RESPONSE PACKETS **/
			ACCEPT     = 200,
			REJECT     = 201,
			BLOCK      = 202,
			STALE      = 203,
			
					
			/** GENERIC **/
			PING     = 253,
			CLOSE    = 254
		};
		
		enum
		{
			/** DATA PACKETS **/
			SOLO_BLOCK_DATA = 0,
			SUBMIT_BLOCK = 1,
			BLOCK_HEIGHT = 2,
			SET_CHANNEL = 3,

			/** REQUEST PACKETS **/
			SOLO_GET_HEIGHT = 130,

			/** DATA REQUESTS **/
			CHECK_BLOCK = 64,
			SUBSCRIBE = 65,

			/** ROUND VALIDATIONS. **/
			NEW_ROUND = 204,
			OLD_ROUND = 205
		};

		
		/** Current Newly Read Packet Access. **/
		inline Packet NewPacket() { return this->INCOMING; }
		
		
		/** Create a Packet with the Given Header. **/
		inline Packet GetPacket(unsigned char HEADER, unsigned int nLength = 0)
		{
			Packet PACKET;
			PACKET.HEADER = HEADER;
			PACKET.LENGTH = nLength;
			return PACKET;
		}

		
		/** Get a new Block from the Pool Server. **/
		inline void GetBlock()    { this -> WritePacket(GetPacket(GET_BLOCK));    }
		
		
		/** Get your current balance in NXS that has not been included in a payout. **/
		inline void GetBalance()  { this -> WritePacket(GetPacket(GET_BALANCE));  }
				
		/** Get the Current Pending Payouts for the Next Coinbase Tx. **/
		inline void GetPayouts()  { this -> WritePacket(GetPacket(GET_PAYOUT)); }
		
		/** Ping the Pool Server to let it know Connection is Still Alive. **/
		inline void Ping() { this -> WritePacket(GetPacket(PING)); }
		
		/** Send current PPS / WPS data to the pool **/
		inline void SubmitPPS(double PPS, double WPS) 
		{ 
			Packet PACKET = GetPacket(SUBMIT_PPS);
			std::vector<unsigned char> vPPSBytes = double2bytes(PPS);
			std::vector<unsigned char> vWPSBytes = double2bytes(WPS);
			// add PPS
			PACKET.DATA.insert(PACKET.DATA.end(),vPPSBytes.begin(), vPPSBytes.end()  );
			
			// add WPS
			PACKET.DATA.insert(PACKET.DATA.end(),vWPSBytes.begin(), vWPSBytes.end()  );

			PACKET.LENGTH = PACKET.DATA.size();

			this->WritePacket(PACKET);
			
		}
		
		
		/** Send your address for Pool Login. **/
		inline void Login(std::string ADDRESS)
		{
			Packet PACKET = GetPacket(LOGIN);
			PACKET.DATA   = string2bytes(ADDRESS);
			PACKET.LENGTH = PACKET.DATA.size();
			
			printf("[MASTER] Logged in With Address: %s Bytes: %u\n", ADDRESS.c_str(), PACKET.LENGTH);
			this->WritePacket(PACKET);
		}
			
		/** Submit a Share to the Pool Server. **/
		inline void SubmitShare(uint1024 nPrimeOrigin, uint64 nNonce)
		{
			Packet PACKET = GetPacket(SUBMIT_SHARE);
			PACKET.DATA = nPrimeOrigin.GetBytes();
			std::vector<unsigned char> NONCE  = uint2bytes64(nNonce);
			
			PACKET.DATA.insert(PACKET.DATA.end(), NONCE.begin(), NONCE.end());
			PACKET.LENGTH = 136;
			
			this->WritePacket(PACKET);
		}

		// Solo Mining related
		void SetChannel(unsigned int nChannel)
		{
			Packet packet = GetPacket(SET_CHANNEL, 4);
			packet.DATA = uint2bytes(nChannel);
			this->WritePacket(packet);
		}
		
		void Subscribe(unsigned int nOfBlocks)
		{
			Packet packet = GetPacket(SUBSCRIBE, 4);
			packet.DATA = uint2bytes(nOfBlocks);
			this->WritePacket(packet);
		}

		Core::CBlock* GetSoloBlock()
		{
			this->WritePacket(GetPacket(GET_BLOCK));

			Packet RESPONSE = ReadNextPacket(nTimeout);

			if (RESPONSE.IsNull() || RESPONSE.DATA.size() == 0)
				return NULL;

			Core::CBlock* BLOCK = DeserializeSoloBlock(RESPONSE.DATA);
			ResetPacket();

			return BLOCK;
		}

		bool GetSoloBlock(Core::CBlock* block)
		{
			bool res = true;
			this->WritePacket(GetPacket(GET_BLOCK));

			Packet RESPONSE = ReadNextPacket(nTimeout);

			if (RESPONSE.IsNull() || RESPONSE.DATA.size() == 0)
				return false;

			res = DeserializeSoloBlock(RESPONSE.DATA, block);
			ResetPacket();

			return res;
		}
		
		
		unsigned int GetHeight()
		{
			this->WritePacket(GetPacket(SOLO_GET_HEIGHT));

			Packet RESPONSE = ReadNextPacket(nTimeout);

			if (RESPONSE.IsNull() || RESPONSE.LENGTH == 0)
				return 0;

			unsigned int nHeight = bytes2uint(RESPONSE.DATA);
			ResetPacket();

			return nHeight;
		}


		unsigned char SubmitBlock(uint512 hashMerkleRoot, uint64 nNonce)
		{
			Packet PACKET;
			PACKET.HEADER = SUBMIT_BLOCK;

			PACKET.DATA = hashMerkleRoot.GetBytes();
			std::vector<unsigned char> NONCE = uint2bytes64(nNonce);

			PACKET.DATA.insert(PACKET.DATA.end(), NONCE.begin(), NONCE.end());
			PACKET.LENGTH = 72;

			this->WritePacket(PACKET);
			Packet RESPONSE = ReadNextPacket(nTimeout);
			if (RESPONSE.IsNull())
				return 0;

			ResetPacket();

			return RESPONSE.HEADER;
		}
	};
	
}

#define MAXCANDIDATESPERSIEVE 1000
#define MAX_PRIME_TEST_JOBQUEUE_SIZE 1000
#define MAX_SIEVE_JOBQUEUE_SIZE 256

extern volatile uint64 sieveCandidateCount;
extern bool bUseExperimentalSieve;

namespace Core
{
	class ServerConnection;
	class MinerConfig;

	typedef struct
	{
		CBigNum	*	baseHash;
		uint512	*	hashMerkleRoot;
		unsigned long  * candidates;
		mpz_ptr		zFirstSieveElement;
		mpz_ptr		zPrimeOrigin;
	}primeTestJob;

	typedef struct
	{
		CBigNum	* baseHash;
		uint512	* hashMerkleRoot;
		uint64 nNonce;
		unsigned int nDiff;
	}submitBlockData;

	typedef struct
	{
		CBlock 	pBlock;
		uint1024 	primeOrigin;
		uint16_t 	nStartOrigin;
		uint16_t 	nMaxOriginCount;
		uint32_t 	nMinimumShare;
		uint32_t	nHeight;
	}sieveJob;


	extern unsigned int *primes;
	extern unsigned int *inverses;
	extern unsigned int nBitArray_Size;
	extern mpz_t  zPrimorial;

	extern unsigned int prime_limit;
	extern unsigned int nPrimeLimit;
	extern unsigned int nPrimorialEndPrime;
	boost::lockfree::queue<int> pcJobQueueActive(MAX_PRIME_TEST_JOBQUEUE_SIZE);
	boost::lockfree::queue<int> pcJobQueuePassive(MAX_PRIME_TEST_JOBQUEUE_SIZE);
		
	boost::lockfree::queue<submitBlockData> submitBlockQueue(10);

	extern uint64 octuplet_origins[];
	extern uint64 tentuplet2_origins[];

	void InitializePrimes();
	unsigned int SetBits(double nDiff);
	double GetPrimeDifficulty(CBigNum prime, int checks);
	double GetSieveDifficulty(CBigNum next, unsigned int clusterSize);
	unsigned int GetPrimeBits(CBigNum prime, int checks);
	unsigned int GetFractionalDifficulty(CBigNum composite);
	std::vector<unsigned int> Eratosthenes(int nSieveSize);
	bool DivisorCheck(CBigNum test);
	unsigned long PrimeSieve(CBigNum BaseHash, unsigned int nDifficulty, unsigned int nHeight);
	bool PrimeCheck(CBigNum test, int checks);
	CBigNum FermatTest(CBigNum n, CBigNum a);
	bool Miller_Rabin(CBigNum n, int checks);
	
	void cpusieve(uint64_t * sieve, unsigned int sieveSize, mpz_t zPrimorial, mpz_t zPrimeOrigin, unsigned long long ktuple_origin, unsigned int * primes, unsigned int * inverses, unsigned int nPrimorialEndPrime, unsigned int nPrimeLimit, mpz_t * zFirstSieveElement, unsigned long * candidates);


	/** Class to hold the basic data a Miner will use to build a Block.
		Used to allow one Connection for any amount of threads. **/
	class MinerThread
	{
	public:
		ServerConnection* cServerConnection;
		
		uint1024 hashPrimeOrigin;
		uint512 hashMerkleRoot;
		unsigned int nMinimumShare;
		volatile bool fNewBlock, fBlockWaiting, fNewBlockRestart;
		LLP::Thread_t THREAD;
		LLP::Timer IDLE_TIME;
		boost::mutex MUTEX;
		uint64_t* bit_array_sieve;
		


		
		MinerThread(ServerConnection* cConnection) : cServerConnection(cConnection), fNewBlock(true), fBlockWaiting(false), fNewBlockRestart(true), THREAD(boost::bind(&MinerThread::PrimeMiner, this)) 
		{ 
			bit_array_sieve = (uint64_t *)aligned_alloc(64, (nBitArray_Size) / 8);			
		}

		~MinerThread()
		{
			free(bit_array_sieve);
		}

		void PrimeMiner();
	};
	
		/** Class to handle all the Connections via Mining LLP.
		Independent of Mining Threads for Higher Efficiency. **/
	class ServerConnection
	{
	public:
		LLP::Miner* CLIENT;
		int nPTestThreads , nSieveThreads, nTimeout;
		bool bSoloMining;
		std::vector<MinerThread*> THREADS;
		
		std::map<int, primeTestJob> primeTestJobs;
		std::map<int, sieveJob *> sieveJobs;
		uint16_t nSieveJobsQueueSize;

		std::vector<LLP::Thread_t*> PRIMETESTTHREADS;

		LLP::Thread_t THREAD;
		LLP::Thread_t* STATUSTHREAD;
		LLP::Timer    TIMER;
		std::string   IP, PORT;
		
		boost::mutex    SUBMIT_MUTEX;

		std::queue<std::pair<uint1024, uint64> > SUBMIT_QUEUE;
		std::queue<std::pair<uint1024, uint64> > RESPONSE_QUEUE;

		boost::lockfree::queue<int> * sieveJobQueueActive;
		boost::lockfree::queue<int> * sieveJobQueuePassive;

		unsigned int nMinimumShare;
		
		ServerConnection(std::string ip, std::string port, int nMaxSThreads, int nMaxPTThreads, int nMaxTimeout, bool bSolo);

		~ServerConnection()
		{
			if (STATUSTHREAD != NULL)
				delete STATUSTHREAD;
			for (int nIndex = 0; nIndex < nPTestThreads; nIndex++)
			{
				if (PRIMETESTTHREADS[nIndex] != NULL)
					delete PRIMETESTTHREADS[nIndex];
			}
			delete sieveJobQueueActive;
			delete sieveJobQueuePassive;

		}

		
		/*** Reset the block on each of the Threads. ***/
		void ResetThreads();
		void SubmitShare(uint1024 hashPrimeOrigin, uint64 nNonce);
		void ServerThread();
		void PrintStats();
		void SoloServerThread();
		void PrintStatThread();

		void PrimeTestThread();
	};
}



#endif
