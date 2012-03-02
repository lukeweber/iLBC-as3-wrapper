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

OBJ = $(SOURCES:.c=.o) $(MAIN:.c=.o)

swc: $(OBJ)
	$(CC) $(CFLAGS) -swc -o $(PNAME).swc $(OBJ) -lm

%.o: %.c
	$(CC) $(CC_SWITCHES) -c $< -o $@

clean:
	rm -f $(OBJ) core *.bak *.o *.exe *.exe.bc *.l.bc *.achacks.* *.stackdump *.swc

install:
