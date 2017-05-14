@echo off
set CYGWINBIN=C:\cygwin64_Test\bin\
set PATH=%CYGWINBIN%;%PATH%
%CYGWINBIN%bash.exe /cygdrive/c/GitHub/hg5fm/PrimePoolMiner/build-cygwin.sh %1 %2 %3 %4
echo Done!!!
pause