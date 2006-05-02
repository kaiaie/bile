# $Id: Makefile,v 1.2 2006/05/02 23:10:07 ken Exp $
EXE = Bile.exe
OBJ = Bile.o astring.o BileObj.o Buffer.o Dict.o Expr.o FileHandler.o Func.o \
HtmlHandler.o ImgHandler.o List.o Logging.o memutils.o Ops.o Pair.o path.o \
stringext.o Template.o TextFile.o tokenize.o Type.o Vars.o
LIB = -liberty
OPT = -Wall -g

all: $(EXE)

$(EXE): $(OBJ)
	gcc $(OPT) -o $@ $(OBJ) $(LIB)

%.o: %.c
	gcc $(OPT) -c $< -o $@

.PHONY: clean
clean:
	del $(EXE)
	del *.o
