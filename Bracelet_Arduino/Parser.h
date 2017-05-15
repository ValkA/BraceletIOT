// JSON_Formatter.h

#ifndef _PARSER_H
#define _PARSER_H

#include "Logs_Container.h"

Stream& operator<<(Stream& stream, const double& val){
 // prints val with number of decimal places determine by precision
 // precision is a number from 0 to 6 indicating the desired decimial places
 // example: printDouble( 3.1415, 2); // prints 3.14 (two decimal places)
 byte precision = 6;
 stream.print (int(val));  //prints the int part
 if( precision > 0) {
   stream.print("."); // print the decimal point
   unsigned long frac;
   unsigned long mult = 1;
   byte padding = precision -1;
   while(precision--)
      mult *=10;
      
   if(val >= 0)
     frac = (val - int(val)) * mult;
   else
     frac = (int(val)- val ) * mult;
   unsigned long frac1 = frac;
   while( frac1 /= 10 )
     padding--;
   while(  padding--)
     stream.print("0");
   stream.print(frac,DEC) ;
 }
}

/**
 * send record to stream.
 * output format is <$type,$time,$tsid,$data>
 */
Stream& operator<<(Stream& stream, const LogRecord& record) {
	stream.print(F("<"));
	stream.print(record.type);
	stream.print(",");
	stream.print(record.timestamp.time);
	stream.print(",");
	stream.print(record.timestamp.id);
	stream.print(",");

	switch (record.type) {
	case tag_scan:
		stream.print(record.data.tagId);
		break;
	case mobile_device_id:
		stream.print(record.data.mobileIdData);
		break;
	case app_new_data:
		stream.print(record.data.rawData);
		break;
	case app_update_data:
		stream.print(record.data.rawData);
		break;
	case app_headquarter_communication:
		stream.print(record.data.rawData);
		break;
	case blood_pressure:
		stream.print(record.data.rawData);
		break;
	case app_command:
		stream.print(record.data.rawData);
		break;
	case app_soldier_status:
		stream.print(record.data.statusData);
		break;
	case app_location_lat:
		stream << (((double)record.data.location)/LOCATION_FACTOR);
		break;
	case app_location_lon:
		stream << (((double)record.data.location)/LOCATION_FACTOR);
		break;
	case custom:
		stream.print(record.data.rawData);
		break;
	default:
		stream.print(record.data.rawData);
		break;
	}

	stream.print(F(">"));
	return stream;
}

/**
 * read record from stream, return true iff success.
 * input format is <$type,$data> TODO:add case for edit message type (right now we dont support this type at all)
 */
bool operator >> (Stream& stream, LogRecord& record) {
	if (stream.read() != '<') {
		return false;
	}

	Data_Type type = (Data_Type)stream.parseInt();
	if (stream.read() != ',') {
		return false;
	}

	Data data;
	switch (type) {
		// case tag_scan: //tags are recieved via NFC and not via bluetooth and that's why this type should go to default case which is return false
		//     data.tagId = stream.parseInt();
		// break;
	case mobile_device_id:
		data.mobileIdData = stream.parseInt();
		break;
	case app_new_data:
		data.rawData = stream.parseInt();
		break;
	case app_update_data:
		data.rawData = stream.parseInt();
		break;
	case app_headquarter_communication:
		data.rawData = stream.parseInt();
		break;
	case blood_pressure:
		data.rawData = stream.parseInt();
		break;
	case app_command:
		data.rawData = stream.parseInt();
		break;
	case app_soldier_status:
		data.statusData = stream.parseInt();
		break;
	case app_location_lat:
		float lat;
		lat = stream.parseFloat();
    Serial << lat;
		data.location = (int)(lat * LOCATION_FACTOR);
    Serial.println();
    Serial.println((int)(lat * LOCATION_FACTOR));
    Serial.println(data.location);
    stream << (((double)record.data.location)/LOCATION_FACTOR);
    Serial.println();
		break;
	case app_location_lon:
		float lon;
		lon = stream.parseFloat();
		data.location = (int)(lon * LOCATION_FACTOR);
		break;
	case custom:
		data.rawData = stream.parseInt();
		break;
	default:
		return false;
		break;
	}

	if (stream.read() != '>') {
		return false;
	}

	record = LogRecord(type, data);
	return true;
}

/**
 * send container to stream.
 */
Stream& operator<<(Stream& stream, const LogsContainer& container) {
	stream.print('[');
	for (int i = 0; i < container.size; i++) {
		stream << container.records[i];
	}
	stream.println("]");
}

#endif
