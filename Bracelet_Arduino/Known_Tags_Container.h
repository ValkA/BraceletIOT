// Known_Tags_Container.h

#ifndef _KNOWN_TAGS_CONTAINER_h
#define _KNOWN_TAGS_CONTAINER_h

#include <ArduinoSTL.h>
#include <set>

static constexpr int NUMBER_OF_TAGS = 5;
static constexpr uint8_t KNOWN_TAGS[NUMBER_OF_TAGS][7] = 
                            {{0x1a,0x61,0x17,0x53,0x0,0x0,0x0}, //blue bracelet tag
                             {0x8E,0x9C,0x75,0xDD,0x0,0x0,0x0}, //exposed tag
                             {0x04,0x9E,0x8E,0x4A,0xA8,0x4A,0x80},
                             {0x04,0xA7,0x8C,0x4A,0xA8,0x4A,0x80},
                             {0x04,0xAB,0x8D,0x4A,0xA8,0x4A,0x80}};

class UID {
public:
	const uint8_t* val;
	UID(const uint8_t uid[]) :val(uid) {};
	bool operator<(const UID& uid) const{ //for some reason set needs "<" and not "==".
		for (int i = 0; i < 7; i++) {
			if (val[i] < uid.val[i]) {
				return true;
			}
		}
		return false;
	}
};


//using loop (to save space):
class Known_Tags_Container {
public:
	bool isKnownTag(const UID& uid) const {
		for (int i = 0; i < NUMBER_OF_TAGS; i++) {
			UID temp = UID(KNOWN_TAGS[i]);
			if (!(temp < uid) && !(uid < temp)) { //this is equivalent to '=='
				return true;
			}
		}
		return false;
	}
};

/*
//using set (takes more space (~76 bytes) but faster):
class Known_Tags_Container {
private:
	std::set<UID> tags;
public:
	Known_Tags_Container() {
		for (int i = 0; i < NUMBER_OF_TAGS; i++) {
			tags.insert(UID(KNOWN_TAGS[i]));
		}
	}
	bool isKnownTag(const UID& uid) const{
		if (tags.count(UID(uid)) > 0) {
			return true;
		}
		return false;
	}
};
*/

#endif

