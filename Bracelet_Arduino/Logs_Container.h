#ifndef _LOGS_CONTAINER_h
#define _LOGS_CONTAINER_h

#include <Arduino.h>

// Maximum number of records that can be stored:
static constexpr int CONTAINER_SIZE = 100;

//bit field consts:
static constexpr int TS_TIME_BITS = 11;
static constexpr int TS_ID_BITS = 3;
static constexpr int TYPE_BITS = 4;
static constexpr int DEFAULT_DATA_BITS = 28;

//Note: This is the maximum factor possible. On Arduino, float = double, and has only 6 digits of precision.
//Even using this factor, there is a small loss of percision on the last digit.
//It also must be long, not int, since int on arduino is only 16 bits, which means max is 65,535.
static constexpr unsigned long LOCATION_FACTOR = 1000000; //10^6

/**
 * structure to identify when events happened, used as the unique id of each record.
 * (2 records cant have the same timestamp)
 */
struct Timestamp {
	unsigned int time : TS_TIME_BITS; //stored in one minute intervals (since turn on).
	unsigned int id : TS_ID_BITS; //used to distinguish records recorded in the same minute.
};

/**
 * LogRecord types, used to distinguish between record types
 */
enum Data_Type {
	tag_scan = 0,
	mobile_device_id = 1,
	app_new_data = 2,
	app_update_data = 3,
	app_headquarter_communication = 4,
	blood_pressure = 5,
	app_command = 6, //buzzer
	app_delete_data = 7,
	soldier_id = 8,
	custom = 9,
	app_location_lat = 10,
	app_location_lon = 11,
	app_soldier_status = 13,
	get_db = 14, //this type is used for sync. It is not recorded in the DB.
	record_error = 15 //special type for internal use only.
};

/**
 * Our objective is to record an update for a record without deleting the original record.
 * This type of record points to the updated record (using the updated record's unique Timestamp) and contains its new value.
 */
struct UpdateRecord {
	Timestamp ts;
	unsigned long data : (DEFAULT_DATA_BITS - (TS_TIME_BITS + TS_ID_BITS));
};

/**
 * union between all data types that can be in a LogRecord
 */
union Data {
	unsigned long tagId : DEFAULT_DATA_BITS;
	unsigned long mobileIdData : DEFAULT_DATA_BITS;
	unsigned long rawData : DEFAULT_DATA_BITS;
	unsigned long statusData : DEFAULT_DATA_BITS;
	UpdateRecord UpdateRecord;
	long location : DEFAULT_DATA_BITS;
};

/**
 * The LogRecord
 */
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
	LogRecord() {
		this->type = record_error;
	}; //This is needed to create the records array, is also used to id error records.
};

class LogsContainer {
private:
	LogRecord records[CONTAINER_SIZE];
	uint16_t size = 0;
public:
	/**
	 * adds a new record into the LogsContainer
	 * @param  type - type of record
	 * @param  data - its data
	 * @returns the created LogRecord with valid timestamp.
	 */
	LogRecord addNewRecord(Data_Type type, Data data) {
		LogRecord newTag = LogRecord(type, data);
		if (size == CONTAINER_SIZE) {
			newTag.type = record_error;
			return newTag; //will return an error record (type = record_error).
		}
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

	/**
	 * The timestamp isn't copied ! it calculates the current timestamp.
	 */
	LogRecord addNewRecord(const LogRecord& newRecord) {
		return addNewRecord(newRecord.type, newRecord.data);
	}

	uint16_t getSize() const {
		return size;
	}

	/**
	 * serializer for the Log container
	 */
	friend Stream& operator<<(Stream& stream, const LogsContainer& container);
};

#endif
