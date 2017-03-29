// Known_Tags_Container.h

#ifndef _KNOWN_TAGS_CONTAINER_h
#define _KNOWN_TAGS_CONTAINER_h

#include <ArduinoSTL.h>
#include <set>

//Blue bracelet NFC fode: {0x1a,0x61,0x17,0x53,0x0,0x0,0x0}

static constexpr int NUMBER_OF_TAGS = 2;
static constexpr uint8_t KNOWN_TAGS[NUMBER_OF_TAGS][7] = { {0x1a,0x61,0x17,0x53,0x0,0x0,0x0},
															{ 0x1a,0x61,0x17,0x53,0x0,0x0,0x0 } };
//todo: currently the second tag is a dupicate of the first, should be changed to our other tag.

class UID {
public:
	const uint8_t* val;
	UID(const uint8_t uid[]) :val(uid) {};
	bool operator<(const UID& uid) { //for some reason set needs "<" and not "==".
		for (int i = 0; i < 7; i++) {
			if (val[i] < uid.val[i]) {
				return true;
			}
		}
		return false;
	}
};

class Known_Tags_Container {
private:
	std::set<UID> tags;
public:
	Known_Tags_Container() {
		for (int i = 0; i < NUMBER_OF_TAGS; i++) {
			tags.insert(UID(KNOWN_TAGS[i]));
		}
	}
	bool isKnownTag(const UID& uid) {
		if (tags.count(UID(uid)) > 0) {
			return true;
		}
		return false;
	}
};

#endif

