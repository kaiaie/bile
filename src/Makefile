# $Id: Makefile,v 1.1 2006/05/02 10:10:34 ken Exp $
EXE = Bile.exe
OBJ = Bile.o astring.o BileObj.o Buffer.o Dict.o Expr.o FileHandler.o Func.o \
HtmlHandler.o ImgHandler.o List.o Logging.o memutils.o Ops.o Pair.o path.o \
stringext.o Template.o TextFile.o tokenize.o Type.o Vars.o
LIB = -liberty

all: $(EXE)

$(EXE): $(OBJ)
	gcc -Wall -O2 -o $@ $(OBJ) $(LIB)

%.o: %.c
	gcc -Wall -O2 -c $< -o $@

.PHONY: clean
clean:
	del $(EXE)
	del *.o
