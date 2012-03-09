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

#ifdef USE_BASE64
#include "b64/cencode.h"
#include "b64/cdecode.h"
#endif

int resetPositionByteArray(AS3_Val byteArray)
{
	AS3_Val zero = AS3_Int(0);
	/* just reset the position */
	AS3_SetS((AS3_Val)byteArray, "position", zero);
	AS3_Release(zero);
	return 0;
}

float calculateStep(short a, short b, int samples){
	return (float)(b - a) / samples;
}

/**
 * 
 * @return count of samples in output
 */
int upsample8to44(short* input, short* output, int samples){
	int i, j, position, loop_samples;
	float step;
	position = 0;
	for (i = 0; i < samples - 1; i++) {
		loop_samples = 5 + (i % 2);//Switch between 5 or 6 to convert 8k to 44k
		step = calculateStep(input[i], input[i+1], loop_samples);
		for (j = 0; j < loop_samples; j++){
			output[position++] = (short)(step * j) + input[i];
		}
	}
	output[position] = input[samples-1];
	
	return position+1;
}

static void encodeForFlash(void * self, AS3_Val args)
{
	AS3_Val progress;
	AS3_Val src, dest;
	int len, srcLen, bytesRemaining, yieldTicks, samples;
	short mode = 30;//30ms
	short raw_data[BLOCKL_MAX], encoded_data[NO_OF_BYTES_30MS];
	
#ifdef USE_BASE64
	char base64_data[BLOCKL_MAX * 2];
	base64_encodestate state;
	base64_init_encodestate(&state);
#endif

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
		
#ifdef USE_BASE64
		len = base64_encode_block((char *)encoded_data, samples, base64_data, &state);
		AS3_ByteArray_writeBytes(dest, base64_data, len);
#else
		AS3_ByteArray_writeBytes(dest, encoded_data, len);
#endif
	
		if(i % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)bytesRemaining / srcLen)) * 100));
			flyield();//yield to main process
		}
	}

#ifdef USE_BASE64
	len = base64_encode_blockend(base64_data, &state);
	AS3_ByteArray_writeBytes(dest, base64_data, len);
#endif

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
	int len, srcLen, yieldTicks, samples, bytesRemaining;
	short decoded_data[BLOCKL_MAX], speechType;
	short mode = 30;//30 ms
	short upsampled_data[BLOCKL_MAX * 6];//Really 5.5
	
#ifdef USE_BASE64
	char raw_data[200];//200 base64 bytes will yield 150 bytes or 3 chunks of NO_OF_BYTES_30MS(50)
	char base64_decoded[150];
	base64_decodestate state;
	base64_init_decodestate(&state);
#else
	char raw_data[NO_OF_BYTES_30MS]
#endif
	
	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	iLBC_decinst_t *Dec_Inst;
	WebRtcIlbcfix_DecoderCreate(&Dec_Inst);
	WebRtcIlbcfix_DecoderInit(Dec_Inst, mode);

	resetPositionByteArray(src);
	bytesRemaining = srcLen;
	
#ifdef USE_BASE64
	int i, j, k;
	k = 0;
	for (i = 0; bytesRemaining > 0; i++){
		len = AS3_ByteArray_readBytes(raw_data, src, 200);
		bytesRemaining -= len;
		len = base64_decode_block(raw_data, len, base64_decoded, &state);
		if(len % NO_OF_BYTES_30MS == 0){
			for(j = 0; j < len; j+= NO_OF_BYTES_30MS){
				samples = WebRtcIlbcfix_Decode(Dec_Inst, (short *)&base64_decoded[j], NO_OF_BYTES_30MS, decoded_data, &speechType);
				samples = upsample8to44(decoded_data, upsampled_data, samples);
				AS3_ByteArray_writeBytes(dest, upsampled_data, samples * sizeof(short));
				if(k++ % yieldTicks == 0){
					AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)bytesRemaining / srcLen)) * 100));
					flyield();//yield to main process
				}
			}
		}
	}
#else
	int i = 0;
	while(AS3_ByteArray_readBytes(raw_data, src, NO_OF_BYTES_30MS) == NO_OF_BYTES_30MS){
		len = AS3_ByteArray_readBytes(raw_data, src, NO_OF_BYTES_30MS);
		bytesRemaining -= NO_OF_BYTES_30MS;
		samples = WebRtcIlbcfix_Decode(Dec_Inst, (short *)raw_data, NO_OF_BYTES_30MS, decoded_data, &speechType);
		samples = upsample8to44(decoded_data, upsampled_data, samples);
		AS3_ByteArray_writeBytes(dest, upsampled_data, samples * sizeof(short));
		if(i++ % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)bytesRemaining / srcLen)) * 100));
			flyield();//yield to main process
		}
	}
#endif
		
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
