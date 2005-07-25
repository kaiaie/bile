@ECHO OFF
REM A reminder of which-way-around the arguments of GCC go: build a proper Makefile!
gcc -Wall -O2 -o toktest TokTest.c Tokenizer.c Buffer.c Util.c
