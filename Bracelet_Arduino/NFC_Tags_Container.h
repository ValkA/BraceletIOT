// NFC_Tags_Container.h

#ifndef _NFC_TAGS_CONTAINER_h
#define _NFC_TAGS_CONTAINER_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
*/

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
			String id;
			char buffer[10] = { 0 };
			for (int i = 0; i < 7; i++) {
				itoa(td.uid[i], buffer, 16);
				id = id + String(buffer) + ' ';
			}
			//s = s + id + "," + td.timestamp + "\n";
			Serial.println(id + "," + td.timestamp);
		}
		//return s;
	}
};



#endif

