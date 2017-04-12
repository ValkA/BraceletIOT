// NFC_Tags_Container.h

#ifndef _NFC_TAGS_CONTAINER_h
#define _NFC_TAGS_CONTAINER_h

#include <Arduino.h>
#include <ArduinoSTL.h>
#include <list>

static constexpr int TS_TIME_BITS = 11;
static constexpr int TS_ID_BITS = 3;

struct Timestamp{ //This is a bit field!
	unsigned int time : TS_TIME_BITS; //stored in one minute intervals.
	unsigned int id : TS_ID_BITS; //in case several tags get scanned in the same minute.
};

class Tag_Data {
public:
	Timestamp timestamp;
	uint8_t uid[7];
	Tag_Data(uint8_t uid[]){
		timestamp.time = millis() / 60000; //divided by number of ms in one minute.
		timestamp.id = 0;
		//uid = new uint8_t[7];
		memcpy(this->uid, uid, 7);
	}
	/*
	~Tag_Data() {
		delete[] uid;
	}
	*/
};

class NFC_Tags_Container {
private:
	std::list<Tag_Data> tags;
public:
	void add(uint8_t uid[]) {
		Tag_Data newTag = Tag_Data(uid);
		if (tags.size() > 0) {
			const Tag_Data& last = tags.back();
			if (last.timestamp.time == newTag.timestamp.time) {
				//if max number of tags (max val of timestamp.id) added this minute, we add to next minute
				if (last.timestamp.id < pow(2, TS_ID_BITS) - 1) {
					newTag.timestamp.id = last.timestamp.id + 1;
				}
				else {
					newTag.timestamp.time += 1;
				}
			}
		}
		tags.push_back(newTag);
	}
	void printJSON(Stream& stream) const{
		stream.print('[');
		int count = tags.size();
		int i = 1;
		for (auto td : tags) {
			stream.print(F("{\"uid\":\""));
			char hexid[21] = { 0 };
			for (int i = 0; i < 7; i++) {
				if (td.uid[i] <= 0x0F) {
					itoa(td.uid[i], &hexid[i * 3 + 1], 16);
					hexid[i * 3] = '0';
				}
				else if (td.uid[i] <= 0xFF) {
					itoa(td.uid[i], &hexid[i * 3], 16);
				} //else error.. cant be coz td.[uid] is a byte
				hexid[i * 3 + 2] = ' ';

			}
			hexid[20] = 0;
			stream.print(hexid);

			stream.print(F("\",\"ts\":"));
			stream.print(td.timestamp.time);
			stream.print(F("\",\"tsid\":"));
			stream.print(td.timestamp.id);
			stream.print('}');
			if (i++ != count) stream.print(',');
		}
		stream.print("]");
	}
};



#endif
