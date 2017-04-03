#ifndef _json_Formatter_
#define _json_Formatter_

#include <ArduinoJson.h>
#include <Arduino.h>
#include "NFC_Tags_Container.h"


static constexpr int BUFFER_SIZE = 200;

class JSON_Formatter {
private:
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
	JsonArray& root = jsonBuffer.createArray();
public:
	int getFreeMem() {
		return BUFFER_SIZE - jsonBuffer.size();
	}
	void insertTagIntoBuffer(const Tag_Data& td) {     
        JsonObject& jObject = jsonBuffer.createObject();
        char hexid[21];
        for (int i = 0; i < 7; i++) {
          itoa(td.uid[i], &hexid[i*3], 16);
          if(hexid[i*3] == 0) hexid[i*3] = '0';
          if(hexid[i*3+1] == 0) hexid[i*3+1] = '0';
          hexid[i*3+2] = ' ';
        }
        hexid[20] = 0;
        jObject["uid"] = String(hexid);
        //jObject["timestamp"] = td.timestamp; //maybe should change "timestamp" to something shorter to conserve memory:
		jObject["ts"] = td.timestamp;
        root.add(jObject);
	}

  JsonArray& getJsonArray() {
     return root;
  }
};

#endif
