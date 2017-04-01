/*
 Name:		Bracelet_Arduino.ino
 Created:	3/22/2017 11:16:11 PM
 Author:	Michael
*/

#include <MemoryFree.h>
#include <Wire.h>

#include "SPI.h"
#include "PN532.h"
#include "PN532_SPI.h"

#include "emulatetag.h"
#include "NFC_Tags_Container.h"
#include "Known_Tags_Container.h"
#include "NFCPairingProtocol.h"
#include "jsonFormatter.h"
#include <SoftwareSerial.h>

PN532_SPI pn532spi(SPI, 10);
EmulateTag nfc(pn532spi);
uint8_t emulatedUID[3] = { 0x12, 0x34, 0x56 };  //this is the UID of the emulated tag (that holds the message above for pairing)
uint8_t bluetoothPairNFCMessage[] = BLUETOOTH_PAIR_NFC_MESSAGE;

static constexpr int NFC_READ_TIMEOUT = 1000; //in ms.
static constexpr int NFC_PAIR_TIMEOUT = 5000; //in ms.

NFC_Tags_Container tags_cont;
Known_Tags_Container known_tags;
BluetoothCommunication bluetoothObject;

//SoftwareSerial bluetoothSerial(6,7);

void setup(void) {
	//serial setup
	Serial.begin(115200);
  //bluetoothSerial.begin(115200);
	Serial.println(F("Starting"));
	pinMode(LED_BUILTIN, OUTPUT);

	//nfc setup
	nfc.setNdefFile(bluetoothPairNFCMessage, sizeof(bluetoothPairNFCMessage));
	nfc.setUid(emulatedUID);
	nfc.init();
	uint32_t versiondata = nfc.getFirmwareVersion();
	if (!versiondata) {
		Serial.print(F("Didn't find PN53x board"));
		while (1); // halt
	}
}

void loop(void) {
	readTag(0);
//************Begin: *For debug- I didnt have tags to scan... if you use it, put the other rows in loop in comment.
// Tag_Data x = Tag_Data(1);
// tags_cont.tags.push_back(x);
// Tag_Data y = Tag_Data(2);
// tags_cont.tags.push_back(y);
// Tag_Data t = Tag_Data(33);
// tags_cont.tags.push_back(t);
// bluetoothObject.insertTagsIntoBuffer(tags_cont);
// bluetoothObject.getJsonArray().prettyPrintTo(Serial);
//*************End: For debug
}

void readTag(uint16_t timeout) {
	Serial.println(F("Waiting for an ISO14443A Card ..."));
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
	uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
	uint8_t success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
	if (success) {
		if (known_tags.isKnownTag(uid)) {
			//Serial.println(F("Found an ISO14443A card"));
			Serial.println(F("Found a known tag."));
			tags_cont.tags.push_back(Tag_Data(uid));
			//nfc.PrintHex(uid, uidLength);
			//For testing:
			Serial.println(F("All saved tags:"));
			//Serial.println(tags_cont.toString()); //doesnt work. Gets corrupted when String too long.
			//tags_cont.print();
			bluetoothObject.insertTagsIntoBuffer(tags_cont);
			Serial.println(F("Memory Left:"));
			Serial.println(freeMemory());
			delay(NFC_READ_TIMEOUT);
		}
		else {
			Serial.println(F("Found an unknown tag."));
			tryToPairViaNFC(NFC_PAIR_TIMEOUT);
		}
	}
}

void tryToPairViaNFC(uint16_t timeout) {
	Serial.println(F("changing to pair state"));
	nfc.init();//dont know why, but doesnt work without that here..
	nfc.emulate(timeout);
}