#ifndef COINSHIELD_LLP_CORE_H
#define COINSHIELD_LLP_CORE_H

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
		Core::CBlock* DeserializeSoloBlock(std::vector<unsigned char> DATA)
		{
			Core::CBlock* BLOCK = new Core::CBlock();
			BLOCK->nVersion = bytes2uint(std::vector<unsigned char>(DATA.begin(), DATA.begin() + 4));

			BLOCK->hashPrevBlock.SetBytes(std::vector<unsigned char>(DATA.begin() + 4, DATA.begin() + 132));
			BLOCK->hashMerkleRoot.SetBytes(std::vector<unsigned char>(DATA.begin() + 132, DATA.end() - 20));

			BLOCK->nChannel = bytes2uint(std::vector<unsigned char>(DATA.end() - 20, DATA.end() - 16));
			BLOCK->nHeight = bytes2uint(std::vector<unsigned char>(DATA.end() - 16, DATA.end() - 12));
			BLOCK->nBits = bytes2uint(std::vector<unsigned char>(DATA.end() - 12, DATA.end() - 8));
			BLOCK->nNonce = bytes2uint64(std::vector<unsigned char>(DATA.end() - 8, DATA.end()));

			return BLOCK;
		}


	public:
		Miner(std::string ip, std::string port) : Outbound(ip, port){}
		
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

			Packet RESPONSE = ReadNextPacket(); //TODO: set the timeout

			if (RESPONSE.IsNull() || RESPONSE.DATA.size() == 0)
				return NULL;

			Core::CBlock* BLOCK = DeserializeSoloBlock(RESPONSE.DATA);
			ResetPacket();

			return BLOCK;
		}
	};
	
}

#define MAXCANDIDATESPERSIEVE 1000

namespace Core
{
	class ServerConnection;
	class MinerConfig;
	extern unsigned int *primes;
	extern unsigned int *inverses;
	extern unsigned int nBitArray_Size;
	extern mpz_t  zPrimorial;

	extern unsigned int prime_limit;
	extern unsigned int nPrimeLimit;
	extern unsigned int nPrimorialEndPrime;
	boost::lockfree::queue<int> pcJobQueueActive(1000);
	boost::lockfree::queue<int> pcJobQueuePassive(1000);

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
	
	typedef struct
	{
		CBigNum	*	baseHash;
		//unsigned long  candidates[MAXCANDIDATESPERSIEVE];
		unsigned long  * candidates;
		mpz_ptr		zFirstSieveElement;
		mpz_ptr		zPrimeOrigin;
	}primeTestJob;


	/** Class to hold the basic data a Miner will use to build a Block.
		Used to allow one Connection for any amount of threads. **/
	class MinerThread
	{
	public:
		ServerConnection* cServerConnection;
		
		uint1024 hashPrimeOrigin;
		unsigned int nMinimumShare;
		bool fNewBlock, fBlockWaiting, fNewBlockRestart;
		LLP::Thread_t THREAD;
		LLP::Timer IDLE_TIME;
		boost::mutex MUTEX;
		
		MinerThread(ServerConnection* cConnection) : cServerConnection(cConnection), fNewBlock(true), fBlockWaiting(false), fNewBlockRestart(true), THREAD(boost::bind(&MinerThread::PrimeMiner, this)) { }

		void PrimeMiner();
	};
	
		/** Class to handle all the Connections via Mining LLP.
		Independent of Mining Threads for Higher Efficiency. **/
	class ServerConnection
	{
	public:
		LLP::Miner* CLIENT;
		int nPTestThreads , nSieveThreads, nTimeout;
		std::vector<MinerThread*> THREADS;
		
		std::map<int, primeTestJob> primeTestJobs;

		std::vector<LLP::Thread_t*> PRIMETESTTHREADS;

		LLP::Thread_t THREAD;
		LLP::Timer    TIMER;
		std::string   IP, PORT;
		
		boost::mutex    SUBMIT_MUTEX;

		std::queue<std::pair<uint1024, uint64> > SUBMIT_QUEUE;
		std::queue<std::pair<uint1024, uint64> > RESPONSE_QUEUE;

		unsigned int nMinimumShare;
		
		ServerConnection(std::string ip, std::string port, int nMaxSThreads, int nMaxPTThreads, int nMaxTimeout) : IP(ip), PORT(port), TIMER(), nSieveThreads(nMaxSThreads), nPTestThreads(nMaxPTThreads), nTimeout(nMaxTimeout), THREAD(boost::bind(&ServerConnection::ServerThread, this))
		{
			for (size_t i = 0; i < 1000; i++)
			{
				primeTestJob job; 
				job.baseHash = new CBigNum(0);
				job.candidates = (unsigned long *)malloc(sizeof(unsigned long) * MAXCANDIDATESPERSIEVE);
				job.zFirstSieveElement = (mpz_ptr)malloc(sizeof(__mpz_struct));
				job.zPrimeOrigin = (mpz_ptr)malloc(sizeof(__mpz_struct));
				mpz_init(job.zFirstSieveElement);
				mpz_init(job.zPrimeOrigin);
				primeTestJobs.insert(std::pair<int, primeTestJob>(i, job));
				Core::pcJobQueuePassive.push(i);
			}

			for (int nIndex = 0; nIndex < nSieveThreads; nIndex++)
				THREADS.push_back(new MinerThread(this));

			for (int nIndex = 0; nIndex < nPTestThreads; nIndex++)
			{
				boost::thread * checkThread = new boost::thread(boost::bind(&ServerConnection::PrimeTestThread, this));
				PRIMETESTTHREADS.push_back(checkThread);				
			}
		}
		
		/*** Reset the block on each of the Threads. ***/
		void ResetThreads();
		void SubmitShare(uint1024 hashPrimeOrigin, uint64 nNonce);
		void ServerThread();

		void PrimeTestThread();
	};
}



#endif
