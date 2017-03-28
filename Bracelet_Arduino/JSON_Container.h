// JSON_Container.h

#ifndef _JSON_CONTAINER_h
#define _JSON_CONTAINER_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
*/

#include <ArduinoJson.h>
#include "NFC_Tags_Container.h"

static constexpr int BUFFER_SIZE=200;

class JSON_Container {
private:
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	const uint8_t bluetoothOutputPin;
	const uint8_t bluetoothInputPin;
public:
	JSON_Container(uint8_t bluetoothOutputPin, uint8_t bluetoothInputPin)
		: bluetoothOutputPin(bluetoothOutputPin), bluetoothInputPin(bluetoothInputPin) {};
	void insertTagsIntoBuffer(const NFC_Tags_Container& cont) {
		//todo: implement
	}
	void sendTagsBluetooth(const NFC_Tags_Container& cont) {
		//todo: implement
	}
	void recieveTagsBluetooth(NFC_Tags_Container& cont, const String& tags) {
		//todo: implement (long term, maybe we will do it differently)
	}
};

#endif

