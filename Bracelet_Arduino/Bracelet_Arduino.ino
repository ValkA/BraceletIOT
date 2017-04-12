/*
 Name:		Bracelet_Arduino.ino
 Created:	3/22/2017 11:16:11 PM
 Author:	Michael
*/

#include <MemoryFree.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "PN532.h"
#include "PN532_SPI.h"

#include "emulatetag.h"
#include "NFC_Tags_Container.h"
#include "Known_Tags_Container.h"
#include "NFCPairingProtocol.h"

PN532_SPI pn532spi(SPI, 10);
EmulateTag nfc(pn532spi);
uint8_t emulatedUID[3] = { 0x12, 0x34, 0x56 };  //this is the UID of the emulated tag (that holds the message above for pairing)
uint8_t bluetoothPairNFCMessage[] = BLUETOOTH_PAIR_NFC_MESSAGE;

static constexpr int NFC_READ_TIMEOUT = 1000; //in ms.
static constexpr int NFC_PAIR_TIMEOUT = 7000; //in ms.
static constexpr int BT_DATA_SEND_TIMEOUT = 30000; //in ms.

NFC_Tags_Container tags_cont;
Known_Tags_Container known_tags;

static constexpr int RX = 2; //On nano: white
static constexpr int TX = 3; //On nano: grey
SoftwareSerial bluetoothSerial(RX, TX); //On UNO: 2 violet, 3 blue

void setup(void) {
	Serial.begin(115200);
	Serial.println(F("Starting..."));
	Serial.println(F("Memory Left:"));
	Serial.println(freeMemory());

	bluetoothSerial.begin(9600);
	//bluetoothSerial.println(F("Starting"));

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
	readTag(500);//0 = infinity timeout

	//checking if pairing is finished, and phone sent '#':
	//in the future, we should add a flag that we are waiting for pair
	//right now it's not neccessary since the phone never sends anything other than pair auth.
	if (bluetoothSerial.available()) {
		Serial.print(F("Got #"));
		sendDataBT();
	}
	while (bluetoothSerial.available()) { //clearing the BT buffer.
		bluetoothSerial.read();
	}
}

void readTag(uint16_t timeout) {
	//Serial.println(F("Waiting for an ISO14443A Card ..."));

	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
	uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
	uint8_t success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
	if (!success) return;
	if (known_tags.isKnownTag(uid)) {
		Serial.print(F("Found known tag:"));
		Serial.println();
		nfc.PrintHex(uid, uidLength);
		tags_cont.add(uid);
		Serial.println(F("Memory Left:"));
		Serial.println(freeMemory());
		sendDataBT();
		delay(NFC_READ_TIMEOUT);
	}
	else {
		Serial.print(F("Found unknown tag:"));
		Serial.println();
		nfc.PrintHex(uid, uidLength);
		tryToPairViaNFC(NFC_PAIR_TIMEOUT);
	}
}

void sendDataBT() {
	Serial.println(F("Sending over BT:"));
	tags_cont.printJSON(Serial);
	Serial.println();
	tags_cont.printJSON(bluetoothSerial);
	bluetoothSerial.println('#'); //printing "ln" causes the phone app to crash!
}

void tryToPairViaNFC(uint16_t timeout) {
	Serial.println(F("changing to pair state"));
	nfc.init();//dont know why, but doesnt work without that here..
	nfc.emulate(timeout);
	
	/* removed because we'll try dynamic pairing instead.
	Serial.println(F("waiting for #"));
	unsigned long temp = millis();
	while (!bluetoothSerial.available()) { //waiting for some data to be sent from the phone to make sure there is a connection.
		if (millis() - temp > BT_DATA_SEND_TIMEOUT) {
			Serial.println(F("Timeout! Didnt get #!"));
			break;
		}
	}
	if (bluetoothSerial.available()) {
		Serial.print(F("Got #"));
	}
	while (bluetoothSerial.available()) { //clearing the buffer.
		bluetoothSerial.read();
	}
	sendDataBT();
	*/
}
