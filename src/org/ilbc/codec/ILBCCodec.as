package org.ilbc.codec {
	
	import cmodule.iLBC.CLibInit;
	import flash.utils.getTimer;
	import flash.utils.ByteArray;
	
	/**
	 * Wrapper class for the C iLBC codec.
	 * 
	 * @see http://labs.adobe.com/wiki/index.php/Alchemy:Documentation:Developing_with_Alchemy:AS3_API
	 * @see http://labs.adobe.com/wiki/index.php/Alchemy:Documentation:Developing_with_Alchemy:C_API
	 * 
	 * @see http://ccgi.codegadget.plus.com/blog/2010/04/03/alchemy-c-library-1/ (good Alchemy write up / 101)
	 * 
	 * @author Wijnand Warren
	 */
	public class ILBCCodec {
		
		public var encodedData:ByteArray;
		public var decodedData:ByteArray;
		
		private var ilbcCoder:Object;
		private var initTime:uint;
		
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
		}
		
		/**
		 * 
		 */
		private function start() : void {
			initTime = getTimer();
			
			//timer = new Timer(30);
			//timer.addEventListener(TimerEvent.TIMER, update);
			
			ilbcCoder = (new cmodule.iLBC.CLibInit).init();
			//ilbcCoder.init(this, decodedData, encodedData);
		}
		
		// ------------------------
		// PUBLIC METHODS
		// ------------------------
		
		/**
		 * Encodes an (specs??) audio stream to iLBC.
		 */
		public function encode(data:ByteArray):ByteArray {
			trace("ILBCCodec.encode(data):", data.length);
			
			encodedData = new ByteArray();
			decodedData = data;
			
			start();
			
			ilbcCoder.encode(this, decodedData, encodedData);
			encodedData.position = 0;
			trace(" - encodedData.length:", encodedData.length);
			return encodedData;
		}
		
		/**
		 * Decodes an iLBC encoded audio stream to (specs??).
		 */
		public function decode(data:ByteArray):ByteArray {
			trace("ILBCCodec.decode(data):", data.length);
			
			encodedData = data;
			decodedData = new ByteArray();
			
			start();
			
			ilbcCoder.decode(this, encodedData, decodedData);
			decodedData.position = 0;
			trace(" - decodedData.length:", decodedData.length);
			return decodedData;
		}
		
	}
	
}
