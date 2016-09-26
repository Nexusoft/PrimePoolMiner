#include "core.h"

#pragma GCC optimize ("unroll-loops")

unsigned int nBestHeight = 0;
unsigned int nStartTimer = 0;
uint64 nWeight = 0, nElements = 0, nPrimes = 0;
double nAverage = 0;
unsigned int nLargestShare = 0;
unsigned int nDifficulty   = 0;

std::string ADDRESS;
uint64 nCurrentPayout = 0, nAccountBalance = 0;

static LLP::Timer cPrimeTimer;

namespace Core
{

	/** Main Miner Thread. Bound to the class with boost. Might take some rearranging to get working with OpenCL. **/
	void MinerThread::PrimeMiner()
	{
		loop
		{
			try
			{
				/** Keep thread at idle CPU usage if waiting to submit or recieve block. **/
				Sleep(1);
				
				
				/** Assure that this thread stays idle when waiting for new block, or share submission. **/
				if(fNewBlock || fBlockWaiting || !fNewBlockRestart)
					continue;
				
                {
                    LOCK(MUTEX);
                    fNewBlockRestart = false;
                }
                
				/** Lock the Thread at this Mutex when Changing Block Pointer. **/
				CBigNum BaseHash(hashPrimeOrigin);
				mpz_t zPrimeOrigin, zPrimeOriginOffset, zFirstSieveElement, zPrimorialMod, zTempVar, zResidue, zTwo, zN, zOctuplet;
						
				unsigned int i = 0;
				unsigned int j = 0;
				unsigned int nSize = 0;
				unsigned int nPrimeCount = 0;
				unsigned int nSieveDifficulty = 0;
				uint64 nStart = 0;
				uint64 nStop = 0;
				unsigned int nLastOffset = 0;
				
				#if defined _WIN32 || defined WIN32
					uint64 nNonce = 0;
				#else
					unsigned long nNonce = 0;
				#endif
				
				long nElapsedTime = 0;
				long nStartTime = 0;
				mpz_init(zPrimeOriginOffset);
				mpz_init(zFirstSieveElement);
				mpz_init(zPrimorialMod);
				mpz_init(zOctuplet);
				mpz_init(zTempVar);
				mpz_init(zPrimeOrigin);
				mpz_init(zResidue);
				mpz_init_set_ui(zTwo, 2);
				mpz_init(zN);

				bignum2mpz(&BaseHash, zPrimeOrigin);
				
				nSize = mpz_sizeinbase(zPrimeOrigin, 2);
				unsigned char bit_array_sieve[nBitArray_Size/8];
				
				for(j=0; j<256 && !fNewBlockRestart; j++)
				{
					memset(bit_array_sieve, 0x00, (nBitArray_Size)/8);

					mpz_mod(zPrimorialMod, zPrimeOrigin, zPrimorial);
					mpz_sub(zPrimorialMod, zPrimorial, zPrimorialMod);

					mpz_mod(zPrimorialMod, zPrimorialMod, zPrimorial);
					
					#if defined _WIN32 || defined WIN32
						mpz_import(zOctuplet, 1, 1, sizeof(octuplet_origins[j]), 0, 0, &octuplet_origins[j]);
						mpz_add(zPrimorialMod, zPrimorialMod, zOctuplet);
					#else
						mpz_add_ui(zPrimorialMod, zPrimorialMod, octuplet_origins[j]);
					#endif

					
					mpz_add(zTempVar, zPrimeOrigin, zPrimorialMod);
					mpz_set(zFirstSieveElement, zTempVar);

					for(unsigned int i=nPrimorialEndPrime; i<nPrimeLimit && !fNewBlockRestart; i++)
					{
						unsigned long  p = primes[i];
						unsigned int inv = inverses[i];
						unsigned int base_remainder = mpz_tdiv_ui(zTempVar, p);

						unsigned int remainder = base_remainder;
						unsigned long r = (p-remainder)*inv;
						unsigned int index = r % p;
						while(index < nBitArray_Size)
						{
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
							bit_array_sieve[(index)>>3] |= (1<<((index)&7));
							index += p;
						}


						remainder = base_remainder + 8;
						if (p<remainder)
							remainder -= p;
						r = (p - remainder) * inv;
						index = r % p;
						while(index < nBitArray_Size)
						{
								bit_array_sieve[(index)>>3] |= (1<<((index)&7));
							index += p;
						}
						
						remainder = base_remainder + 12;
						if (p<remainder)
							remainder -= p;
						r = (p - remainder) * inv;
						index = r % p;
						while(index < nBitArray_Size)
						{
							bit_array_sieve[(index)>>3] |= (1<<((index)&7));
							index += p;
						}
					}

					
					for(i=0; i<nBitArray_Size && !fNewBlockRestart; i++)
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

						nStart = 2;
						nStop = 2;
						nPrimeCount = 1;
						nLastOffset = 2;
						cPrimeTimer.Reset();
						nPrimes+=2;

                         mpz_add_ui(zTempVar, zTempVar, 2);
						for(nStart; nStart <= nStop + 12 && !fNewBlockRestart; nStart += 2)
						{
							mpz_sub_ui(zN, zTempVar, 1);
							mpz_powm(zResidue, zTwo, zN, zTempVar);
							if (mpz_cmp_ui(zResidue, 1) == 0)
							{
								nStop = nStart;
								nPrimeCount++;
								nPrimes++;
								//nPrimesForSieve++;
							}

							mpz_add_ui(zTempVar, zTempVar, 2);
							
							nLastOffset += 2;
						}

						if(nPrimeCount >= 4 && !fNewBlockRestart)
						{   
							mpz_sub(zTempVar, zPrimeOriginOffset, zPrimeOrigin);
							
							#if defined _WIN32 || defined WIN32
								nNonce = mpz2uint64(zTempVar);
							#else
								nNonce = mpz_get_ui(zTempVar);
							#endif
													
							unsigned int nSieveBits = SetBits(GetSieveDifficulty(BaseHash + nNonce + nLastOffset, nPrimeCount));
							if(nSieveBits > nLargestShare)
								nLargestShare = nSieveBits;

							if(nSieveBits < nMinimumShare)
								continue;
							
							unsigned int nDiff = GetPrimeBits((BaseHash + nNonce), 1);
							if(nDiff >= nMinimumShare)
							{
								nWeight += nDiff * 50;
								cServerConnection->SubmitShare(BaseHash.getuint1024(), nNonce);
							}
						}
					}
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

                if( !fNewBlockRestart && !fBlockWaiting )
				    fNewBlock = true;
			}
			catch(std::exception& e){ printf("ERROR: %s\n", e.what()); }
		}
	}
	

	/** Reset the block on each of the Threads. **/
	void ServerConnection::ResetThreads()
	{
		/** Clear the Submit Queue. **/
		SUBMIT_MUTEX.lock();
		
		while(!SUBMIT_QUEUE.empty())
			SUBMIT_QUEUE.pop();
			
		SUBMIT_MUTEX.unlock();
		
		while(!RESPONSE_QUEUE.empty())
			RESPONSE_QUEUE.pop();
		
		/** Reset each individual flag to tell threads to stop mining. **/
		for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
        {
            LOCK(THREADS[nIndex]->MUTEX);
			THREADS[nIndex]->fNewBlock      = true;		
            THREADS[nIndex]->fNewBlockRestart = true;	
        }
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
				/** Run this thread at 100 Cycles per Second. **/
				Sleep(10);
					
					
				/** Attempt with best efforts to keep the Connection Alive. **/
				if(!CLIENT->Connected() || CLIENT->Errors() || CLIENT->Timeout(30))
				{
					if(!CLIENT->Connect())
						continue;
					else
					{
						CLIENT->Login(ADDRESS);
							
						CLIENT->GetBalance();
						CLIENT->GetPayouts();
						
						ResetThreads();
					}
				}
					
				/** Rudimentary Meter **/
				if(TIMER.Elapsed() > 10)
				{
				
					nElements++;
					
					unsigned int SecondsElapsed = (unsigned int)time(0) - nStartTimer;
					unsigned int nElapsed = TIMER.Elapsed();
					
					double PPS = (double) nPrimes / (nElapsed * nElements);
					double WPS = nWeight / (nElapsed * nElements * 10000000);

					printf("[METERS] %f PPS | %f WPS | Largest Share %f | Difficulty %f | Height = %u | Balance: %f NXS | Payout: %f NXS | %02d:%02d:%02d\n", PPS, WPS, nLargestShare / 10000000.0, nDifficulty / 10000000.0, nBestHeight, nAccountBalance / 1000000.0, nCurrentPayout / 1000000.0, (SecondsElapsed/3600)%60, (SecondsElapsed/60)%60, (SecondsElapsed)%60);
						
					CLIENT->Ping();
					TIMER.Reset();
				}
					
					
				/** Submit any Shares from the Mining Threads. **/
				SUBMIT_MUTEX.lock();
				while(!SUBMIT_QUEUE.empty())
				{
					std::pair<uint1024, uint64> pShare = SUBMIT_QUEUE.front();
					SUBMIT_QUEUE.pop();
					
					CLIENT->SubmitShare(pShare.first, pShare.second);
					RESPONSE_QUEUE.push(pShare);
				}
				SUBMIT_MUTEX.unlock();
				
				
				/** Check if there is work to do for each Miner Thread. **/
				for(int nIndex = 0; nIndex < THREADS.size(); nIndex++)
				{
					/** Attempt to get a new block from the Server if Thread needs One. **/
					if(THREADS[nIndex]->fNewBlock)
					{
						CLIENT->GetBlock();
						THREADS[nIndex]->fBlockWaiting = true;
						THREADS[nIndex]->fNewBlock = false;
                        THREADS[nIndex]->fNewBlockRestart = true;
						
						//printf("[MASTER] Asking For New Block for Thread %u\n", nIndex);
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
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					double nDiff = GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1);
					printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Accepted]\n", nDiff, pResponse.first.ToString().substr(0, 20).c_str());
				}
					
					
				/** Output if a Share is Rejected. **/
				else if(PACKET.HEADER == CLIENT->REJECT) 
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Rejected]\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().substr(0, 20).c_str());
				}
					
				/** Output if a Share is a Block **/
				else if(PACKET.HEADER == CLIENT->BLOCK) 
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					printf("\n******************************************************\n\nBlock Accepted | Difficulty %f | Hash %s\n\n******************************************************\n\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().c_str());
				}
					
				/** Output if a Share is Stale **/
				else if(PACKET.HEADER == CLIENT->STALE) 
				{
					if(RESPONSE_QUEUE.empty())
						continue;
						
					std::pair<uint1024, uint64> pResponse = RESPONSE_QUEUE.front();
					RESPONSE_QUEUE.pop();
					
					printf("[MASTER] Share Found | Difficulty %f | Hash %s  --> [Stale]\n", GetPrimeDifficulty(CBigNum(pResponse.first + pResponse.second), 1), pResponse.first.ToString().substr(0, 20).c_str());
				}
					
					
				/** Reset the Threads if a New Block came in. **/
				else if(PACKET.HEADER == CLIENT->NEW_BLOCK)
				{
					printf("[MASTER] NXS Network: New Block\n");
							
					ResetThreads();
					
					CLIENT->GetBalance();
					CLIENT->GetPayouts();
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
							LOCK(THREADS[nIndex]->MUTEX);
							
							THREADS[nIndex]->hashPrimeOrigin.SetBytes(std::vector<unsigned char>  (PACKET.DATA.begin(),   PACKET.DATA.end() - 12));
							THREADS[nIndex]->nMinimumShare = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 12, PACKET.DATA.end() - 8));
		
								
							/** Check that the Block Received is not Obsolete, if so request a new one. **/
							unsigned int nHeight = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 4, PACKET.DATA.end()));
							if(nHeight < nBestHeight)
							{
								printf("[MASTER] Received Obsolete Block %u... Requesting New Block.\n", nHeight);
								CLIENT->GetBlock();
									
								break;
							}
							
							nDifficulty   = bytes2uint(std::vector<unsigned char>(PACKET.DATA.end() - 8,  PACKET.DATA.end() - 4));
							nBestHeight   = nHeight;
								
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
