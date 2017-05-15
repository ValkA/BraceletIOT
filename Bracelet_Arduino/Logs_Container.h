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
static constexpr int LOCATION_FIELD_BITS = 28; //TODO: check how many bits it takes
static constexpr int DEFAULT_DATA_BITS = 28;

static constexpr int LOCATION_FACTOR = 1000;

struct Timestamp {
	unsigned int time : TS_TIME_BITS; //stored in one minute intervals.
	unsigned int id : TS_ID_BITS; //in case several tags get scanned in the same minute.
};

enum Data_Type {
	tag_scan = 0,
	mobile_device_id = 1,
	app_new_data = 2,
	app_update_data = 3,
	app_headquarter_communication = 4,
	blood_pressure = 5,
	app_command = 6,//buzzer
	app_soldier_status = 7,
	// app_location = 8,
	custom = 9,
	app_location_lat = 10,
	app_location_lon = 11
};

union Data {
	unsigned int tagId : TAG_DATA_BITS;
	unsigned int mobileIdData : DEFAULT_DATA_BITS;
	unsigned int rawData : DEFAULT_DATA_BITS;
	unsigned int statusData : DEFAULT_DATA_BITS;
	int location : DEFAULT_DATA_BITS;
};


//class Location {
//
//    unsigned int latitude : LOCATION_FIELD_BITS;
//    unsigned int longitude : LOCATION_FIELD_BITS;
//
//    Location() {};
//    Location(unsigned int latitude , unsigned int longitude) {
//      this->latitude = latitude;
//      this->longitude = longitude;
//    }
//
//    unsigned int locationGetLatitude() {
//      return this->latitude;
//    }
//    unsigned int locationGetLongitude() {
//      return this->latitude;
//    }
//    unsigned int locationUpdateLatitude() {
//      return this->latitude;
//    }
//    unsigned int locationUpdateLongitude() {
//      return this->latitude;
//    }
//};

class LogRecord {
public:
	Timestamp timestamp;
	Data_Type type : TYPE_BITS;
	Data data;
	LogRecord(Data_Type type, Data data) {
		timestamp.time = millis() / 60000; //divided by number of ms in one minute.
		timestamp.id = 0;
		this->type = type;
		this->data = data;
	}
	LogRecord() {}; //This is needed to create the records array.
};

class LogsContainer {
private:
	LogRecord records[CONTAINER_SIZE];
	uint16_t size = 0;
	void printTag(Stream & stream, const LogRecord & td) const
	{
		stream.print(F("{\"data\":\""));
		stream.print(td.data.tagId);
		stream.print(F("\",\"ts\":\""));
		stream.print(td.timestamp.time);
		stream.print(F("\",\"tsid\":\""));
		stream.print(td.timestamp.id);
		stream.print(F("\""));
		stream.print('}');
	}
public:
	LogRecord addNewRecord(Data_Type type, Data data) {
		if (size == CONTAINER_SIZE - 1) {
			Serial.print(F("ERROR: OUT OF MEMORY!"));
			//todo: add buzzer here.
			return;
		}
		LogRecord newTag = LogRecord(type, data);
		if (size > 0) {
			const LogRecord& last = records[size - 1];
			if (last.timestamp.time >= newTag.timestamp.time) {
				//if max number of records (max val of timestamp.id) added this minute, we add to next minute
				if (last.timestamp.id < pow(2, TS_ID_BITS) - 1) {
					newTag.timestamp.id = last.timestamp.id + 1;
					newTag.timestamp.time = last.timestamp.time;
				}
				else {
					newTag.timestamp.time = last.timestamp.time + 1;
				}
			}
		}
		records[size] = newTag;
		size++;
		return newTag;
	}

	int getSize() const {
		return size;
	}

	friend Stream& operator<<(Stream& stream, const LogsContainer& container);
};

//moved the old uid print method to a function in case we need it in the future.
//void printUID(Stream & stream, LogRecord &td)
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
