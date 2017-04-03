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
    Serial.print('[');
    int count = tags.size();
    int i = 1;
    for (auto td : tags) {
      Serial.print("{\"uid\":\"");
      /*char hexid[21] = {0};
      for (int i = 0; i < 7; i++) {
        itoa(td.uid[i], &hexid[i*3], 16);
        if(hexid[i*3] == 0) hexid[i*3] = '0';
        if(hexid[i*3+1] == 0) hexid[i*3+1] = '0';
        hexid[i*3+2] = ' ';
      }
      hexid[20] = 0; 
      Serial.print(hexid);*/

      String id;
      char buffer[10] = { 0 };
      for (int i = 0; i < 7; i++) {
        itoa(td.uid[i], buffer, 16);
        id = id + String(buffer);
      }
      Serial.print(id);
           
      Serial.print("\",\"ts\":");
      Serial.print(td.timestamp);
      Serial.print('}');
      if(i++ != count) Serial.print(',');
    }
    Serial.println("]");
  }
  
  void sendJsonToSerial(SoftwareSerial& serial) { //Just for testing
    //String s;
    serial.print('[');
    int count = tags.size();
    int i = 1;
    for (auto td : tags) {
      serial.print("{\"uid\":\"");
      /*char hexid[21] = {0};
      for (int i = 0; i < 7; i++) {
        itoa(td.uid[i], &hexid[i*3], 16);
        if(hexid[i*3] == 0) hexid[i*3] = '0';
        if(hexid[i*3+1] == 0) hexid[i*3+1] = '0';
        hexid[i*3+2] = ' ';
      }
      hexid[20] = 0; 
      serial.print(hexid);*/

      String id;
      char buffer[10] = { 0 };
      for (int i = 0; i < 7; i++) {
        itoa(td.uid[i], buffer, 16);
        id = id + String(buffer);
      }
      serial.print(id);
      serial.print("\",\"ts\":");
      serial.print(td.timestamp);
      serial.print('}');
      if(i++ != count) serial.print(',');
    }
    serial.print("]");
  }
};



#endif
