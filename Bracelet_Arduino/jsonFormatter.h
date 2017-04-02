#ifndef _json_Formatter_
#define _json_Formatter_

#include <ArduinoJson.h>
#include <ArduinoSTL.h>
#include <Arduino.h>
#include "NFC_Tags_Container.h"
#include <list>


static constexpr int BUFFER_SIZE = 200;

class BluetoothCommunication {
private:
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonArray& root = jsonBuffer.createArray();
public:

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
        jObject["timestamp"] = td.timestamp;
        root.add(jObject);    
        Serial.println(jsonBuffer.size());
	}

  JsonArray& getJsonArray() {
     return root;
  }
};

#endif
