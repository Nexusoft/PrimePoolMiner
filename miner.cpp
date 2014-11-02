#include "core.h"

unsigned int nBlocksFoundCounter = 0;
unsigned int nBlocksAccepted = 0;
unsigned int nBlocksRejected = 0;
unsigned int nDifficulty = 0;
unsigned int nBestHeight = 0;
unsigned int nStartTimer = 0;
bool isBlockSubmission = false;

std::string ADDRESS;
uint64 nCurrentPayout = 0, nAccountBalance = 0;

namespace Core
{

	/** Class to hold the basic data a Miner will use to build a Block.
		Used to allow one Connection for any amount of threads. **/
	class MinerThread
	{
	public:
		uint1024 hashPrimeOrigin;
		uint64 nNonce;
		unsigned int nMinimumShare, nDifficulty;
		bool fBlockFound, fNewBlock, fBlockWaiting, fShareResponse;
		LLP::Thread_t THREAD;
		boost::mutex MUTEX;
		
		unsigned int nPrimes = 0;
		MinerThread() : fBlockFound(false), fNewBlock(true), fBlockWaiting(false), fShareResponse(true), THREAD(boost::bind(&MinerThread::PrimeMiner, this)) { }
		
		/** Main Miner Thread. Bound to the class with boost. Might take some rearranging to get working with OpenCL. **/
		void PrimeMiner()
		{
			loop
			{
				try
				{
					/** Keep thread at idle CPU usage if waiting to submit or recieve block. **/
					Sleep(1);
					
					/** Assure that this thread stays idle when waiting for new block, or share submission. **/
					if(fNewBlock || fBlockFound || fBlockWaiting)
						continue;

					/** Lock the Thread at this Mutex when Changing Block Pointer. **/
					MUTEX.lock();
					CBigNum BaseHash(hashPrimeOrigin);
					nNonce = 0;
					MUTEX.unlock();
					
					mpz_t zPrimeOrigin, zPrimeOriginOffset, zFirstSieveElement, zPrimorialMod, zTempVar, zResidue, zTwo, zN, zNewTemp;
							
					unsigned int i = 0;
					unsigned int j = 0;
					unsigned int nSize = 0;
					unsigned int nPrimeCount = 0;
					unsigned int nSieveDifficulty = 0;
					uint64 nStart = 0;
					uint64 nStop = 0;
					unsigned int nLastOffset = 0;

					long nElapsedTime = 0;
					long nStartTime = 0;

					mpz_init(zPrimeOriginOffset);
					mpz_init(zFirstSieveElement);
					mpz_init(zPrimorialMod);

					mpz_init(zNewTemp);
					mpz_init(zTempVar);
					mpz_init(zPrimeOrigin);
					mpz_init(zResidue);
					mpz_init_set_ui(zTwo, 2);
					mpz_init(zN);

					bignum2mpz(&BaseHash, zPrimeOrigin);
					nSize = mpz_sizeinbase(zPrimeOrigin,2);

					unsigned char* bit_array_sieve = (unsigned char*)malloc((nBitArray_Size)/8);
					for(j=0; j<256 && !fNewBlock && !fBlockWaiting; j++)
					{
						memset(bit_array_sieve, 0x00, (nBitArray_Size)/8);

						mpz_mod(zPrimorialMod, zPrimeOrigin, zPrimorial);
						mpz_sub(zPrimorialMod, zPrimorial, zPrimorialMod);

						mpz_mod(zPrimorialMod, zPrimorialMod, zPrimorial);
						mpz_add_ui(zPrimorialMod, zPrimorialMod, octuplet_origins[j]);
						mpz_add(zTempVar, zPrimeOrigin, zPrimorialMod);

						mpz_set(zFirstSieveElement, zTempVar);

						for(unsigned int i=nPrimorialEndPrime; i<nPrimeLimit && !fNewBlock && !fBlockWaiting; i++)
						{
							unsigned long  p = primes[i];
							unsigned int inv = inverses[i];
							unsigned int base_remainder = mpz_tdiv_ui(zTempVar, p);

							unsigned int remainder = base_remainder;
							unsigned long r = (p-remainder)*inv;
							unsigned int index = r % p;
							while(index < nBitArray_Size)
							{
								//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
									bit_array_sieve[(index)>>3] |= (1<<((index)&7));
								index += p;
							}
							
							remainder = base_remainder + 2;
							if (p<remainder)
								remainder -= p;
							r = (p-remainder)*inv;
							index = r % p;
							while(index < nBitArray_Size)
							{
								//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
									bit_array_sieve[(index)>>3] |= (1<<((index)&7));
								index += p;
							}

							remainder = base_remainder + 6;
							if (p<remainder)
								remainder -= p;
							r = (p-remainder)*inv;
							index = r % p;
							while(index < nBitArray_Size)
							{
								//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
									bit_array_sieve[(index)>>3] |= (1<<((index)&7));
								index += p;
							}

							remainder = base_remainder + 8;
							if (p<remainder)
								remainder -= p;
							r = (p-remainder)*inv;
							index = r % p;
							while(index < nBitArray_Size)
							{
								//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
									bit_array_sieve[(index)>>3] |= (1<<((index)&7));
								index += p;
							}

							if ( nDifficulty > 50000000)
							{
								remainder = base_remainder + 12;
								if (p<remainder)
									remainder -= p;
								r = (p-remainder)*inv;
								index = r % p;
								while(index < nBitArray_Size)
								{
									//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
										bit_array_sieve[(index)>>3] |= (1<<((index)&7));
									index += p;
								}
							}

							if ( nDifficulty > 60000000)
							{
								remainder = base_remainder + 18;
								if (p<remainder)
									remainder -= p;
								r = (p-remainder)*inv;
								index = r % p;
								while(index < nBitArray_Size)
								{
									//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
										bit_array_sieve[(index)>>3] |= (1<<((index)&7));
									index += p;
								}
							}

							if ( nDifficulty > 70000000)
							{
								remainder = base_remainder + 20;
								if (p<remainder)
									remainder -= p;
								r = (p-remainder)*inv;
								index = r % p;
								while(index < nBitArray_Size)
								{
									//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
										bit_array_sieve[(index)>>3] |= (1<<((index)&7));
									index += p;
								}
							}

							if ( nDifficulty > 80000000)
							{
								remainder = base_remainder + 26;
								if (p<remainder)
									remainder -= p;
								r = (p-remainder)*inv;
								index = r % p;
								while(index < nBitArray_Size)
								{
									//if( !(bit_array_sieve[(i)>>3] & (1<<((i)&7))) )
										bit_array_sieve[(index)>>3] |= (1<<((index)&7));
									index += p;
								}
							}
						}

						for(i=0; i<nBitArray_Size && !fNewBlock && !fBlockWaiting; i++)
						{
							if( bit_array_sieve[(i)>>3] & (1<<((i)&7)) )
								continue;

							// p1
							mpz_mul_ui(zTempVar, zPrimorial, i);
							mpz_add(zTempVar, zFirstSieveElement, zTempVar);
							mpz_set(zPrimeOriginOffset, zTempVar);

							mpz_sub_ui(zN, zTempVar, 1);
							mpz_powm(zResidue, zTwo, zN, zTempVar);
							if (mpz_cmp_ui(zResidue, 1) != 0)
								continue;

							nStart = 0;
							nStop = 2;
							nPrimeCount = 0;
							nLastOffset = 0;
							nPrimes++;

							for(nStart; nStart <= nStop + 12; nStart += 2)
							{
								mpz_sub_ui(zN, zTempVar, 1);
								mpz_powm(zResidue, zTwo, zN, zTempVar);
								if (mpz_cmp_ui(zResidue, 1) == 0)
								{
									//uint64 nonce = mpz_get_ui(zTempVar);
									//CBigNum PRIME(BaseHash + nonce);
									//printf("[PRIME] Prime Test %llu:%s\n", nonce , PrimeCheck(PRIME, 1) ? "TRUE" : "FALSE");
									
									nStop = nStart;
									nPrimeCount++;
									nPrimes++;
								}

								mpz_add_ui(zTempVar, zTempVar, 2);
								nLastOffset+=2;
							}

							//mpz_set(zNewTemp, zTempVar);

							if(nPrimeCount >= 3)
							{	
								mpz_sub(zTempVar, zPrimeOriginOffset, zPrimeOrigin);
								nNonce = mpz2uint64(zTempVar);
							
								double nDiff = GetPrimeBits((BaseHash + nNonce), 1);
								if(nDiff >= nMinimumShare)
								{
									fBlockFound = true;
									
									while(!fNewBlock && fBlockFound)
										Sleep(1);
								}
							}
						}
					}

					mpz_clear(zPrimeOrigin);
					mpz_clear(zPrimeOriginOffset);
					mpz_clear(zFirstSieveElement);
					mpz_clear(zResidue);
					mpz_clear(zTwo);
					mpz_clear(zN);
					mpz_clear(zPrimorialMod);
					mpz_clear(zTempVar);

					free(bit_array_sieve);
				}
				catch(std::exception& e){ printf("ERROR: %s\n", e.what()); }
			}
		}
	};
	
	
	/** Class to handle all the Connections via Mining LLP.
		Independent of Mining Threads for Higher Efficiency. **/
	class ServerConnection
	{
	public:
		LLP::Miner* CLIENT;
		int nThreads, nTimeout;
		std::vector<MinerThread*> THREADS;
		LLP::Thread_t THREAD;
		LLP::Timer    TIMER;
		std::string   IP, PORT;
		
		ServerConnection(std::string ip, std::string port, int nMaxThreads, int nMaxTimeout) : IP(ip), PORT(port), TIMER(), nThreads(nMaxThreads), nTimeout(nMaxTimeout), THREAD(boost::bind(&ServerConnection::ServerThread, this))
		{
			for(int nIndex = 0; nIndex < nThreads; nIndex++)
				THREADS.push_back(new MinerThread());
		}
		
		/** Reset the block on each of the Threads. **/
		void ResetThreads()
		{
		
			/** Reset each individual flag to tell threads to stop mining. **/
			for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
			{
				THREADS[nIndex]->fBlockFound   = false;
				THREADS[nIndex]->fNewBlock      = true;
				THREADS[nIndex]->fShareResponse = true;
			}
				
		}
		
		/** Get the total Primes Found from Each Mining Thread.
			Then reset their counter. **/
		unsigned int Primes()
		{	
			unsigned int nPrimes = 0;
			for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
			{
				nPrimes += THREADS[nIndex]->nPrimes;
				THREADS[nIndex]->nPrimes = 0;
			}
			
			return nPrimes;
		}
		
		/** Main Connection Thread. Handles all the networking to allow
			Mining threads the most performance. **/
		void ServerThread()
		{
		
			/** Don't begin until all mining threads are Created. **/
			while(THREADS.size() != nThreads)
				Sleep(1);
				
				
			/** Initialize the Server Connection. **/
			CLIENT = new LLP::Miner(IP, PORT);
				
				
			/** Initialize a Timer for the Hash Meter. **/
			TIMER.Start();
			
			//unsigned int nBestHeight = 0;
			loop
			{
				try
				{
					/** Run this thread at 1 Cycle per Second. **/
					Sleep(1);
					
					
					/** Attempt with best efforts to keep the Connection Alive. **/
					if(!CLIENT->Connected() || CLIENT->Errors() || CLIENT->Timeout(30))
					{
						ResetThreads();
						
						if(!CLIENT->Connect())
							continue;
						else
							CLIENT->Login(ADDRESS);
							
						CLIENT->GetBalance();
						CLIENT->GetPayouts();
					}
					
					/** Rudimentary Meter **/
					if(TIMER.Elapsed() > 10)
					{
						unsigned int SecondsElapsed = (unsigned int)time(0) - nStartTimer;
						unsigned int nElapsed = TIMER.Elapsed();
						double PPS = (double) Primes() / nElapsed;
						printf("[METERS] %f PPS | Height = %u | Balance: %f CSD | Payout: %f CSD | %02d:%02d:%02d\n", PPS, nBestHeight, nAccountBalance / 1000000.0, nCurrentPayout / 1000000.0, (SecondsElapsed/3600)%60, (SecondsElapsed/60)%60, (SecondsElapsed)%60);
						
						CLIENT->Ping();
						TIMER.Reset();
						
						CLIENT->GetBalance();
						CLIENT->GetPayouts();
					}

				
					/** Check if there is work to do for each Miner Thread. **/
					for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
					{
						/** Submit a block from Mining Thread if Flagged. **/
						if(THREADS[nIndex]->fBlockFound && THREADS[nIndex]->fShareResponse)
						{
							CLIENT->SubmitShare(THREADS[nIndex]->hashPrimeOrigin, THREADS[nIndex]->nNonce);
							THREADS[nIndex]->fShareResponse = false;
						}
						
						/** Attempt to get a new block from the Server if Thread needs One. **/
						else if(THREADS[nIndex]->fNewBlock)
						{
							CLIENT->GetBlock();
							THREADS[nIndex]->fBlockWaiting = true;
							THREADS[nIndex]->fNewBlock = false;
							
							printf("[MASTER] Asking For New Block for Thread %u\n", nIndex);
						}
					}
					
					CLIENT->ReadPacket();
					if(!CLIENT->PacketComplete())
						continue;
						
					/** Handle the New Packet, and Interpret its Data. **/
					LLP::Packet PACKET = CLIENT->NewPacket();
					CLIENT->ResetPacket();
							
							
					/** Output if a Share is Accepted. **/
					if(PACKET.HEADER == CLIENT->ACCEPT)
					{
						for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
						{
							if(!THREADS[nIndex]->fShareResponse)
							{
								printf("[MASTER] Share Found | Difficulty %f | Hash %s | Thread %.2i --> [Accepted]\n", GetPrimeDifficulty(CBigNum(THREADS[nIndex]->hashPrimeOrigin + THREADS[nIndex]->nNonce), 1), THREADS[nIndex]->hashPrimeOrigin.ToString().substr(0, 20).c_str(), nIndex);
								THREADS[nIndex]->fBlockFound   = false;
								THREADS[nIndex]->fShareResponse = true;
								
								break;
							}
						}
					}
					
					
					/** Output if a Share is Rejected. **/
					else if(PACKET.HEADER == CLIENT->REJECT) 
					{
						for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
						{
							if(!THREADS[nIndex]->fShareResponse)
							{
								printf("[MASTER] Share Found | Difficulty %f | Hash %s | Thread %.2i --> [Rejected]\n", GetPrimeDifficulty(CBigNum(THREADS[nIndex]->hashPrimeOrigin + THREADS[nIndex]->nNonce), 1), THREADS[nIndex]->hashPrimeOrigin.ToString().substr(0, 20).c_str(), nIndex);
								THREADS[nIndex]->fBlockFound = false;
								THREADS[nIndex]->fShareResponse = true;
								
								break;
							}
						}
					}
					
					/** Output if a Share is a Block **/
					else if(PACKET.HEADER == CLIENT->BLOCK) 
					{
						for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
						{
							if(!THREADS[nIndex]->fShareResponse)
							{
								printf("[MASTER] Block Found | Difficulty %f | Hash %s | Thread %.2i --> [Accepted]\n", GetPrimeDifficulty(CBigNum(THREADS[nIndex]->hashPrimeOrigin + THREADS[nIndex]->nNonce), 1), THREADS[nIndex]->hashPrimeOrigin.ToString().substr(0, 20).c_str(), nIndex);
								THREADS[nIndex]->fBlockFound = false;
								THREADS[nIndex]->fShareResponse = true;
								
								break;
							}
						}
					}
					
					/** Output if a Share is Stale **/
					else if(PACKET.HEADER == CLIENT->STALE) 
					{
						for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
						{
							if(!THREADS[nIndex]->fShareResponse)
							{
								printf("[MASTER] Share Found | Difficulty %f | Hash %s | Thread %.2i --> [Stale Share]\n", GetPrimeDifficulty(CBigNum(THREADS[nIndex]->hashPrimeOrigin + THREADS[nIndex]->nNonce), 1), THREADS[nIndex]->hashPrimeOrigin.ToString().substr(0, 20).c_str(), nIndex);
								THREADS[nIndex]->fBlockFound = false;
								THREADS[nIndex]->fShareResponse = true;
								
								break;
							}
						}
					}
					
					
					/** Reset the Threads if a New Block came in. **/
					else if(PACKET.HEADER == CLIENT->NEW_BLOCK)
					{
						if(nBestHeight > 0)
							printf("[MASTER] Coinshield Network: New Block %u.\n", nBestHeight + 1);
							
						ResetThreads();
					}
					
					/** Set the Current Account Balance if Message Received. **/
					else if(PACKET.HEADER == CLIENT->ACCOUNT_BALANCE) { nAccountBalance = bytes2uint64(PACKET.DATA); }
					
					
					/** Set the Current Pending Payout if Message Received. **/
					else if(PACKET.HEADER == CLIENT->PENDING_PAYOUT) { nCurrentPayout = bytes2uint64(PACKET.DATA); }
					
					
					/** Set the Block for the Thread if there is a New Block Packet. **/
					else if(PACKET.HEADER == CLIENT->BLOCK_DATA)
					{
						/** Search for a Thread waiting for a New Block to Supply its need. **/
						for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
						{
							if(THREADS[nIndex]->fBlockWaiting)
							{
								THREADS[nIndex]->MUTEX.lock();
								THREADS[nIndex]->hashPrimeOrigin.SetBytes(std::vector<unsigned char>  (PACKET.DATA.begin(),   PACKET.DATA.end() - 12));
								THREADS[nIndex]->nMinimumShare = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 12, PACKET.DATA.end() - 8));
								THREADS[nIndex]->nDifficulty   = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 8,  PACKET.DATA.end() - 4));
								THREADS[nIndex]->MUTEX.unlock();
								
								/** Check that the Block Received is not Obsolete, if so request a new one. **/
								unsigned int nHeight = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 4, PACKET.DATA.end()));
								if(nHeight < nBestHeight)
								{
									printf("[MASTER] Received Obsolete Block %u... Requesting New Block.\n", nHeight);
									CLIENT->GetBlock();
									
									break;
								}
								
								/** Set the Best Height for the Block. **/
								if(nHeight > nBestHeight)
									nBestHeight = nHeight;
								
								printf("[MASTER] Block %s Height = %u Received on Thread %u\n", THREADS[nIndex]->hashPrimeOrigin.ToString().substr(0, 20).c_str(), nHeight, nIndex);
								THREADS[nIndex]->fBlockWaiting = false;
								
								break;
							}
						}
					}
					
				}
				catch(std::exception& e)
				{
					printf("%s\n", e.what()); CLIENT = new LLP::Miner(IP, PORT); 
				}
			}
		}
	};
}

int main(int argc, char *argv[])
{

	if(argc < 4)
	{
		printf("Too Few Arguments. The Required Arguments are 'IP PORT ADDRESS'\n");
		printf("Default Arguments are Total Threads = CPU Cores and Connection Timeout = 10 Seconds\n");
		printf("Format for Arguments is 'IP PORT ADDRESS THREADS TIMEOUT'\n");
		
		Sleep(10000);
		
		return 0;
	}
		
	std::string IP = argv[1];
	std::string PORT = argv[2];
	ADDRESS          = argv[3];
	
	int nThreads = GetTotalCores(), nTimeout = 10;
	
	if(argc > 4)
		nThreads = boost::lexical_cast<int>(argv[4]);
	
	if(argc > 5)
		nTimeout = boost::lexical_cast<int>(argv[5]);
	
	printf("Coinshield Prime Pool Miner 1.0.0 - Created by Videlicet - Optimized by Supercomputing\n");
	printf("Using Supplied Account Address %s\n", ADDRESS.c_str());
	printf("Initializing Miner %s:%s Threads = %i Timeout = %i\n", IP.c_str(), PORT.c_str(), nThreads, nTimeout);
	
	Core::InitializePrimes();
	nStartTimer = (unsigned int)time(0);
	
	Core::ServerConnection MINERS(IP, PORT, nThreads, nTimeout);
	loop { Sleep(10); }
	
	return 0;
}
