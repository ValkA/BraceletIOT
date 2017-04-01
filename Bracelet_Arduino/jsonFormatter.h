#ifndef _Bluetooth_Communication_
#define _Bluetooth_Communication_

#include <ArduinoJson.h>
#include <ArduinoSTL.h>
#include <Arduino.h>
#include "NFC_Tags_Container.h"
#include <SoftwareSerial.h>
#include <list>


static constexpr int BUFFER_SIZE = 200;

class BluetoothCommunication {
private:
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonArray& root = jsonBuffer.createArray();
public:

	void insertTagsIntoBuffer(const NFC_Tags_Container& cont) {     
      
    for (auto td : cont.tags) {
        JsonObject& jObject = jsonBuffer.createObject();
      String id;
      char buffer[10] = { 0 };
      for (int i = 0; i < 7; i++) {
        itoa(td.uid[i], buffer, 16);
        id = id + String(buffer) + ' ';
      }
      jObject["uid"] = id;
      jObject["timestamp"] = td.timestamp;
      root.add(jObject);    
    }
	}

  JsonArray& getJsonArray() {
     return root;
  }
};

#endif
