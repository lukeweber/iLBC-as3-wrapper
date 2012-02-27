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

	iLBC_decode(decblock, (unsigned char *)encoded_data,
		iLBCdec_inst, mode);

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


// ALCHEMY UTILS
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

static AS3_Val encodeForFlash(void * self, AS3_Val args)
{
	void * ref;
	void * src;
	void * dest;
	FILE *input;
	FILE *output;
	short data[BLOCKL_MAX];
	int len;
    short encoded_data[ILBCNOOFWORDS_MAX];

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType", &ref, &src, &dest);

	//flashErrorsRef = (AS3_Val)ref;
	
	input	= funopen((void *)src, readByteArray, writeByteArray, seekByteArray, closeByteArray);
	output	= funopen((void *)dest, readByteArray, writeByteArray, seekByteArray, closeByteArray);
	
	if (input == NULL || output == NULL) {
		ERROR("Unable to set bytes arrays");
	}
	
	iLBC_Enc_Inst_t Enc_Inst;
	initEncode(&Enc_Inst, 30);
	while (fread(data,sizeof(short),Enc_Inst.blockl,input)== Enc_Inst.blockl) {
		len=encode(&Enc_Inst, encoded_data, data);
		fwrite(encoded_data, sizeof(unsigned char), len, output);
	}
	return AS3_Int(1);
}

static AS3_Val decodeForFlash(void * self, AS3_Val args)
{
	void * ref;
	void * src;
	void * dest;
	FILE *input;
	FILE *output;
	short data[BLOCKL_MAX];
	int len;
	short decoded_data[BLOCKL_MAX];

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType", &ref, &src, &dest);

	//flashErrorsRef = (AS3_Val)ref;
	
	input	= funopen((void *)src, readByteArray, writeByteArray, seekByteArray, closeByteArray);
	output	= funopen((void *)dest, readByteArray, writeByteArray, seekByteArray, closeByteArray);
	
	if (input == NULL || output == NULL) {
		ERROR("Unable to set bytes arrays");
	}

	iLBC_Dec_Inst_t Dec_Inst;
	initDecode(&Dec_Inst, 30, 1);//30ms mode
	while (fread(data,sizeof(short),Dec_Inst.blockl,input)== Dec_Inst.blockl) {
		len=decode(&Dec_Inst, decoded_data, data, 1);//1 for no packet loss
		/* write output file */
		fwrite(decoded_data,sizeof(short),len,output);
	}
	return AS3_Int(1);
}

int main(int argc, char **argv)
{
	AS3_Val encodeMethod = AS3_Function( NULL, encodeForFlash);
	AS3_Val decodeMethod = AS3_Function( NULL, decodeForFlash);

	AS3_Val result = AS3_Object("encode:AS3ValType, decode:AS3ValType", encodeMethod, decodeMethod);

	AS3_Release( encodeMethod );
	AS3_Release( decodeMethod );
	
	AS3_LibInit( result );
	return 0;
}
