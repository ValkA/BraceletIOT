// Known_Tags_Container.h

#ifndef _KNOWN_TAGS_CONTAINER_h
#define _KNOWN_TAGS_CONTAINER_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
*/

#include <ArduinoSTL.h>
#include <set>

//Known tag: 1a 61 17 53 0 0 0
static constexpr uint8_t known_tag[7] = { 0x1a,0x61,0x17,0x53,0x0,0x0,0x0 };

class UID {
public:
	uint8_t val[7];
	UID(const uint8_t uid[]) {
		memcpy(this->val, uid, 7);
	}
	bool operator<(const UID& uid) {
		for (int i = 0; i < 7; i++) {
			if (val[i] < uid.val[i]) {
				return true;
			}
		}
		return false;
	}
};

class Known_Tags_Container{
private:
	std::set<UID> tags;
public:
	Known_Tags_Container() {
		tags.insert(UID(known_tag));
	}
};

#endif

