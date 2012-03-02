package org.ilbc.event {
	import flash.utils.ByteArray;
	import flash.events.Event;

	/**
	 * @author Wijnand Warren
	 */
	public class ILBCEvent extends Event {
		
		public static const ENCODING_COMPLETE:String = "ILBCEventEncodingComplete";
		public static const DECODING_COMPLETE:String = "ILBCEventDecodingComplete";
		
		public var data:ByteArray;
		public var duration:int;
		
		/**
		 * Creates a new event.
		 * 
		 * @param type The type of event.
		 * @param data The encoded or decoded ByteArray.
		 * @param duration (Optional) The time it took to encode or decode the data.
		 * @param bubbles (Optional) Indicates whether an event is a bubbling event.
		 * @param cancelable (Optional) Indicates whether the behavior associated with the event can be prevented.
		 */
		public function ILBCEvent(type:String, data:ByteArray, duration:int = 0, bubbles:Boolean = false, cancelable:Boolean = false) {
			super(type, bubbles, cancelable);
			
			this.data = data;
			this.duration = duration;
		}
		
		/**
		 * @inheritDoc
		 */
		override public function clone():Event {
			return new ILBCEvent(type, data, duration, bubbles, cancelable);
		}
	}
}
