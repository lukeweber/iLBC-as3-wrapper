package org.ilbc.codec {
	
	import cmodule.iLBC_webrtcb64.CLibInit;
	import org.ilbc.event.ILBCEvent;

	import flash.events.EventDispatcher;
	import flash.events.ProgressEvent;
	import flash.utils.ByteArray;
	import flash.utils.Endian;
	import flash.utils.getTimer;
	
	
	[Event(name="ILBCEventEncodingComplete", type="org.ilbc.event.ILBCEvent")]
	[Event(name="ILBCEventDecodingComplete", type="org.ilbc.event.ILBCEvent")]
	[Event(name="progress", type="flash.events.ProgressEvent")]
	
	/**
	 * Wrapper class for the C iLBC codec.
	 * 
	 * @author Wijnand Warren
	 */
	public class ILBCCodec extends EventDispatcher {
		
		/**
		 * iLBC processes data in chunks of 240 shorts, 1 byte.
		 * Flash records data as floats, which are 4 bytes.
		 * Base64 prefers muliples of 3, so we multiply by that here.
		 * 
		 * 240 shorts * 4 (for sizeof( float )) * 3 (Base64 preference), leads to the "optimal" number here, results in no padding.
		 */ 
		public static const PREFERRED_MIN_CHUNK_SIZE:int = 2880;
		
		public static const NOISE_SUPPRESSION_OFF:int = 0;
		public static const NOISE_SUPPRESSION_MILD:int = 1;
		public static const NOISE_SUPPRESSION_MEDIUM:int = 2;
		public static const NOISE_SUPPRESSION_AGGRESSIVE:int = 3;
		
		public var encodedData:ByteArray;
		public var decodedData:ByteArray;
		
		private var isLastEncoding:Boolean;
		private var isLastDecoding:Boolean;
		
		private var ilbcCodec:Object;
		private var initTime:int;
		
		/**
		 * CONSTRUCTOR
		 */
		public function ILBCCodec() {
			init();
		}
		
		/**
		 * Initializes this class.
		 */
		private function init():void {
			initTime = -1;
			ilbcCodec = (new cmodule.iLBC_webrtcb64.CLibInit).init();
		}
		
		// ------------------------
		// PRIVATE METHODS
		// ------------------------
		
		/**
		 * 
		 */
		private function start() : void {
			initTime = getTimer();
		}
		
		/**
		 * Calculates how long an encoding or decoding process took.
		 */
		private function getElapsedTime():int {
			var now:int = getTimer();
			return now - initTime;
		}
		
		// ------------------------
		// EVENT HANDLERS
		// ------------------------
		
		/**
		 * Called when encoding has finished.
		 */
		private function encodingCompleteHandler(event:*):void {
			encodedData.position = 0;
			dispatchEvent( new ILBCEvent(ILBCEvent.ENCODING_COMPLETE, encodedData, getElapsedTime(), isLastEncoding) );
		}
		
		/**
		 * Called when the encoding task notifies progress.
		 */
		private function encodingProgressHandler(progress:int):void {
			dispatchEvent( new ProgressEvent(ProgressEvent.PROGRESS, false, false, progress, 100));
		}

		/**
		 * Called when decoding has finished.
		 */
		private function decodingCompleteHandler(event:*):void {
			decodedData.position = 0;
			dispatchEvent( new ILBCEvent(ILBCEvent.DECODING_COMPLETE, decodedData, getElapsedTime(), isLastDecoding) );
		}

		
		/**
		 * Called when the decoding task notifies progress.
		 */
		private function decodingProgressHandler(progress:int):void {
			dispatchEvent( new ProgressEvent(ProgressEvent.PROGRESS, false, false, progress, 100));
		}
		
		// ------------------------
		// PUBLIC METHODS
		// ------------------------
		
		/**
		 * Encodes an (16 bit 8kHz) audio stream to iLBC.
		 * 
		 * @param data The audio stream to encode.
		 * @param isLast Whether or not this is the last piece of the 
		 * 	(chunked) audio data to encode.
		 * @param noiseCancellationLevel (Optional) The noise suppression level to use while encoding the stream. 
		 * 	Allowed values are defined as constants at the top of this class. The default value is medium suppression.
		 */
		public function encode(data:ByteArray, isLast:Boolean = false, noiseSuppressionLevel:int = 2):void {
			trace("ILBCCodec.encode(data):", data.length);
			
			isLastEncoding = isLast;
			
			encodedData = new ByteArray();
			encodedData.endian = Endian.LITTLE_ENDIAN;
			decodedData = data;
			
			start();
			
			ilbcCodec.encode(encodingCompleteHandler, encodingProgressHandler, decodedData, encodedData, decodedData.length, 10, noiseSuppressionLevel);
		}
		
		/**
		 * Decodes an iLBC encoded audio stream (16 bit 8kHz).
		 */
		public function decode(data:ByteArray, isLast:Boolean = false):void {
			trace("ILBCCodec.decode(data):", data.length);
			
			
			isLastDecoding = isLast;
			
			encodedData = data;
			decodedData = new ByteArray();
			decodedData.endian = Endian.LITTLE_ENDIAN;
			
			start();
			
			ilbcCodec.decode(decodingCompleteHandler, decodingProgressHandler, encodedData, decodedData, encodedData.length, 10);
		}
		
		/**
		 * Resets the encoder.
		 * Should be called when finished with encoding an entire stream.
		 * 
		 * NOTE: When this isn't called it could cause a memory leak.
		 */
		public function resetEncoder():void {
			trace("ILBCCodec.resetEncoder()");
			try {
				ilbcCodec.resetEncoder();
			} catch(e:Error) {
				trace("ERROR RESETTING ENCODER");
				trace(e.getStackTrace());
			}
		}
		
		/**
		 * Resets the decoder.
		 * Should be called when finished with decoding an entire stream.
		 * 
		 * NOTE: When this isn't called it could cause a memory leak.
		 */
		public function resetDecoder():void {
			trace("ILBCCodec.resetDecoder()");
			try {
				ilbcCodec.resetDecoder();
			} catch(e:Error) {
				trace("ERROR RESETTING DECODER");
				trace(e.getStackTrace());
			}
		}
		
	}
	
}
