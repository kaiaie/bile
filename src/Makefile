# $Id: Makefile,v 1.4 2006/05/16 13:30:01 ken Exp $
RM = del
EXE = Bile.exe
SRC = Bile.c astring.c BileObj.c Buffer.c Command.c Dict.c Expr.c FileHandler.c Func.c \
HtmlHandler.c ImgHandler.c List.c Logging.c memutils.c Ops.c Pair.c path.c \
stringext.c Template.c TextFile.c tokenize.c Type.c Vars.c
OBJ := $(patsubst %.c,%.o,$(SRC))
LIB = -liberty
OPT = -Wall -g

.PHONY: all
all: $(EXE)

$(EXE): $(OBJ)
	gcc $(OPT) -o $@ $(OBJ) $(LIB)

include $(OBJ:.o=.d)

%.d: %.c
	gcc -MM -MG $< > $@

%.o: %.c
	gcc $(OPT) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(EXE)
	$(RM) *.o
