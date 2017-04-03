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
 
	void print(Stream& stream) { //Just for testing - nothing beautiful to merge these 2 funcs - https://github.com/arduino/Arduino/issues/570
    //String s;
    stream.print('[');
    int count = tags.size();
    int i = 1;
    for (auto td : tags) {
      stream.print("{\"uid\":\"");
      char hexid[21] = {0};
      for (int i = 0; i < 7; i++) {
        if(td.uid[i] <= 0x0F){
          itoa(td.uid[i], &hexid[i*3+1], 16);
          hexid[i*3] = '0';
        } else if(td.uid[i] <= 0xFF){
          itoa(td.uid[i], &hexid[i*3], 16);
        } //else error.. cant be foz td.[uid] is a byte
        hexid[i*3+2]=' ';
        
      }
      hexid[20] = 0; 
      stream.print(hexid);
           
      stream.print("\",\"ts\":");
      stream.print(td.timestamp);
      stream.print('}');
      if(i++ != count) stream.print(',');
    }
    stream.print("]");
  }
};



#endif
