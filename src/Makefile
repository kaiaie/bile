ifeq '$(OS)' 'Windows_NT'
RM = del
EXE = Bile.exe
else
RM = rm -f
EXE = bile
endif
SRC = Bile.c astring.c Buffer.c Command.c Dict.c Expr.c FileHandler.c Func.c \
HtmlHandler.c ImgHandler.c List.c Logging.c memutils.c Ops.c Pair.c path.c \
stringext.c Template.c TextFile.c tokenize.c Type.c Vars.c ConfigFile.c \
Publication.c Section.c Index.c Story.c Tags.c
ifeq '$(OS)' 'Windows_NT'
RC = BileResources.rc
OBJ := $(patsubst %.c,%.o,$(SRC)) $(patsubst %.rc,%.o,$(RC))
NSI = Bile.nsi
SETUP = setup.exe
else
OBJ := $(patsubst %.c,%.o,$(SRC))
endif
LIB = -lm
# Uncomment for release build
OPT = -Wall -O2
# Uncomment for test build
#OPT = -Wall -g

.PHONY: all
all: $(SETUP) $(EXE)

ifeq '$(OS)' 'Windows_NT'
$(SETUP): $(NSI) $(EXE)
	makensis /V2 $(NSI)
endif

$(EXE): $(OBJ)
	gcc $(OPT) -o $@ $(OBJ) $(LIB)

-include $(SRC:.c=.d)

%.d: %.c
	gcc -MM -MG $< > $@

%.o: %.c
	gcc $(OPT) -c $< -o $@

%.o: %.rc
	windres -i $< -o $@

.PHONY: clean
clean:
	$(RM) $(EXE) $(SETUP)
	$(RM) *.o
	$(RM) *.d
