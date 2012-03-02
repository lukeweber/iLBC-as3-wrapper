/******************************************************************

	iLBC Speech Coder ANSI-C Source Code

	iLBC_test.c

	Copyright (C) The Internet Society (2004).
	All Rights Reserved.

******************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "iLBC_define.h"
#include "iLBC_encode.h"
#include "iLBC_decode.h"
#include "AS3.h"

/* Runtime statistics */
#include <time.h>

#define ILBCNOOFWORDS_MAX   (NO_OF_BYTES_30MS/2)

/*----------------------------------------------------------------*
*  Encoder interface function
*---------------------------------------------------------------*/

short encode(   /* (o) Number of bytes encoded */
	iLBC_Enc_Inst_t *iLBCenc_inst,
								/* (i/o) Encoder instance */
	short *encoded_data,    /* (o) The encoded bytes */
	short *data                 /* (i) The signal block to encode*/
){
	float block[BLOCKL_MAX];
	int k;

	/* convert signal to float */

	for (k=0; k<iLBCenc_inst->blockl; k++)
		block[k] = (float)data[k];

	/* do the actual encoding */

	iLBC_encode((unsigned char *)encoded_data, block, iLBCenc_inst);


	return (iLBCenc_inst->no_of_bytes);
}

/*----------------------------------------------------------------*
*  Decoder interface function
*---------------------------------------------------------------*/

short decode(       /* (o) Number of decoded samples */
	iLBC_Dec_Inst_t *iLBCdec_inst,  /* (i/o) Decoder instance */
	short *decoded_data,        /* (o) Decoded signal block*/
	short *encoded_data,        /* (i) Encoded bytes */
	short mode                       /* (i) 0=PL, 1=Normal */
){
	int k;
	float decblock[BLOCKL_MAX], dtmp;

	/* check if mode is valid */

	if (mode<0 || mode>1) {
		printf("\nERROR - Wrong mode - 0, 1 allowed\n"); exit(3);}

	/* do actual decoding of block */
	iLBC_decode(decblock, (unsigned char *)encoded_data, iLBCdec_inst, mode);

	/* convert to short */
	for (k=0; k<iLBCdec_inst->blockl; k++){
		dtmp=decblock[k];

		if (dtmp<MIN_SAMPLE)
			dtmp=MIN_SAMPLE;
		else if (dtmp>MAX_SAMPLE)
			dtmp=MAX_SAMPLE;
		decoded_data[k] = (short) dtmp;
	}

	return (iLBCdec_inst->blockl);
}

int readByteArray(void *cookie, char *dst, int size)
{
	return AS3_ByteArray_readBytes(dst, (AS3_Val)cookie, size);
}

int writeByteArray(void *cookie, const char *src, int size)
{
	return AS3_ByteArray_writeBytes((AS3_Val)cookie, (char *)src, size);
}

fpos_t seekByteArray(void *cookie, fpos_t offs, int whence)
{
	return AS3_ByteArray_seek((AS3_Val)cookie, offs, whence);
}

int closeByteArray(void * cookie)
{
	AS3_Val zero = AS3_Int(0);
	/* just reset the position */
	AS3_SetS((AS3_Val)cookie, "position", zero);
	AS3_Release(zero);
	return 0;
}


static void encodeForFlash(void * self, AS3_Val args)
{
	AS3_Val open, progress, complete;
	AS3_Val src, dest;
	int len, srcLen, yieldTicks;
	short raw_data[BLOCKL_MAX], encoded_data[ILBCNOOFWORDS_MAX];

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_Enc_Inst_t Enc_Inst;
	initEncode(&Enc_Inst, 30);
	int i;
	int loops = srcLen / sizeof(short) / Enc_Inst.blockl;
	for(i = 0; i < loops; i++){
		AS3_ByteArray_readBytes(raw_data, src, (Enc_Inst.blockl * sizeof(short)));
		len = encode(&Enc_Inst, encoded_data, raw_data);
		AS3_ByteArray_writeBytes(dest, encoded_data, len);
		if(i % 10 == 0){
			AS3_CallT(progress, NULL, "IntType", i);
			flyield();//yield to main process
		}
	}
}

static void decodeForFlash(void * self, AS3_Val args)
{
	AS3_Val open, progress, complete;
	AS3_Val src, dest;
	int len, srcLen, yieldTicks;
	short encoded_data[ILBCNOOFWORDS_MAX], decoded_data[BLOCKL_MAX];

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_Dec_Inst_t Dec_Inst;
	initDecode(&Dec_Inst, 30, 1);//30ms mode
	
	int i;
	int loops = srcLen / Dec_Inst.no_of_bytes;
	for(i = 0; i < loops; i++){
		AS3_ByteArray_readBytes(encoded_data, src, Dec_Inst.no_of_bytes);
		len=decode(&Dec_Inst, decoded_data, encoded_data, 1);//1 for no packet loss
		AS3_ByteArray_writeBytes(dest, decoded_data, len);
		/* write output file */
		if(i % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", i);
			flyield();//yield to main process
		}
	}
}

int main(int argc, char **argv)
{
	AS3_Val ilbc_lib = AS3_Object("");
	AS3_Val encodeMethod = AS3_FunctionAsync( NULL, (AS3_ThunkProc) encodeForFlash);
	AS3_Val decodeMethod = AS3_FunctionAsync( NULL, (AS3_ThunkProc) decodeForFlash);
	AS3_SetS(ilbc_lib, "encode", encodeMethod);
	AS3_SetS(ilbc_lib, "decode", decodeMethod);
	AS3_Release( encodeMethod );
	AS3_Release( decodeMethod );
	
	//No code below here. AS3_LibInit does not return
	AS3_LibInit( ilbc_lib );
	return 0;
}
