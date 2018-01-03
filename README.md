# Nexus CPU Pool/Solo Miner

# Build instructions (Ubuntu)
### Install dependencies

```sh
sudo apt-get install build-essential libboost-all-dev libdb-dev libdb++-dev libssl-dev libminiupnpc-dev libgmp-dev
```

### Clone the repo

```sh
cd ~
git clone https://github.com/hg5fm/PrimePoolMiner.git PrimePoolMiner
```

### Build the miner

```sh
cd PrimePoolMiner
make MARCHFLAGS=-march=native -f makefile
```

### miner.conf
The pool miner can now be configured via miner.conf.  Two example files miner.conf.* are included.  
Please copy/rename them to miner.conf in order to use it .
 
You can adjust the parameters used to initialise the prime sieving / wheel factorisation to optimise the prime chain rates (or WPS) for your CPU. 

	"host": <the pool hostname / ipaddress to connect to, in solo mode this should be the address of the NSX wallet node, quotation marks needed> 
	"port": <the pool port, default 9549 for pool, for solo mining the default is 9325, quotation marks needed>
	"nxs_address": <your payout address - if it's left empty ("") the miner will try to work in SOLO mode, quotation marks needed>
	"sieve_threads": <number of threads to use for sieving, default is 0 (all available cores)
	"ptest_threads": <number of threads to use for primality testing, default is 0 (all available cores)
	"timeout": <timeout when connecting to pool, default 10s>
	"bit_array_size": <the size of the prime sieve in bytes, default 8388608. Adjust this to suit your CPU cache size> 
	"prime_limit": <max prime number used to initialise the sieve, default 71378571 - not in use at the current stage>
	"n_prime_limit": <max inverses prime limit, default 4194304 - not in use at the current stage>
	"primorial_end_prime": <largest primorial, default 12>

### Run the miner with miner.conf
```
./nexus_cpuminer
```

### Run the miner without miner.conf
The Required Arguments are 'IP PORT ADDRESS'
Default Arguments are Total Threads = CPU Cores and Connection Timeout = 10 Seconds
Format for Arguments is 'IP PORT ADDRESS SIEVE-THREADS PRIMETEST-THREADS TIMEOUT'
Solo mining example:
```
./nexus_cpuminer localhost 9325 ""
```
Pool mining example:
```
./nexus_cpuminer nexusminingpool.com 9549 <your-own-nexus-address>
```
