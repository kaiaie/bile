@ECHO OFF
REM A reminder of which-way-around the arguments of GCC go: build a proper Makefile!
gcc -Wall -O2 -o VarsTest VarsTest.c Vars.c Dict.c List.c Pair.c util.c
