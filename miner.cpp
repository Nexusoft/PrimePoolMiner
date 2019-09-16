#include "core.h"
#include "config.h"
#include <deque>
#include <numeric>

#include "oacc/AccSieve.h"

#pragma GCC optimize ("unroll-loops")


#ifdef __CYGWIN__
#define SLASH "\\"
#else
#define SLASH "/"
#endif

volatile unsigned int nBestHeight = 0;
unsigned int nStartTimer = 0;
volatile uint64 nWeight = 0;
volatile uint64 nElements = 0;
volatile uint64 nPrimes = 0;
double nAverage = 0;
volatile unsigned int nLargestShare = 0;
unsigned int nDifficulty = 0;
volatile uint64 sieveTime = 0;
volatile uint64 pTestTime = 0;
volatile uint64 sieveCount = 0;
volatile uint64 testCount = 0;
volatile uint64 shareCount = 0;
volatile uint64 candidateCount = 0;
volatile uint64 sieveCandidateCount = 0;
volatile uint64 candidateHitCount = 0;
volatile uint64 candidateHit2Count = 0;
volatile uint64 totalShareWeight = 0;
volatile uint32_t chainCounter[14] = { 0 };
volatile bool	bBlockSubmission = false;
volatile uint32_t nBlocksFoundCounter = 0;
volatile uint32_t nBlocksAccepted = 0;
volatile uint32_t nBlocksRejected = 0;
bool bSoloMining = false;
volatile bool exitSignal = false;
volatile bool fResetThreads = false;
std::string ADDRESS;
uint64 nCurrentPayout = 0, nAccountBalance = 0;
bool bUseExperimentalSieve;

static LLP::Timer cPrimeTimer;
std::deque<double> vPPSValues;
std::deque<double> vWPSValues;

namespace Core
{
	/** Main Miner Thread. Bound to the class with boost. Might take some rearranging to get working with OpenCL. **/
	void MinerThread::PrimeMiner()
	{
		//unsigned long candidates[MAXCANDIDATESPERSIEVE];
		//printf("Starting Prime Miner thread\n");
		loop
		{
			try
			{
				/** Keep thread at idle CPU usage if waiting to submit or recieve block. **/
				Sleep(1);
				if (exitSignal)
					break;

				/** Assure that this thread stays idle when waiting for new block, or share submission. **/
				//if (fNewBlock || fBlockWaiting || !fNewBlockRestart)
				//	continue;
				if (bBlockSubmission)
					continue;
				

				/** Lock the Thread at this Mutex when Changing Block Pointer. **/
				//CBigNum BaseHash(hashPrimeOrigin);
				mpz_t zPrimeOrigin, zPrimeOriginOffset, zFirstSieveElement, zPrimorialMod, zTempVar, zResidue, zTwo, zN, zOctuplet;

				unsigned int i = 0;
				unsigned int j = 0;
				unsigned int nPrimeCount = 0;
				//unsigned int nSieveDifficulty = 0;
				uint64 nStart = 0;
				uint64 nStop = 0;
				unsigned int nLastOffset = 0;

				#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
					uint64 nNonce = 0;
				#else
					unsigned long nNonce = 0;
				#endif

				//long nElapsedTime = 0;
				//long nStartTime = 0;
				mpz_init(zPrimeOriginOffset);
				mpz_init(zFirstSieveElement);
				mpz_init(zPrimorialMod);
				mpz_init(zOctuplet);
				mpz_init(zTempVar);
				mpz_init(zPrimeOrigin);
				mpz_init(zResidue);
				mpz_init_set_ui(zTwo, 2);
				mpz_init(zN);


				int sieveJobId = -1;
				while (!cServerConnection->sieveJobQueueActive->pop(sieveJobId))
				{
					//printf("Waiting for job from the server!!!!!!\n");
					Sleep(20);
				}
				sieveJob * sJob = cServerConnection->sieveJobs.at(sieveJobId);
				if (sJob->nHeight < nBestHeight || fNewBlock)
				{
					cServerConnection->sieveJobQueuePassive->push(sieveJobId);
					continue;
				}

				/* Reset the fNewBlockRestart flag since we are on a new iteration */
				{
					LOCK(MUTEX);
					fNewBlockRestart = false;
				}

				CBigNum BaseHash;
				if (sJob->pBlock.nHeight != 0) //solo
				{
					BaseHash.setuint1024( sJob->pBlock.GetHash());
					hashMerkleRoot = sJob->pBlock.hashMerkleRoot;
				}
				else if (sJob->primeOrigin != 0) // pool
					BaseHash.setuint1024(sJob->primeOrigin);
				else
				{
					printf("Invalid Block Hash !!!\n");
					cServerConnection->sieveJobQueuePassive->push(sieveJobId);
					Sleep(100);
					continue;
				}
				int nCurrentJobBlockHeight = sJob->nHeight;
				nMinimumShare = sJob->nMinimumShare;
				int startOrigin = sJob->nStartOrigin;
				int stopOrigin = sJob->nStartOrigin + sJob->nMaxOriginCount;				
				cServerConnection->sieveJobQueuePassive->push(sieveJobId);
				bignum2mpz(&BaseHash, zPrimeOrigin);

				for (j = startOrigin; j < stopOrigin; j++)
				{
					//if (fNewBlock || fBlockWaiting || !fNewBlockRestart)
					//	break;
					if (fNewBlock || bBlockSubmission || fNewBlockRestart)
						break;

					if (nCurrentJobBlockHeight < nBestHeight)
						break;

					int primeTestJobId = 0;

					while (!cServerConnection->pcJobQueuePassive->pop(primeTestJobId))
					{
						printf("Test queue is full!!! Consider rising Prime Limit to produce less candidates per sieve or add more prime checking threads.\n");
						Sleep(300);
					}
					primeTestJob ptJob = cServerConnection->primeTestJobs.at(primeTestJobId);
					ptJob.baseHash->setuint1024(BaseHash.getuint1024());
					*ptJob.hashMerkleRoot = hashMerkleRoot;
					int64 nStartTime = GetTimeMicros();
					uint64 tupleOrigin = tentuplet2_origins[j];
					if (bUseExperimentalSieve)
						cpusieve(bit_array_sieve, nBitArray_Size, zPrimorial, zPrimeOrigin, tupleOrigin, primes, inverses, nPrimorialEndPrime, nPrimeLimit, &zFirstSieveElement, ptJob.candidates);
					else
						pgisieve(bit_array_sieve, nBitArray_Size, zPrimorial, zPrimeOrigin, tupleOrigin, primes, inverses, nPrimorialEndPrime, nPrimeLimit, &zFirstSieveElement, ptJob.candidates);
					
					
					int64 nAfterSieve = GetTimeMicros();

					mpz_set(ptJob.zFirstSieveElement, zFirstSieveElement);
					mpz_set(ptJob.zPrimeOrigin, zPrimeOrigin);

					if(!fNewBlockRestart)
						cServerConnection->pcJobQueueActive->push(primeTestJobId);

					int64 nElapsedTime = nAfterSieve - nStartTime;
					sieveTime += nElapsedTime;
					sieveCount++;
				}

				mpz_clear(zPrimeOrigin);
				mpz_clear(zOctuplet);
				mpz_clear(zPrimeOriginOffset);
				mpz_clear(zFirstSieveElement);
				mpz_clear(zResidue);
				mpz_clear(zTwo);
				mpz_clear(zN);
				mpz_clear(zPrimorialMod);
				mpz_clear(zTempVar);


				//if (!fNewBlockRestart && !fBlockWaiting)
				//	fNewBlock = true;
			}
			catch (std::exception& e) { printf("ERROR: %s\n", e.what()); }
		}
	}
	

	ServerConnection::ServerConnection(std::string ip, std::string port, int nMaxSThreads, int nMaxPTThreads, int nMaxTimeout, bool bSolo) : IP(ip), PORT(port), TIMER(), nSieveThreads(nMaxSThreads), nPTestThreads(nMaxPTThreads), nTimeout(nMaxTimeout), bSoloMining(bSolo), THREAD(bSolo ? boost::bind(&ServerConnection::SoloServerThread, this) : boost::bind(&ServerConnection::ServerThread, this))
	{
		sieveJobQueueActive = new boost::lockfree::queue<int>(MAX_SIEVE_JOBQUEUE_SIZE);
		sieveJobQueuePassive = new boost::lockfree::queue<int>(MAX_SIEVE_JOBQUEUE_SIZE);

		pcJobQueueActive = new boost::lockfree::queue<int>(MAX_PRIME_TEST_JOBQUEUE_SIZE);
		pcJobQueuePassive = new boost::lockfree::queue<int>(MAX_PRIME_TEST_JOBQUEUE_SIZE);

		submitBlockQueue = new boost::lockfree::queue<submitBlockData>(5);

		for (size_t i = 0; i < MAX_PRIME_TEST_JOBQUEUE_SIZE; i++)
		{
			primeTestJob job;
			job.baseHash = new CBigNum(0);
			job.hashMerkleRoot = new uint512(0);
			job.candidates = (unsigned long *)malloc(sizeof(unsigned long) * MAXCANDIDATESPERSIEVE);
			job.zFirstSieveElement = (mpz_ptr)malloc(sizeof(__mpz_struct));
			job.zPrimeOrigin = (mpz_ptr)malloc(sizeof(__mpz_struct));
			mpz_init(job.zFirstSieveElement);
			mpz_init(job.zPrimeOrigin);
			primeTestJobs.insert(std::pair<int, primeTestJob>(i, job));
			pcJobQueuePassive->push(i);
		}

		nSieveJobsQueueSize = std::min(nSieveThreads * 2, MAX_SIEVE_JOBQUEUE_SIZE);

		for (size_t i = 0; i <  nSieveJobsQueueSize; i++)
		{
			sieveJob * job = new sieveJob();
			if (bSoloMining)
			{
				//job->pBlock = new CBlock();
				job->primeOrigin = 0;
			}
			else
			{
				job->pBlock.nHeight = 0;
				//job.primeOrigin = new uint1024();
			}
			sieveJobs.insert(std::pair<int, sieveJob*>(i, job));
			sieveJobQueuePassive->push(i);
		}

		for (int nIndex = 0; nIndex < nSieveThreads; nIndex++)
			THREADS.push_back(new MinerThread(this));

		for (int nIndex = 0; nIndex < nPTestThreads; nIndex++)
		{
			boost::thread * checkThread = new boost::thread(boost::bind(&ServerConnection::PrimeTestThread, this));
			PRIMETESTTHREADS.push_back(checkThread);
		}

		// printing out status text is on different low priority thread
		STATUSTHREAD = new boost::thread(boost::bind(&ServerConnection::PrintStatThread, this));
	}

	/** Reset the block on each of the Threads. **/
	void ServerConnection::ResetThreads()
	{
		fResetThreads = true; 

		/** Reset each individual flag to tell threads to stop mining. **/
		for (int nIndex = 0; nIndex < THREADS.size(); nIndex++)
		{
			LOCK(THREADS[nIndex]->MUTEX);
			THREADS[nIndex]->fNewBlock = true;
			THREADS[nIndex]->fNewBlockRestart = true;
		}

		pcJobQueueActive->consume_all([](int i) {});
		pcJobQueuePassive->consume_all([](int i) {});
		sieveJobQueueActive->consume_all([](int i) {});
		sieveJobQueuePassive->consume_all([](int i) {});
		for (size_t i = 0; i < MAX_PRIME_TEST_JOBQUEUE_SIZE; i++)
		{
			pcJobQueuePassive->push(i);
		}
		for (size_t i = 0; i < nSieveJobsQueueSize; i++)
		{
			sieveJobQueuePassive->push(i);
		}

		/** Clear the Submit Queue. **/
		SUBMIT_MUTEX.lock();

		while (!SUBMIT_QUEUE.empty())
			SUBMIT_QUEUE.pop();

		SUBMIT_MUTEX.unlock();

		while (!RESPONSE_QUEUE.empty())
			RESPONSE_QUEUE.pop();

	}

	/** Add a Block to the Submit Queue. **/
	void ServerConnection::SubmitShare(uint1024 hashPrimeOrigin, uint64 nNonce)
	{
		SUBMIT_MUTEX.lock();
		SUBMIT_QUEUE.push(std::make_pair(hashPrimeOrigin, nNonce));
		SUBMIT_MUTEX.unlock();
	}
	
	/** Main Connection Thread. Handles all the networking to allow
		Mining threads the most performance. **/
	void ServerConnection::ServerThread()
	{
		// Hight priority for the current thread in order to get & send packet ASAP 
		SetThreadPriority(THREAD.native_handle(), 32);
		/** Don't begin until all mining threads are Created. **/
		while(THREADS.size() != nSieveThreads)
			Sleep(1);

#ifdef __PGI
		// when using OpenACC the sieave thread is not CPU intensive but needs high priority to provide continous job for the GPU
		bool allMinerThreadsAreActive = false;
		do
		{
			allMinerThreadsAreActive = true;
			for (int nIndex = 0; nIndex < THREADS.size(); nIndex++)
			{
				if (THREADS[nIndex] == NULL || THREADS[nIndex]->THREAD.native_handle() == 0)
				{
					allMinerThreadsAreActive = false;
					printf("Waiting for miner threads\n");
					Sleep(1);
					break;
				}
				SetThreadPriority(THREADS[nIndex]->THREAD.native_handle(), 2);
			}
		} while (allMinerThreadsAreActive == false);
#endif

		int maxChToPrint = 9;

		/** Initialize the Server Connection. **/
		CLIENT = new LLP::Miner(IP, PORT);


		/** Initialize a Timer for the Hash Meter. **/
		TIMER.Start();
		cPrimeTimer.Start();


		const uint16_t maxJobs = nSieveThreads * 3;
		const uint16_t originSegmentSize = floor(153.0 / (float)nSieveThreads);

		bool bBlockRequestSent = false;
		loop
		{
			try
			{
			/** Run this thread at 100 Cycles per Second. **/
			Sleep(10);
			if (exitSignal)
			{
				CLIENT->Disconnect();
				break;
			}

			/** Attempt with best efforts to keep the Connection Alive. **/
			if (!CLIENT->Connected())
			{
				if (!CLIENT->Connect())
					continue;
				else
				{
					CLIENT->Login(ADDRESS);

					//CLIENT->GetBalance();
					//CLIENT->GetPayouts();

					ResetThreads();
				}
			}

			if (CLIENT->Timeout(120))
			{
				printf("Timeout in the communication protocol ...\n");
				CLIENT->Disconnect();
				Sleep(100);
				bBlockRequestSent = false;
				continue;
			}

			if (CLIENT->Errors())
			{
				printf("Error in the communication protocol ...\n");
				CLIENT->Disconnect();
				Sleep(100);
				bBlockRequestSent = false;
				continue;
			}

			if (cPrimeTimer.Elapsed() >= 1)
			{
				unsigned int nElapsed = cPrimeTimer.Elapsed();

				double PPS = (double)nPrimes / (double)(nElapsed);
				double WPS = (double)nWeight / (double)(nElapsed * 10000000);

				if (vPPSValues.size() >= 300)
					vPPSValues.pop_front();

				vPPSValues.push_back(PPS);

				if (vWPSValues.size() >= 300)
					vWPSValues.pop_front();

				vWPSValues.push_back(WPS);

				nPrimes = 0;
				nWeight = 0;
				cPrimeTimer.Reset();
			}

			/** Rudimentary Meter **/
			if (TIMER.Elapsed() >= 10)
			{
				//double PPS = 1.0 * std::accumulate(vPPSValues.begin(), vPPSValues.end(), 0LL) / vPPSValues.size();
				unsigned int SecondsElapsed = (unsigned int)time(0) - nStartTimer;
				double shareValPH = ((double)totalShareWeight / SecondsElapsed) * 3.6;
				double WPS = 1.0 * std::accumulate(vWPSValues.begin(), vWPSValues.end(), 0LL) / vWPSValues.size();
				
				if (!bSoloMining)
				{
					CLIENT->SubmitPPS(shareValPH, WPS);
					CLIENT->GetBalance();
					CLIENT->GetPayouts();
				}
				//PrintStats();
				TIMER.Reset();
			}


			/** Submit any Shares from the Mining Threads. **/
			SUBMIT_MUTEX.lock();
			while (!SUBMIT_QUEUE.empty())
			{
				std::pair<uint1024, uint64> pShare = SUBMIT_QUEUE.front();
				SUBMIT_QUEUE.pop();

				CLIENT->SubmitShare(pShare.first, pShare.second);
				RESPONSE_QUEUE.push(pShare);
			}
			SUBMIT_MUTEX.unlock();


			/** Check if there is work to do for each Miner Thread. **/
			if (!sieveJobQueuePassive->empty() && !bBlockRequestSent)
			{
				//printf("Requesting new block form the server.\n");
				CLIENT->GetBlock();
				bBlockRequestSent = true;
			}


			CLIENT->ReadPacket();
			if (!CLIENT->PacketComplete())
				continue;

			/** Handle the New Packet, and Interpret its Data. **/
			LLP::Packet PACKET = CLIENT->NewPacket();
			CLIENT->ResetPacket();


			/** Output if a Share is Accepted. **/
			if (PACKET.HEADER == CLIENT->ACCEPT)
			{
				if (RESPONSE_QUEUE.empty())
					continue;

				std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
				RESPONSE_QUEUE.pop();

				double nDiff = GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1);
				printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Accepted]\n", nDiff, pResponse.first.ToString().substr(0, 20).c_str());
				//totalShareWeight += pow(13.0, nDiff - 2.0);
				totalShareWeight += pow(25.0, floor(nDiff) - 3.0); //For better share rewards that pools should implement
			}


			/** Output if a Share is Rejected. **/
			else if (PACKET.HEADER == CLIENT->REJECT)
			{
				if (RESPONSE_QUEUE.empty())
					continue;

				std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
				RESPONSE_QUEUE.pop();

				printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Rejected]\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().substr(0, 20).c_str());
			}

			/** Output if a Share is a Block **/
			else if (PACKET.HEADER == CLIENT->BLOCK)
			{
				if (RESPONSE_QUEUE.empty())
					continue;

				std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
				RESPONSE_QUEUE.pop();

				printf("\n******************************************************\n\nBlock Accepted | Difficulty %f | Hash %s\n\n******************************************************\n\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().c_str());
			}

			/** Output if a Share is Stale **/
			else if (PACKET.HEADER == CLIENT->STALE)
			{
				if (RESPONSE_QUEUE.empty())
					continue;

				std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
				RESPONSE_QUEUE.pop();

				printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Stale]\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().substr(0, 20).c_str());
			}


			/** Reset the Threads if a New Block came in. **/
			else if (PACKET.HEADER == CLIENT->NEW_BLOCK)
			{
				printf("[MASTER] NXS Network: New Block\n");

				ResetThreads();

				bBlockRequestSent = false;
			}

			/** Set the Current Account Balance if Message Received. **/
			else if (PACKET.HEADER == CLIENT->ACCOUNT_BALANCE) { nAccountBalance = bytes2uint64(PACKET.DATA); }


			/** Set the Current Pending Payout if Message Received. **/
			else if (PACKET.HEADER == CLIENT->PENDING_PAYOUT) { nCurrentPayout = bytes2uint64(PACKET.DATA); }


			/** Set the Block for the Thread if there is a New Block Packet. **/
			else if (PACKET.HEADER == CLIENT->BLOCK_DATA)
			{
				if (!sieveJobQueuePassive->empty())
				{
					std::vector<unsigned char> primeOriginVector = std::vector<unsigned char>(PACKET.DATA.begin(), PACKET.DATA.end() - 12);
					uint32_t nMinShare = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 12, PACKET.DATA.end() - 8));
					nMinimumShare = nMinShare;
					uint32_t nReceivedBlockHeight = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 4, PACKET.DATA.end()));
					if (nReceivedBlockHeight < nBestHeight)
					{
						printf("[MASTER] Received Obsolete Block %u... Requesting New Block.\n", nReceivedBlockHeight);
						CLIENT->GetBlock();					
						continue;
					}
					nDifficulty = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 8, PACKET.DATA.end() - 4));
					nBestHeight = nReceivedBlockHeight;
					bBlockRequestSent = false; //received response for block request, ready to get a new one.
								
					for (int nIndex = 0; nIndex < THREADS.size(); nIndex++)
					{
						int jobId = -1;
						if (sieveJobQueuePassive->pop(jobId) && jobId != -1)
						{
							sieveJob * job = sieveJobs.at(jobId);

							job->primeOrigin.SetBytes(primeOriginVector);
							job->pBlock.nHeight = 0; // this indicates that we are pool mining and the hash is in the primeOringin and not in the CBlock
							job->nStartOrigin = nIndex * originSegmentSize;
							job->nMaxOriginCount = originSegmentSize;
							job->nMinimumShare = nMinimumShare;
							job->nHeight = nReceivedBlockHeight;
							sieveJobQueueActive->push(jobId);
							//printf("Created Job (%u) from Block %s \n", jobId, job->primeOrigin.GetHex().substr(0, 20).c_str());
						}
						THREADS[nIndex]->fNewBlock = false;
						//THREADS[nIndex]->fNewBlockRestart = false;
					}

					fResetThreads = false;
				}

			}

		}
		catch (std::exception& e)
		{
			printf("%s\n", e.what()); CLIENT = new LLP::Miner(IP, PORT);
		}
		}
	}

	void ServerConnection::PrintStatThread()
	{
		do
		{
			try
			{
				Sleep(5000);
				if (exitSignal)
					break;
				PrintStats();				
			}
			catch (const std::exception&)
			{

			}
		} while (true);
	}
	
	void ServerConnection::SoloServerThread()
	{
		SetThreadPriority(THREAD.native_handle(), 32);

#ifdef __PGI
		// when using OpenACC the sieave thread is not CPU intensive but needs high priority to provide continous job for the GPU
		bool allMinerThreadsAreActive = false;
		do
		{
			allMinerThreadsAreActive = true;
			for (int nIndex = 0; nIndex < THREADS.size(); nIndex++)
			{
				if (THREADS[nIndex] == NULL || THREADS[nIndex]->THREAD.native_handle() == 0)
				{
					allMinerThreadsAreActive = false;
					printf("Waiting for miner threads\n");
					Sleep(1);
					break;
				}
				SetThreadPriority(THREADS[nIndex]->THREAD.native_handle(), 2);
			}
		} while (allMinerThreadsAreActive == false);
#endif


		/** Don't begin until all mining threads are Created. **/
		//while(THREADS.size() != nThreads)
		//	Sleep(1);

		/** Initialize the Server Connection. **/
		CLIENT = new LLP::Miner(IP, PORT);


		/** Initialize a Timer for the Hash Meter. **/
		TIMER.Start();
		cPrimeTimer.Start();
		int maxSieveJobInQueue = 6;
		//unsigned int nBestHeight = 0;

		const uint16_t maxJobs = nSieveThreads * 3;
		const uint16_t originSegmentSize = floor(153.0 / (float)nSieveThreads);

		loop
		{
			try
			{
				/** Run this thread at 5 Cycles per Second. **/
				Sleep(200);
				if (exitSignal)
				{
					CLIENT->Disconnect();
					break;
				}
				/** Attempt with best efforts to keep the Connection Alive. **/
				if (!CLIENT->Connected())
				{
					if (!CLIENT->Connect())
					{
						printf("Failed to connect. Trying again.\n");
						Sleep(500);
						continue;
					}
					else
					{
						CLIENT->SetChannel(1);
						ResetThreads();
					}
				}

				if (CLIENT->Timeout(120))
				{
					printf("Timeout in the communication protocol ...\n");
					CLIENT->Disconnect();
					Sleep(100);
					continue;
				}

				if (CLIENT->Errors())
				{
					printf("Error in the communication protocol ...\n");
					CLIENT->Disconnect();
					Sleep(100);
					continue;
				}

				bool fNewRound = CLIENT->NewRound();
				
				unsigned int nHeight = CLIENT->GetHeight();
				if (nHeight == -1)
				{
					printf("Failed to Update Height...\n");
					CLIENT->Disconnect();
					//delete CLIENT;
					//CLIENT = new LLP::SoloMiner(IP, PORT);
					Sleep(500);
					continue;
				}

				/** If there is a new block, Flag the Threads to Stop Mining. **/
				if (fNewRound || nHeight != nBestHeight)
				{
					nBestHeight = nHeight;
					printf("[MASTER] NXS Network: New Block %u\n", nHeight);
					ResetThreads();
				}


				if (cPrimeTimer.Elapsed() >= 1)
				{
					unsigned int nElapsed = cPrimeTimer.Elapsed();

					double PPS = (double)nPrimes / (double)(nElapsed);
					double WPS = (double)nWeight / (double)(nElapsed * 10000000);

					if (vPPSValues.size() >= 300)
						vPPSValues.pop_front();

					vPPSValues.push_back(PPS);

					if (vWPSValues.size() >= 300)
						vWPSValues.pop_front();

					vWPSValues.push_back(WPS);

					nPrimes = 0;
					nWeight = 0;
					cPrimeTimer.Reset();
				}

				/** Rudimentary Meter **/
				//if (TIMER.Elapsed() >= 5)
				//{
				//	//PrintStats();
				//	TIMER.Reset();
				//}


				/** Submit any Shares from the Mining Threads. **/
				while (!submitBlockQueue->empty())
				{

					bBlockSubmission = true;
					submitBlockData data;
					submitBlockQueue->pop(data);
										
					nBlocksFoundCounter++;

					//printf("\nSubmitting Block %s\n", data.baseHash->GetHex().c_str());

					printf("[MASTER] Prime Cluster of Difficulty %f Found \n", GetPrimeDifficulty(*data.baseHash + data.nNonce, 0));
					
					/** Attempt to Submit the Block to Network. **/
					unsigned char RESPONSE = CLIENT->SubmitBlock(*data.hashMerkleRoot, data.nNonce);
					bBlockSubmission = false;

					/** Check the Response from the Server.**/
					if (RESPONSE == 200)
					{
						printf("[MASTER] Block Accepted By NXS Network.\n");
						nBlocksAccepted++;

						/* Clear the submission queue and reset threads as the height is going to change. */
						submitBlockQueue->consume_all([](submitBlockData data) {});
						ResetThreads();
					}
					else if (RESPONSE == 201)
					{
						printf("[MASTER] Block Rejected by NXS Network.\n");
						nBlocksRejected++;
						ResetThreads();
					}
					/** If the Response was Bad, Reconnect to Server. **/
					else
					{
						nBlocksRejected++;
						printf("[MASTER] Failure to Submit Block. Reconnecting...\n");
						CLIENT->Disconnect();
						Sleep(100);
						ResetThreads();
						break;
					}
					//RESPONSE_QUEUE.push(pShare);
				}

				while (!sieveJobQueuePassive->empty())
				{
					CBlock block;
					if (CLIENT->GetSoloBlock(&block))
					{
						for (int nIndex = 0; nIndex < THREADS.size(); nIndex++)
						{
							int jobId = -1;
							if (sieveJobQueuePassive->pop(jobId) && jobId != -1)
							{
								sieveJob * job = sieveJobs.at(jobId);
								job->pBlock.nNonce = 0;
								job->primeOrigin = 0; // this is used only for pool mining. setting it to 0 also to indicate solo
								job->pBlock.Set(&block);
								job->nStartOrigin = nIndex * originSegmentSize;
								job->nMaxOriginCount = originSegmentSize;
								nMinimumShare = block.nBits;
								nDifficulty = nMinimumShare;
								job->nMinimumShare = nMinimumShare;
								job->nHeight = block.nHeight;
								sieveJobQueueActive->push(jobId);
								//printf("Created Job (%u) from Block %s \n", jobId, job->pBlock.GetHash().GetHex().substr(0, 20).c_str());
							}
							THREADS[nIndex]->fNewBlock = false;
							//THREADS[nIndex]->fNewBlockRestart = false;
						}

						

					}
					else
					{
						printf("<DEBUG> Got an invalid BLOCK\n");
						CLIENT->Disconnect();
						Sleep(500);												
						CLIENT = new LLP::Miner(IP, PORT);
						Sleep(500);
						ResetThreads();
					}
				}

				fResetThreads = false;


			}
			catch (std::exception& e)
			{
				printf("%s\n", e.what());
			}
		}
	}

	void ServerConnection::PrintStats()
	{
		int maxChToPrint = 9;
		if (sieveCount == 0)
			sieveCount++;
		nElements++;

		unsigned int SecondsElapsed = (unsigned int)time(0) - nStartTimer;
		unsigned int nElapsed = TIMER.Elapsed();
		uint64 avgSieveTime = sieveTime / sieveCount;
		double SPS = (double)sieveCount / (double)SecondsElapsed;
		double TPS = (double)testCount / (double)SecondsElapsed;
		//double PPS = 1.0 * std::accumulate(vPPSValues.begin(), vPPSValues.end(), 0LL) / vPPSValues.size();
		double WPS = 1.0 * std::accumulate(vWPSValues.begin(), vWPSValues.end(), 0LL) / vWPSValues.size();;

		struct tm t;
		time_t aclock = SecondsElapsed;
		gmtime_r(&aclock, &t);

		printf("[METERS]  %-5.02f WPS | Largest Share %f | Diff. %f | Height = %u ", 
			 WPS, nLargestShare / 10000000.0, nDifficulty / 10000000.0, nBestHeight);
		if (bSoloMining)
			printf("| Block(s) ACC=%u REJ=%u ", nBlocksAccepted, nBlocksRejected);
		else
			printf("| Balance: %-3.03f NXS | Payout: %-3.03f NXS ", nAccountBalance / 1000000.0, nCurrentPayout / 1000000.0);
		
		printf("| %02dd-%02d:%02d:%02d\n", t.tm_yday, t.tm_hour, t.tm_min, t.tm_sec);



		printf("-----------------------------------------------------------------------------------------------\nch  \t| ");
		for (int i = 3; i <= maxChToPrint; i++)
			printf("%-7d  |  ", i);
		printf("\n---------------------------------------------------------------------------------------------\ncount\t| ");
		for (int i = 3; i <= maxChToPrint; i++)
			printf("%-7d  |  ", chainCounter[i]);
		printf("\n---------------------------------------------------------------------------------------------\nch/m\t| ");
		for (int i = 3; i <= maxChToPrint; i++)
		{
			double sharePerHour = ((double)chainCounter[i] / SecondsElapsed) * 60.0;
			printf("%-7.03f  |  ", sharePerHour);
		}
		printf("\n--------------------------------------------------------------------------------------------\nratio\t| ");
		for (int i = 3; i <= maxChToPrint; i++)
		{
			double chRatio = 0;
			if (chainCounter[i] != 0)
				chRatio = ((double)chainCounter[i - 1] / (double)chainCounter[i]);
			printf("%-7.03f  |  ", chRatio);
		}

		double sharePerH = ((double)shareCount / SecondsElapsed) * 3600;
		double shareWeightPerH = ((double)totalShareWeight / SecondsElapsed) * 3.6;
		printf("\n---------------------------------------------------------------------------------------------\n");
		printf("AVG Time - Sieve: %llu | PTest: %llu | Siv/s: %-7.03f | Tst/s: %-7.03f", avgSieveTime, testCount == 0 ? 0 : pTestTime / testCount, SPS, TPS);
		if (!bSoloMining)
			printf(" | Shares: %llu | Shr/h: %-7.03f | TotalShareValue: %llu | ShrVal/h: %-5.02fK", shareCount, sharePerH, totalShareWeight, shareWeightPerH);
		//printf("\nAvgCandCnt: %llu / %llu - CandHit : %-2.02f%% 2nd : %-2.02f%%", \
		//	testCount == 0 ? 0 : candidateCount / testCount, testCount == 0 ? 0 : sieveCandidateCount / testCount, \
		//	candidateCount == 0 ? 0 : (double)(candidateHitCount * 100) / (double)candidateCount, \
		//	candidateCount == 0 ? 0 : (double)(candidateHit2Count * 100) / (double)candidateCount);
		printf("\n\n");
	}

	int find_tuples2(unsigned long * candidates, mpz_t zPrimorial, mpz_t zPrimeOrigin, mpz_t zFirstSieveElement, unsigned int nMinimumPrimeCount, std::vector<std::pair<uint64_t, uint16_t>> * nonces)
	{
		mpz_t zPrimeOriginOffset, zTempVar, zTempVar2;
		mpz_init(zPrimeOriginOffset);
		mpz_init(zTempVar);
		mpz_init(zTempVar2);
		int n1stPrimeSearchLimit = 0;
		int cx = 0;
		int nPrimes = 0;

		while (candidates[cx] != -1 && cx < MAXCANDIDATESPERSIEVE)
		{			
			mpz_mul_ui(zTempVar, zPrimorial, candidates[cx]);
			cx++;
			mpz_add(zTempVar, zFirstSieveElement, zTempVar);
			mpz_set(zPrimeOriginOffset, zTempVar);

			unsigned long long nNonce = 0;
			unsigned int nPrimeCount = 0;
			unsigned int nSieveDifficulty = 0;
			unsigned long nStart = 0;
			unsigned long nStop = 0;
			unsigned long nLastOffset = 0;
			int firstPrimeAt = -1;

			if (cx <= 10)
				n1stPrimeSearchLimit = 12;
			else
			{
				mpz_add_ui(zTempVar2, zTempVar, 18);
				if (mpz_probab_prime_p(zTempVar2, 0) > 0)
					n1stPrimeSearchLimit = 12;
				else
					//				continue;
				{ //!!!!! 6
					mpz_add_ui(zTempVar2, zTempVar, 20);
					if (mpz_probab_prime_p(zTempVar2, 0) > 0)
					{
						candidateHit2Count++;
						n1stPrimeSearchLimit = 18;
					}
					else
						//{
						//	mpz_add_ui(zTempVar2, zTempVar, 26);
						//	if (mpz_probab_prime_p(zTempVar2, 0) > 0)
						//		n1stPrimeSearchLimit = 26;
						//	else
						//		continue;
						//}
						continue;
				}
				candidateHitCount++;
				nPrimes++;
			}
			nStop = 0; nPrimeCount = 0; nLastOffset = 0; firstPrimeAt = -1;
			double diff = 0;

			for (nStart = 0; nStart <= nStop + 12; nStart += 2)
			{
				//if (nStart == 4 || nStart == 14 || nStart == 22 || nStart == 10 || nStart == 14 || nStart == 16 || nStart == 24)
				//{
				//	mpz_add_ui(zTempVar, zTempVar, 2);
				//	nLastOffset += 2;
				//	continue;
				//}

				if (mpz_probab_prime_p(zTempVar, 0) > 0)
				{
					nStop = nStart;
					nPrimeCount++;
					
				}
				if (nPrimeCount == 0 && nStart >= n1stPrimeSearchLimit)
					break;

				if ((firstPrimeAt == -1 && nPrimeCount == 1))
				{
					mpz_set(zPrimeOriginOffset, zTempVar); // zPrimeOriginOffset = zTempVar
					firstPrimeAt = nStart;
				}

				mpz_add_ui(zTempVar, zTempVar, 2);
				nLastOffset += 2;
			}

			if (nPrimeCount >= nMinimumPrimeCount)
			{
				mpz_sub(zTempVar, zPrimeOriginOffset, zPrimeOrigin);

#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
				//nNonce = mpz2uint64(zTempVar);
				nNonce = zTempVar->_mp_d[0];
#else
				
				nNonce = mpz_get_ui(zTempVar);
#endif
				nonces->push_back(std::pair<uint64_t, uint16_t>(nNonce, nPrimeCount));
			}
		}
		candidateCount += cx;
		//if (nonces->size() > 0)
		//	printf("Found %u %u+ tuples out of %u candidates\n", nonces->size(), nMinimumPrimeCount, cx);
		return nPrimes;
	}
	
	void ServerConnection::PrimeTestThread()
	{
		//printf("Starting Prime test thread!\n");
		CPrimeTest primeTest;
		loop
		{
			if (exitSignal)
				break;
			
			int jobId = 0;
			if (pcJobQueueActive->pop(jobId))
			{
				
				/* Pause mining if submitting a block */
				if (bBlockSubmission )
				{
					Sleep(20);
					continue;
				}
				primeTestJob job = primeTestJobs.at(jobId);

				std::vector<std::pair<uint64_t,uint16_t>>  nonces;
				int64 nStartTime = GetTimeMicros();
				nPrimes += primeTest.FindTuples(job.candidates, job.zPrimeOrigin, job.zFirstSieveElement, &nonces);
				//nPrimes += find_tuples2(job.candidates, zPrimorial, job.zPrimeOrigin, job.zFirstSieveElement, 3, &nonces);
				pTestTime += (GetTimeMicros() - nStartTime);
				testCount++;
				pcJobQueuePassive->push(jobId);
				
				/* Before we check the nonces, make one last check that the threads have not been flagged to be reset */
				if(fResetThreads)
					continue;
					
				for (std::vector<std::pair<uint64_t, uint16_t>>::iterator it = nonces.begin(); it != nonces.end(); ++it)
				{
					uint64_t nNonce = it->first;					
					unsigned int nDiff = 30000000;
					if (it->second > 3)
						nDiff = GetPrimeBits((*job.baseHash + nNonce), 1);
					uint32_t nPrimeCount = nDiff / 10000000;
					//printf(" - %f - Nonce: %u\n", (double)nDiff / 10000000.0, nNonce);
					if (nPrimeCount < 14)
					chainCounter[nPrimeCount]++;
					nWeight += nDiff * 50;

					if (nDiff > nLargestShare)
						nLargestShare = nDiff;

					if (nDiff < nMinimumShare)
						continue;

					if (nDiff >= nMinimumShare)
					{
						if (bSoloMining)
						{
							bBlockSubmission = true;
							printf("!!! BLOCK found !!! %ul\n", nDiff);
							submitBlockData data;
							data.baseHash =  new CBigNum( *job.baseHash);
							data.hashMerkleRoot = new uint512(*job.hashMerkleRoot);
							data.nNonce = nNonce;
							data.nDiff = nDiff;
							submitBlockQueue->push(data);
						}
						else
						{
							printf("Submitting share %ul\n", nDiff);
							this->SubmitShare(job.baseHash->getuint1024(), nNonce);
							shareCount++;
						}
					}
				}


			}
			else
				Sleep(10);
		}
	}
}

void  INThandler(int sig)
{
	signal(sig, SIG_IGN);
	printf("Quitting !!!\r\n");
	exitSignal = true;
	Sleep(500);
	exit(0);
}


int main(int argc, char *argv[])
{
#if (defined _WIN32 || defined WIN32) && !defined __MINGW32__
	//TODO: Stack in VS ???
#else
	const rlim_t kStackSize = 32 * 1024 * 1024;   // min stack size = 16 MB
	struct rlimit rl;
	int result;

	result = getrlimit(RLIMIT_STACK, &rl);
	if (result == 0)
	{
		if (rl.rlim_cur < kStackSize)
		{
			rl.rlim_cur = kStackSize;
			result = setrlimit(RLIMIT_STACK, &rl);
			if (result != 0)
			{
				fprintf(stderr, "setrlimit returned result = %d\n", result);
			}
		}
	}
#endif
	signal(SIGINT, INThandler);
	// HashToBeWild!:
	std::string IP = "";
	std::string PORT = "";
	ADDRESS = "";

	for (int i = 0; i < sizeof(chainCounter) / sizeof(uint32_t); i++)
	{
		chainCounter[i] = 0;
	}


	Core::MinerConfig Config;
	if (Config.ReadConfig())
	{
		//printf("Using the config file...\n");
		// populate settings from config object
		IP = Config.strHost;
		PORT = Config.nPort;
		ADDRESS = Config.strNxsAddress;
		bUseExperimentalSieve = Config.bExperimental;
	}
	else
	{
		printf("'miner.conf' config file not available... using command line\n");
		if (argc < 4)
		{
			printf("Too Few Arguments. The Required Arguments are 'IP PORT ADDRESS'\n");
			printf("Default Arguments are Total Threads = CPU Cores and Connection Timeout = 10 Seconds\n");
			printf("Format for Arguments is 'IP PORT ADDRESS SIEVE-THREADS PRIMETEST-THREADS TIMEOUT'\n");
			printf("Solo mining example: ."); printf(SLASH); printf("nexus_cpuminer localhost 9325 \"\"\n");
			printf("Pool mining example: ."); printf(SLASH); printf("nexus_cpuminer nexusminingpool.com 9549 2SSbxVqakuEQeTLk8cgGKeWCiVrsyJtFMrvKxJDmuY3nRAmr2uJ\n");
			Sleep(10000);
			return 0;
		}
	}

	// Command line overrides the config file
	if (argc > 3)
	{
		IP = argv[1];
		PORT = argv[2];
		ADDRESS = argv[3];
	}

	if (ADDRESS.length() == 0)
		bSoloMining = true;
	
	int nSieveThreads = GetTotalCores();
	int nPTestThreads = GetTotalCores();
	int nTimeout = 5;

	// When using GPU with OpenACC max sieve threads is one, Prime Test as many as possible
	#ifdef __PGI
	nSieveThreads = 1;
	nPTestThreads = GetTotalCores() * 2;
	#endif
	if (argc > 4)
	{
		nSieveThreads = boost::lexical_cast<int>(argv[4]);
	}
	else
	{
		if (Config.nSieveThreads > 0)
		{
			nSieveThreads = Config.nSieveThreads;
		}
	}

	if (argc > 5)
	{
		nPTestThreads = boost::lexical_cast<int>(argv[5]);
	}
	else
	{
		if (Config.nPTestThreads > 0)
		{
			nPTestThreads = Config.nPTestThreads;
		}
	}

	if (argc > 6)
	{
		nTimeout = boost::lexical_cast<int>(argv[6]);
	}
	else
	{
		if (Config.nTimeout > 0)
		{
			nTimeout = Config.nTimeout;
		}
	}


	printf("Nexus Prime Pool & Solo Miner 1.2.3 - Created by Videlicet - Optimized by Supercomputing, paulscreen, hashtobewild & mumus\n");
	if (!bSoloMining) printf("Using Supplied Account Address %s\n", ADDRESS.c_str());
	else printf("Solo mining mode\n");
	printf("Initializing Miner \n");
	printf("Host %s\n", IP.c_str());
	printf("Port: %s\n", PORT.c_str());
	printf("SieveThreads: %i\n", nSieveThreads);
	printf("Prime Test Threads: %i\n", nPTestThreads);
	printf("Timeout = %i\n\n", nTimeout);

	if (bSoloMining && (PORT.compare("9325") != 0 && PORT.compare("8325") != 0))
	{
		printf("\n\n!!!!!!! Warning! The miner is in SOLO mode but the specified PORT wans't set to the standard port number 9325 (8325 on testnet)\n\n");
		printf("Pointing a solo miner to a pool server in will result in IP ban from the pool.");
		Sleep(5000);
	}

	Core::InitializePrimes();

	nStartTimer = (unsigned int)time(0);
	Core::nBitArray_Size = Config.nBitArraySize;
	Core::prime_limit = Config.primeLimit;
	Core::nPrimeLimit = Config.nPrimeLimit;
	Core::nPrimorialEndPrime = Config.nPrimorialEndPrime;
	Core::ServerConnection * MINERS = new Core::ServerConnection(IP, PORT, nSieveThreads, nPTestThreads, nTimeout, bSoloMining);
	loop { Sleep(1000); }
	do
	{
		char ch = getchar();
		if (ch == 'q')
			break;
		printf("Enter 'q' to quit\n");
	} while (true);
	
	exitSignal = true;
	printf("Quitting !!!\n");
	Sleep(1000);
	delete MINERS;
	Sleep(500);
	return 0;
}

