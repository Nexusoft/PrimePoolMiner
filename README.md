# Nexus (Coinshield) CPU Pool Miner: This is the enhanced and customized CPU Pool miner.

- [x] Add (basic) config file functionality
- [ ] Address all the build warnings
- [ ] Improve command line operation
- [ ] Profiling and perforfance tweaking
- [x] Implement new upstream protocols
- [ ] Multi Pool Switching
- [ ] Interface Improvement - Colour!

# Build instructions (Ubuntu)
### Install dependencies

```sh
sudo apt-get install build-essential libboost-all-dev libdb-dev libdb++-dev libssl-dev libminiupnpc-dev libgmp-dev
```

### Clone the repo

```sh
cd ~
git clone https://github.com/Nexusoft/PrimePoolMiner.git PrimePoolMiner
```

### Build the miner

```sh
cd PrimePoolMiner
make MARCHFLAGS=-march=native -f makefile.unix
```

### miner.conf
The pool miner can now be configured via miner.conf.  An example file miner.conf.example is included.  
Please copy/rename it to miner.conf in order to use it .
 
You can adjust the parameters used to initialise the prime sieving / wheel factorisation to optimise the PPS/WPS for your CPU. 

	"host": <the pool hostname / ipaddress to connect to> 
	"port": <the pool port, default 9549>
	"nxs_address": <your payout address>
    "threads": <number of cores/threads to use, default is 0 (all available cores)
	"timeout": <timeout when connecting to pool, default 10s>
	"bit_array_size": <the size of the prime sieve in bytes, default 37748736. Adjust this to suit your CPU cache size> 
	"prime_limit": <max prime number used to initialise the sieve, default 71378571>
	"n_prime_limit": <max inverses prime limit, default 4194304>
	"primorial_end_prime": <largest primorial, default 12>

### Run the miner
./nexus_cpuminer 

# Original Message: 

This is the Miner for NXS Pools. Start it with commandline arguments IP PORT ADDRESS. Optional arguments are THREADS and TIMEOUT following ADDRESS.

Viz.
