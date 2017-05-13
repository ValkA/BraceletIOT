// JSON_Formatter.h

#ifndef _JSON_FORMATTER_h
#define _JSON_FORMATTER_h

#include <ArduinoJson.h>
#include "NFC_Tags_Container.h"

static constexpr int BUFFER_SIZE = 200;

class JSON_Formatter {
private:
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	const uint8_t bluetoothOutputPin;
	const uint8_t bluetoothInputPin;
public:
	JSON_Formatter(uint8_t bluetoothOutputPin, uint8_t bluetoothInputPin)
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

