# Nexus (Coinshield) CPU Pool Miner: This is the enhanced and customized CPU Pool miner.

- [x] Add (basic) config file functionality
- [ ] Address all the build warnings
- [ ] Improve command line operation
- [ ] Profiling and perforfance tweaking
- [ ] Implement new upstream protocols
- [ ] Multi Pool Switching
- [ ] Interface Improvement - Colour!

# Build instructions (Ubuntu)
### Install dependencies

```sh
sudo apt-get install build-essential libboost-all-dev install libdb-dev libdb++-dev libssl-dev libminiupnpc-dev libqrencode-dev qt4-qmake libqt4-dev libgmp3-dev
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

# Original Message: 

This is the Miner for NXS Pools. Start it with commandline arguments IP PORT ADDRESS. Optional arguments are THREADS and TIMEOUT following ADDRESS.

Viz.
