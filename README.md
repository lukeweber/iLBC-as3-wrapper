iLBC AS3 Wrapper
==========

iLBC or internet low bitrate codec. This is a swc that wraps the base functionality of the iLBC codec as extraced from rfc3951. Codec is now maintained by google as part of the webRTC project.
More info: http://en.wikipedia.org/wiki/Internet_Low_Bit_Rate_Codec

### iLBC encode input and decode output

PCM 16 bit signed, little-Endian, 8 kHz

### Flash Microphone audio

PCM 32 bit float, Big-Endian, mic.rate kHz (http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/media/Microphone.html). This means that to use this with flash audio you will need to set mic.rate=8 and convert from float to short.

### Compile iLBC.swc
Install Adobe Alchemy http://labs.adobe.com/technologies/alchemy/

	$ cd lib/ilbc
	$ alc-on
	$ make swc

Example
------------

Usage of bin/iLBC.swc at src/org/ilbc/codec/ILBCCodec.as

Converting 32 bit floating point to 16 bit signed int in AS3
	private static const SHORT_MAX_VALUE:int = 0x7fff;
	while( source.bytesAvailable ) {
		var sample:Number = source.readFloat() * SHORT_MAX_VALUE;
		
		// Make sure we don't overflow.
		if (sample < -SHORT_MAX_VALUE) sample = -SHORT_MAX_VALUE;
		else if (sample > SHORT_MAX_VALUE) sample = SHORT_MAX_VALUE;

		result.writeShort(sample);
	}
