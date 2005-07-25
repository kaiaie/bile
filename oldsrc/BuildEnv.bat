@ECHO OFF
REM A reminder of which-way-around the arguments of GCC go: build a proper Makefile!
gcc -Wall -O2 -o envtest EnvTest.c Dict.c Env.c Util.c VarList.c
