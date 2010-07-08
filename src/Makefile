# $Id: Makefile,v 1.13 2010/07/08 21:04:24 ken Exp $
# :tabSize=4:indentSize=4:folding=indent:
ifeq '$(OS)' 'Windows_NT'
RM = del
EXE = Bile.exe
else
RM = rm
EXE = bile
endif
SRC = Bile.c astring.c Buffer.c Command.c Dict.c Expr.c FileHandler.c Func.c \
HtmlHandler.c ImgHandler.c List.c Logging.c memutils.c Ops.c Pair.c path.c \
stringext.c Template.c TextFile.c tokenize.c Type.c Vars.c ConfigFile.c \
Publication.c Section.c Index.c Story.c Tags.c
OBJ := $(patsubst %.c,%.o,$(SRC))
LIB = -liberty -lm
#OPT = -Wall -O2
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
	$(RM) *.d
