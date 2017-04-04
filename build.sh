#!/bin/bash
cd /cygdrive/c/GitHub/hg5fm/PrimePoolMiner

if test "$1" = 'REBUILD'; then
make -f makefile clean
fi

make -f makefile -j4

if test "$1" != 'DEBUG' && test "$2" != 'DEBUG'; then
strip nexus_cpuminer.exe  
fi
