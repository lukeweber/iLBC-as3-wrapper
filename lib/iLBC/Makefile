CC = gcc
ARCH=$(PROC)
CFLAGS+= -Wall
CC_SWITCHES = -Wall

PNAME = iLBC

SOURCES=\
	packing.c\
	LPCencode.c\
	filter.c\
	doCPLC.c\
	constants.c\
	helpfun.c\
	StateConstructW.c\
	syntFilter.c\
	StateSearchW.c\
	LPCdecode.c\
	FrameClassify.c\
	anaFilter.c\
	enhancer.c\
	iCBConstruct.c\
	hpInput.c\
	gainquant.c\
	iLBC_decode.c\
	iCBSearch.c\
	getCBvec.c\
	createCB.c\
	hpOutput.c\
	lsf.c\
	iLBC_encode.c

MAIN = iLBC_as3_wrapper.c
TEST = iLBC_test.c

MAIN_OBJ = $(MAIN:.c=.o)
TEST_OBJ = $(TEST:.c=.o)
OBJ = $(SOURCES:.c=.o)
	
compile_swc: $(OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -swc -o $(PNAME).swc $(OBJ) $(MAIN_OBJ) -lm
	mv $(PNAME).swc ../../bin/
	
test: $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $(PNAME)_test.a $(OBJ) $(TEST_OBJ) -lm

%.o: %.c
	$(CC) $(CC_SWITCHES) -c $< -o $@

clean:
	rm -f *.o core *.bak *.a *.exe *.exe.bc *.l.bc *.achacks.* *.stackdump *.swc

swc: compile_swc clean