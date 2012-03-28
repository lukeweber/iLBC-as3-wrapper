iLBC AS3 Wrapper
==========

[Internet low bitrate codec](http://en.wikipedia.org/wiki/Internet_Low_Bit_Rate_Codec). This is a swc that wraps the base functionality of the iLBC codec as extraced from rfc3951. Codec is now maintained by google as part of the [WebRTC](http://www.webrtc.org/) project.

To use this with a flash microphone sampledata/bytearray:

* mic.rate = 8 // 8 kHz
* Convert from big to little endian for iLBC (just readFloat the byte array into a little endian array.
* Convert from 32 bit float to 16 bit signed audio (example below)

### default iLBC encode input and decode output

PCM 16 bit signed, little endian, 8 kHz

### Flash Microphone audio

PCM 32 bit float, Big Endian, mic.rate kHz [Flash Microphone Doc](http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/media/Microphone.html). This means that to use this with flash audio you will need to set mic.rate=8 and convert from float to short.

### ILB wrapper input/output

Encoder takes little endian PCM 32 bit float, 8 kHz audio and decoder outputs little endian PCM 32 bit float 44 kHz. Optionally it can use base64 for encoder output and decoder input with the special base64 swc. The input is the default audio that will come out of a flash microphone, if mic.rate = 8.

### Compile iLBC.swc (optional)

[Download Adobe Alchemy](http://labs.adobe.com/downloads/alchemy.html)

[Download flex 4.6](http://opensource.adobe.com/wiki/display/flexsdk/Download+Flex+4.6)

[Getting Started/Install Adobe Alchemy](http://labs.adobe.com/wiki/index.php/Alchemy:Documentation:Getting_Started)

Two options, you can use the rfc version in lib/rfc or you can use the more up
to date and actively developed webRTC version at lib/webrtc. rfc as it's a test
file should compile, webrtc may or may not compile based on future changes of
the repo. Confirmed working build of webrtc with revision 1842 of trunk.

	$ cd (to webrtc or rfc directory)
	$ alc-on
	$ make extract
	$ make all

Optionally you can run  make compileswcb64 which will give you a swc that uses base64 encoding for encoder output and decoder input.

Example
------------
Full Usage of bin/iLBC_webrtc.swc at src/org/ilbc/codec/ILBCCodec.as

### AS3 Code

Methods encode and decode are asyncronous and take as their first params a callback for completed. yieldTicks represents the number of
 ticks the function loops on a encode/decode before it yields to the main thread. It should be noted that you don't want to reset the encoder
 or decoder inbetween chunks of contiguous audio, but only once the audio has completely processed.

	import cmodule.iLBC_webrtc.CLibInit;
	import org.ilbc.event.ILBCEvent;

Initialization of lib

	private var ilbcCodec:Object;
	ilbcCodec = (new cmodule.iLBC_webrtc.CLibInit).init();

Encode

	encodedData = new ByteArray();
	encodedData.endian = Endian.LITTLE_ENDIAN;
	ilbcCodec.encode(encodingCompleteHandler, encodingProgressHandler, rawPCMByteArray, encodedData, decodedData.length, yieldTicks);

Decode

	decodedData = new ByteArray();
	decodedData.endian = Endian.LITTLE_ENDIAN;
	ilbcCodec.decode(decodingCompleteHandler, decodingProgressHandler, encodedData, decodedData, encodedData.length, yieldTicks);

Progress Handler

	function progressHandler(progress:int):void;

Decode Complete Handler

	function decodingCompleteHandler(event:Event):void {
		ilbcCodec.decoderReset();
	}

Encode Complete Handler

	function encodingCompleteHandler(event:Event):void {
		ilbcCodec.encoderReset();
	}
