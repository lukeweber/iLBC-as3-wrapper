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

static iLBC_encinst_t *Enc_Inst;
static iLBC_decinst_t *Dec_Inst;

static int encoding = 0;
static int decoding = 0;
static int encoderReset = 0;
static int decoderReset = 0;

int reset_position_byte_array(AS3_Val byteArray)
{
	AS3_Val zero = AS3_Int(0);
	/* just reset the position */
	AS3_SetS((AS3_Val)byteArray, "position", zero);
	AS3_Release(zero);
	return 0;
}

float calculate_step(float a, float b, int samples){
	return (float)(b - a) / samples;
}

void array_float_to_short(float* a, short* b, short samples){
	const short shortMax = 32767;
	const short shortMin = -32768;
	int i;
	for (i = 0; i < samples; i++){
		if(a[i] > 1){
			b[i] = shortMax;
		} else if ( a[i] < -1){
			b[i] = shortMin;
		} else {
			b[i] = (short)(a[i] * shortMax);
		}
	}
}

float short_to_float(short a){
	const float shortMax = 32767.0f;
	float b = (float)(a / shortMax);
	if(b > 1){
		b = 1;
	} else if ( b < -1){
		b = -1;
	}
	return b;
}

/**
 * Function will convert from 16bit to 32bit, 8000 to 44000 audio
 *
 * @return count of samples in output
 */
int prepare_output(short* input, float* output, int samples){
	int i, j, position, loop_samples;
	float step, value1, value2;
	position = 0;

	value1 = short_to_float(input[0]);
	for (i = 0; i < samples - 1; i++) {
		loop_samples = 5 + (i % 2);//Switch between 5 or 6 to convert 8k to 44k
		value2 = short_to_float(input[i+1]);
		step = calculate_step(value1, value2, loop_samples);
		for (j = 0; j < loop_samples; j++){
			output[position++] = (float) (step * j) + value1;
		}
		value1 = value2;
	}
	output[position] = short_to_float(input[samples-1]);
	
	return position+1;
}

static void reset_encoder_impl(){
	if(!encoding){
		WebRtcIlbcfix_EncoderFree(Enc_Inst);
		Enc_Inst = NULL;
		encoderReset = 0;
	} else {
		encoderReset = 1;
	}
}

static void resetEncoder(void * self, AS3_Val args){
	reset_encoder_impl();
}

static void encodeForFlash(void * self, AS3_Val args)
{
	AS3_Val progress;
	AS3_Val src, dest;
	int len, srcLen, bytesRemaining, yieldTicks, samples;
	short mode = 30;//30ms
	float raw_data[BLOCKL_MAX];
	short encoded_data[NO_OF_BYTES_30MS];
	
#ifdef USE_BASE64
	char base64_data[BLOCKL_MAX * 2];
	base64_encodestate state;
	base64_init_encodestate(&state);
#endif

	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	encoding = 1;
	if(Enc_Inst == NULL){
		WebRtcIlbcfix_EncoderCreate(&Enc_Inst);
		WebRtcIlbcfix_EncoderInit(Enc_Inst, mode);
	}
	
	bytesRemaining = srcLen;
	reset_position_byte_array(src);
	int i;
	for (i = 0; bytesRemaining > 0; i++){
		bytesRemaining -= AS3_ByteArray_readBytes(raw_data, src,(mode<<3) * sizeof(float));
		array_float_to_short(raw_data, (short* )raw_data, (short)(mode<<3));
		samples = WebRtcIlbcfix_Encode(Enc_Inst, (short* )raw_data, (short)(mode<<3), encoded_data);
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
	reset_position_byte_array(src);
	reset_position_byte_array(dest);
	// Don't remove progess 100 call here, else complete won't be called!
	encoding = 0;
	if(encoderReset){
		reset_encoder_impl();
	}
	AS3_CallT(progress, NULL, "IntType", 100);
}

static void reset_decoder_impl(){
	if(!decoding){
		WebRtcIlbcfix_DecoderFree(Dec_Inst);
		Dec_Inst = NULL;
		decoderReset = 0;
	} else {
		decoderReset = 1;
	}
}

static void resetDecoder(void * self, AS3_Val args){
	reset_decoder_impl();
}

static void decodeForFlash(void * self, AS3_Val args)
{
	AS3_Val progress;
	AS3_Val src, dest;
	int len, srcLen, yieldTicks, samples, bytesRemaining;
	short decoded_data[BLOCKL_MAX], speechType;
	short mode = 30;//30 ms
	float output_buffer[BLOCKL_MAX * 6];//Really 5.5x blockl (8000 to 44000) hz, + float
	
#ifdef USE_BASE64
	char raw_data[200];//200 base64 bytes will yield 150 bytes or 3 chunks of NO_OF_BYTES_30MS(50)
	char base64_decoded[150];
	base64_decodestate state;
	base64_init_decodestate(&state);
#else
	char raw_data[NO_OF_BYTES_30MS];
#endif
	
	AS3_ArrayValue(args, "AS3ValType, AS3ValType, AS3ValType, IntType, IntType", &progress, &src, &dest, &srcLen, &yieldTicks);

	decoding = 1;
	if(Dec_Inst == NULL){
		WebRtcIlbcfix_DecoderCreate(&Dec_Inst);
		WebRtcIlbcfix_DecoderInit(Dec_Inst, mode);
	}

	reset_position_byte_array(src);
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
				samples = prepare_output(decoded_data, output_buffer, samples);
				AS3_ByteArray_writeBytes(dest, output_buffer, samples * sizeof(float));
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
		samples = prepare_output(decoded_data, output_buffer, samples);
		AS3_ByteArray_writeBytes(dest, output_buffer, samples * sizeof(float));
		if(i++ % yieldTicks == 0){
			AS3_CallT(progress, NULL, "IntType", (int)((1 - ((float)bytesRemaining / srcLen)) * 100));
			flyield();//yield to main process
		}
	}
#endif
	
	reset_position_byte_array(src);
	reset_position_byte_array(dest);
	// Don't remove progess 100 call here, else complete won't be called!
	decoding = 0;
	if(decoderReset){
		reset_decoder_impl();
	}
	AS3_CallT(progress, NULL, "IntType", 100);
}

int main(int argc, char **argv)
{
	AS3_Val ilbc_lib = AS3_Object("");
	AS3_Val encodeMethod = AS3_FunctionAsync( NULL, (AS3_ThunkProc) encodeForFlash);
	AS3_Val decodeMethod = AS3_FunctionAsync( NULL, (AS3_ThunkProc) decodeForFlash);
	AS3_Val resetDecoderMethod = AS3_FunctionAsync( NULL, (AS3_ThunkProc) resetDecoder);
	AS3_Val resetEncoderMethod = AS3_FunctionAsync( NULL, (AS3_ThunkProc) resetEncoder);
	AS3_SetS(ilbc_lib, "encode", encodeMethod);
	AS3_SetS(ilbc_lib, "decode", decodeMethod);
	AS3_SetS(ilbc_lib, "resetEncoder", resetEncoderMethod);
	AS3_SetS(ilbc_lib, "resetDecoder", resetDecoderMethod);
	AS3_Release( encodeMethod );
	AS3_Release( decodeMethod );
	AS3_Release( resetEncoderMethod );
	AS3_Release( resetDecoderMethod );

	//No code below here. AS3_LibInit does not return
	AS3_LibInit( ilbc_lib );
	return 0;
}
