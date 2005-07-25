@ECHO OFF
REM A reminder of which-way-around the arguments of GCC go: build a proper Makefile!
gcc -Wall -O2 -o TplTest TplTest.c Template.c Buffer.c DList.c Util.c -liberty
