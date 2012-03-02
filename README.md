iLBC AS3 Wrapper
==========

[Internet low bitrate codec](http://en.wikipedia.org/wiki/Internet_Low_Bit_Rate_Codec). This is a swc that wraps the base functionality of the iLBC codec as extraced from rfc3951. Codec is now maintained by google as part of the [WebRTC](http://www.webrtc.org/) project.

To use this with a flash microphone sampledata/bytearray:

* mic.rate = 8 // 8 kHz
* Convert from big to little endian for iLBC (just readFloat the byte array into a little endian array.
* Convert from 32 bit float to 16 bit signed audio (example below)

### iLBC encode input and decode output

PCM 16 bit signed, little-Endian, 8 kHz

### Flash Microphone audio

PCM 32 bit float, Big-Endian, mic.rate kHz [Flash Microphone Doc](http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/media/Microphone.html). This means that to use this with flash audio you will need to set mic.rate=8 and convert from float to short.

### Compile iLBC.swc

* [Download Adobe Alchemy](http://labs.adobe.com/downloads/alchemy.html)
* [Download flex 4.6](http://opensource.adobe.com/wiki/display/flexsdk/Download+Flex+4.6)
* [Getting Started/Install Adobe Alchemy](http://labs.adobe.com/wiki/index.php/Alchemy:Documentation:Getting_Started)

	$ cd lib/ilbc
	$ alc-on
	$ make swc

Example
------------
Full Usage of bin/iLBC.swc at src/org/ilbc/codec/ILBCCodec.as

### AS3 Code

Methods encode and decode are asyncronous and take as their first params a callback for completed. As well yield param is frequency of loops of processing audio that will be completed before the function calls flyield(). flyeidl(): "This method will force the Alchemy state machine to give up it's time slice. Execution will return to the line following this call on the next Alchemy time-slicing timer tick." - Time slicing timer tick I believe is 1ms. 

Imports

	import cmodule.iLBC.CLibInit;
	import org.ilbc.event.ILBCEvent;

Initialization of lib

	private var ilbcCodec:Object;
	ilbcCodec = (new cmodule.iLBC.CLibInit).init();

Encode

	encodedData = new ByteArray();
	encodedData.endian = Endian.LITTLE_ENDIAN;
	ilbcCodec.encode(encodingCompleteHandler, encodingProgressHandler, rawPCMByteArray, encodedData, decodedData.length, yield);

Decode

	decodedData = new ByteArray();
	decodedData.endian = Endian.LITTLE_ENDIAN;
	ilbcCodec.decode(decodingCompleteHandler, decodingProgressHandler, encodedData, decodedData, encodedData.length, yield);

Progress Handler

	function progressHandler(progress:int):void;

32 bit floating point to 16 bit signed conversion

	private static const SHORT_MAX_VALUE:int = 0x7fff;
	while( source.bytesAvailable ) {
		var sample:Number = source.readFloat() * SHORT_MAX_VALUE;
		// Make sure we don't overflow.
		if (sample < -SHORT_MAX_VALUE) sample = -SHORT_MAX_VALUE;
		else if (sample > SHORT_MAX_VALUE) sample = SHORT_MAX_VALUE;

		result.writeShort(sample);
	}
