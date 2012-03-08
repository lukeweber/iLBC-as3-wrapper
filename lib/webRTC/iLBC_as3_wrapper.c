/******************************************************************

	iLBC Speech Coder as3 wrapper using Adobe Alchemy

	iLBC_as3_wrapper.c

******************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ilbc.h"
#include "defines.h"
#include "encode.h"
#include "decode.h"
#include "AS3.h"
#include "b64/cencode.h"
#include "b64/cdecode.h"

int resetPositionByteArray(AS3_Val byteArray)
{
	AS3_Val zero = AS3_Int(0);
	/* just reset the position */
	AS3_SetS((AS3_Val)byteArray, "position", zero);
	AS3_Release(zero);
	return 0;
}

static void encodeForFlash(void * self, AS3_Val args)
{
	AS3_Val progress;
	AS3_Val src, dest;
	int len, srcLen, remainingBytes, yieldTicks, samples;
	short mode = 30;//30ms
	short raw_data[BLOCKL_MAX], encoded_data[NO_OF_BYTES_30MS];
	
	//Base 64 logic
	char base64data[BLOCKL_MAX * 2];
	base64_encodestate state;
	base64_init_encodestate(&state);

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_encinst_t *Enc_Inst;
	WebRtcIlbcfix_EncoderCreate(&Enc_Inst);
	WebRtcIlbcfix_EncoderInit(Enc_Inst, mode);
	remainingBytes = srcLen;
	int i = 0;
	resetPositionByteArray(src);
	while (remainingBytes > 0){
		remainingBytes -= AS3_ByteArray_readBytes(raw_data, src,(mode<<3) * sizeof(short));
		samples = WebRtcIlbcfix_Encode(Enc_Inst, raw_data, (short)(mode<<3), encoded_data);
		
		//Base 64
		len = base64_encode_block((char *)encoded_data, samples, base64data, &state);
		AS3_ByteArray_writeBytes(dest, base64data, len);
		
		//AS3_ByteArray_writeBytes(dest, encoded_data, len);
		if(i % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)remainingBytes / srcLen)) * 100));
			flyield();//yield to main process
		}
		i++;
	}

	//Base 64
	len = base64_encode_blockend(base64data, &state);
	AS3_ByteArray_writeBytes(dest, base64data, len);
	//free state?
	
	resetPositionByteArray(src);
	resetPositionByteArray(dest);
	
	WebRtcIlbcfix_EncoderFree(Enc_Inst);
	// Don't remove progess 100 call here, else complete won't be called!
	AS3_CallT(progress, NULL, "IntType", 100);
}

static void decodeForFlash(void * self, AS3_Val args)
{
	AS3_Val progress;
	AS3_Val src, dest;
	int len, srcLen, yieldTicks;
	short encoded_data[NO_OF_WORDS_30MS], decoded_data[BLOCKL_MAX], speechType;
	short mode = 30;//30 ms
	
	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_decinst_t *Dec_Inst;
	WebRtcIlbcfix_DecoderCreate(&Dec_Inst);
	WebRtcIlbcfix_DecoderInit(Dec_Inst, mode);
	
	int i = 0;
	int loops = srcLen / NO_OF_BYTES_30MS;
	resetPositionByteArray(src);
	while(AS3_ByteArray_readBytes(encoded_data, src, NO_OF_BYTES_30MS) == NO_OF_BYTES_30MS){
		samples = WebRtcIlbcfix_Decode(Dec_Inst, encoded_data, NO_OF_BYTES_30MS, decoded_data, &speechType);
		AS3_ByteArray_writeBytes(dest, decoded_data, samples * sizeof(short));
		if(i % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", (int)((float)i / loops * 100));
			flyield();//yield to main process
		}
		i++;
	}
	
	resetPositionByteArray(src);
	resetPositionByteArray(dest);
	WebRtcIlbcfix_DecoderFree(Dec_Inst);
	// Don't remove progess 100 call here, else complete won't be called!
	AS3_CallT(progress, NULL, "IntType", 100);
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
