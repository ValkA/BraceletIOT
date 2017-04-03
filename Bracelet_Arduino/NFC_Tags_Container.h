// NFC_Tags_Container.h

#ifndef _NFC_TAGS_CONTAINER_h
#define _NFC_TAGS_CONTAINER_h

#include <Arduino.h>
#include <ArduinoSTL.h>
#include <list>

class Tag_Data {
public:
	const unsigned long timestamp;
	uint8_t uid[7];
	Tag_Data(uint8_t uid[]) : timestamp(millis()) {
		memcpy(this->uid, uid, 7);
	};
};

class NFC_Tags_Container {
public:
	std::list<Tag_Data> tags;
	void print() { //Just for testing
		//String s;
		for (auto td : tags) {
			Serial.print("uid ");
			for (int i = 0; i < 7; i++) {
				Serial.print("0x");
				Serial.print(td.uid[i], HEX);
				Serial.print(' ');
			}
			Serial.print(", timestamp ");
			Serial.println(td.timestamp);
		}
		//return s;
	}
};



#endif
