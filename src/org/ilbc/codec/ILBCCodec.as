package org.ilbc.codec {
	
	import cmodule.iLBC.CLibInit;
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
		
		public var encodedData:ByteArray;
		public var decodedData:ByteArray;
		
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
		}
		
		// ------------------------
		// PRIVATE METHODS
		// ------------------------
		
		/**
		 * 
		 */
		private function start() : void {
			initTime = getTimer();
			ilbcCodec = (new cmodule.iLBC.CLibInit).init();
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
			trace("ILBCCodec.encodingCompleteHandler(event):", event);
			
			encodedData.position = 0;
			dispatchEvent( new ILBCEvent(ILBCEvent.ENCODING_COMPLETE, encodedData, getElapsedTime()) );
		}
		
		/**
		 * Called when the encoding task notifies progress.
		 */
		private function encodingProgressHandler(progress:int):void {
			trace("ILBCCodec.encodingProgressHandler(event):", progress);
			dispatchEvent( new ProgressEvent(ProgressEvent.PROGRESS, false, false, progress, 100));
		}

		/**
		 * Called when decoding has finished.
		 */
		private function decodingCompleteHandler(event:*):void {
			trace("ILBCCodec.decodingCompleteHandler(event):", event);
			
			decodedData.position = 0;
			dispatchEvent( new ILBCEvent(ILBCEvent.DECODING_COMPLETE, decodedData, getElapsedTime()) );
		}

		
		/**
		 * Called when the decoding task notifies progress.
		 */
		private function decodingProgressHandler(progress:int):void {
			trace("ILBCCodec.decodingProgressHandler(event):", progress);
			dispatchEvent( new ProgressEvent(ProgressEvent.PROGRESS, false, false, progress, 100));
		}
		
		// ------------------------
		// PUBLIC METHODS
		// ------------------------
		
		/**
		 * Encodes an (16 bit 8kHz) audio stream to iLBC.
		 */
		public function encode(data:ByteArray):void {
			trace("ILBCCodec.encode(data):", data.length);
			
			encodedData = new ByteArray();
			encodedData.endian = Endian.LITTLE_ENDIAN;
			decodedData = data;
			
			start();
			
			ilbcCodec.encode(encodingCompleteHandler, encodingProgressHandler, decodedData, encodedData, decodedData.length, 10);
		}
		
		/**
		 * Decodes an iLBC encoded audio stream (16 bit 8kHz).
		 */
		public function decode(data:ByteArray):void {
			trace("ILBCCodec.decode(data):", data.length);
			
			encodedData = data;
			decodedData = new ByteArray();
			decodedData.endian = Endian.LITTLE_ENDIAN;
			
			start();
			
			ilbcCodec.decode(decodingCompleteHandler, decodingProgressHandler, encodedData, decodedData, encodedData.length, 10);
		}
		
	}
	
}
