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
	int len, srcLen, bytesRemaining, yieldTicks, samples;
	short mode = 30;//30ms
	short raw_data[BLOCKL_MAX], encoded_data[NO_OF_BYTES_30MS];
	
	//Base 64 logic
	char base64_data[BLOCKL_MAX * 2];
	base64_encodestate state;
	base64_init_encodestate(&state);

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_encinst_t *Enc_Inst;
	WebRtcIlbcfix_EncoderCreate(&Enc_Inst);
	WebRtcIlbcfix_EncoderInit(Enc_Inst, mode);
	
	bytesRemaining = srcLen;
	resetPositionByteArray(src);
	int i;
	for (i = 0; bytesRemaining > 0; i++){
		bytesRemaining -= AS3_ByteArray_readBytes(raw_data, src,(mode<<3) * sizeof(short));
		samples = WebRtcIlbcfix_Encode(Enc_Inst, raw_data, (short)(mode<<3), encoded_data);
		
		//Base 64
		len = base64_encode_block((char *)encoded_data, samples, base64_data, &state);
		AS3_ByteArray_writeBytes(dest, base64_data, len);
		
		if(i % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)bytesRemaining / srcLen)) * 100));
			flyield();//yield to main process
		}
	}

	//Base 64
	len = base64_encode_blockend(base64_data, &state);
	AS3_ByteArray_writeBytes(dest, base64_data, len);
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
	int len, srcLen, yieldTicks, samples, bytesRead, bytesRemaining;
	short decoded_data[BLOCKL_MAX], speechType;
	short mode = 30;//30 ms
	
	//Base 64 logic
	char base64_data[200];
	char base64_decoded[150];
	base64_decodestate state;
	base64_init_decodestate(&state);
	
	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_decinst_t *Dec_Inst;
	WebRtcIlbcfix_DecoderCreate(&Dec_Inst);
	WebRtcIlbcfix_DecoderInit(Dec_Inst, mode);

	resetPositionByteArray(src);
	
	//base 64 logic
	bytesRemaining = srcLen;
	
	int i;
	for (i = 0; bytesRemaining > 0; i++){
		bytesRead = AS3_ByteArray_readBytes(base64_data, src, 200);//200 base64 bytes will yield 150 bytes or 3 chunks of NO_OF_BYTES_30MS(50)
		bytesRemaining -= bytesRead;
		len = base64_decode_block(base64_data, bytesRead, base64_decoded, &state);
		int j;
		if(bytesRead % NO_OF_BYTES_30MS == 0){
			for(j = 0; j < bytesRead; j+= NO_OF_BYTES_30MS){
				samples = WebRtcIlbcfix_Decode(Dec_Inst, (short *)&base64_decoded[j], NO_OF_BYTES_30MS, decoded_data, &speechType);
				AS3_ByteArray_writeBytes(dest, decoded_data, samples * sizeof(short));
				if(i % yieldTicks == 0){
					AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)bytesRemaining / srcLen)) * 100));
					flyield();//yield to main process
				};
			}
		}
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
