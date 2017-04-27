// NFC_Tags_Container.h

#ifndef _NFC_TAGS_CONTAINER_h
#define _NFC_TAGS_CONTAINER_h

#include <Arduino.h>
//#include <ArduinoSTL.h>
//#include <list>

static constexpr int CONTAINER_SIZE = 100;

//bit field consts:
static constexpr int TS_TIME_BITS = 11;
static constexpr int TS_ID_BITS = 3;
static constexpr int TYPE_BITS = 4;
static constexpr int TAG_DATA_BITS = 16;

struct Timestamp {
	unsigned int time : TS_TIME_BITS; //stored in one minute intervals.
	unsigned int id : TS_ID_BITS; //in case several tags get scanned in the same minute.
};

enum Data_Type { type_tag_data }; //todo: add more data types.

union Data
{
	unsigned int tagData : TAG_DATA_BITS; //type = type_tag_data
	//todo: add more data types.
};

class Tag_Data {
public:
	Timestamp timestamp;
	Data_Type type : TYPE_BITS;
	Data data;
	Tag_Data(Data_Type type, uint32_t data) {
		timestamp.time = millis() / 60000; //divided by number of ms in one minute.
		timestamp.id = 0;
		this->type = type;
		this->data.tagData = data;
	}
	Tag_Data() {}; //This is needed to create the tags array.

	// we dont save uid's anymore.
	//uint8_t uid[7];
	//Tag_Data(uint8_t uid[]){
	//	timestamp.time = millis() / 60000; //divided by number of ms in one minute.
	//	timestamp.id = 0;
	//	//uid = new uint8_t[7];
	//	memcpy(this->uid, uid, 7);
	//}
};

class NFC_Tags_Container {
private:
	Tag_Data tags[CONTAINER_SIZE];
	uint16_t size = 0;
	void printTag(Stream & stream, const Tag_Data & td) const
	{
		stream.print(F("{\"data\":\""));
		stream.print(td.data.tagData);
		stream.print(F("\",\"ts\":\""));
		stream.print(td.timestamp.time);
		stream.print(F("\",\"tsid\":\""));
		stream.print(td.timestamp.id);
		stream.print(F("\""));
		stream.print('}');
	}
public:
	void add(Data_Type type, uint32_t data) {
		if (size == CONTAINER_SIZE - 1) {
			Serial.print(F("ERROR: OUT OF MEMORY!"));
			return;
		}
		Tag_Data newTag = Tag_Data(type, data);
		if (size > 0) {
			const Tag_Data& last = tags[size - 1];
			if (last.timestamp.time >= newTag.timestamp.time) {
				//if max number of tags (max val of timestamp.id) added this minute, we add to next minute
				if (last.timestamp.id < pow(2, TS_ID_BITS) - 1) {
					newTag.timestamp.id = last.timestamp.id + 1;
					newTag.timestamp.time = last.timestamp.time;
				}
				else {
					newTag.timestamp.time = last.timestamp.time + 1;
				}
			}
		}
		tags[size] = newTag;
		size++;
	}
	void printJSON(Stream& stream) const {
		stream.print('[');
		for (int i = 0; i < size; i++) {
			const Tag_Data& td = tags[i];
			printTag(stream, td);
			if (i != size - 1) stream.print(',');
		}
		stream.print("]");
	}
	int getSize() {
		return size;
	}
};

//moved the old uid print method to a function in case we need it in the future.
//void printUID(Stream & stream, Tag_Data &td)
//{
//	stream.print(F("{\"uid\":\""));
//	char hexid[21] = { 0 };
//	for (int i = 0; i < 7; i++) {
//		if (td.uid[i] <= 0x0F) {
//			itoa(td.uid[i], &hexid[i * 3 + 1], 16);
//			hexid[i * 3] = '0';
//		}
//		else if (td.uid[i] <= 0xFF) {
//			itoa(td.uid[i], &hexid[i * 3], 16);
//		} //else error.. cant be coz td.[uid] is a byte
//		hexid[i * 3 + 2] = ' ';
//
//	}
//	hexid[20] = 0;
//	stream.print(hexid);
//}

#endif
