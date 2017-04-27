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
#include "NfcAdapter.h"

#include "emulatetag.h"
#include "NFC_Tags_Container.h"
//#include "Known_Tags_Container.h"
//#include "NFCPairingProtocol.h"

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);

//EmulateTag nfc(pn532spi);
//uint8_t emulatedUID[3] = { 0x12, 0x34, 0x56 };  //this is the UID of the emulated tag (that holds the message above for pairing)
//uint8_t bluetoothPairNFCMessage[] = BLUETOOTH_PAIR_NFC_MESSAGE;

static constexpr int NFC_READ_TIMEOUT = 1000; //in ms.
static constexpr int NFC_PAIR_TIMEOUT = 7000; //in ms.
static constexpr int BT_DATA_SEND_TIMEOUT = 30000; //in ms.
static constexpr int TAG_PRESENT_TIMEOUT = 500; //in ms

NFC_Tags_Container tags_cont;
//Known_Tags_Container known_tags;

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

	nfc.begin();

	/*nfc.setNdefFile(bluetoothPairNFCMessage, sizeof(bluetoothPairNFCMessage));
	nfc.setUid(emulatedUID);
	nfc.init();*/
}

void loop(void) {
	readTag(TAG_PRESENT_TIMEOUT);//0 = infinity timeout

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
	if (nfc.tagPresent(timeout)) {
		Serial.println(F("Found tag"));
		NfcTag tag = nfc.read();
		if (!tag.hasNdefMessage()) {
			Serial.println(F("ERROR: No NDEF message!"));
		}
		else {
			NdefMessage message = tag.getNdefMessage();
			uint8_t recordCount = message.getRecordCount();
			if (recordCount > 1) {
				Serial.println(F("ERROR: More than 1 record!"));
			}
			else if (recordCount == 0) {
				Serial.println(F("ERROR: No record!"));
			}
			NdefRecord record = message.getRecord(0); //we assume the message is in the 1st record.
			uint8_t payloadLength = record.getPayloadLength();
			byte payload[payloadLength];
			record.getPayload(payload);

			uint16_t message_data = atoi((char*)payload);
			Serial.println(F("Message: "));
			Serial.println(message_data);

			tags_cont.add(type_tag_data, message_data);
			sendDataBT();
			Serial.println(F("Memory Left:"));
			Serial.println(freeMemory());
			Serial.println(F("DB size:"));
			Serial.println(tags_cont.getSize());
			
			delay(NFC_READ_TIMEOUT);
			
			/*
			// Can be used for debugging:
			// Force the data into a String (might work depending on the content)
			// Real code should use smarter processing
			String payloadAsString = "";
			for (int c = 0; c < payloadLength; c++) {
				payloadAsString += (char)payload[c];
			}
			Serial.print("  Payload (as String): ");
			Serial.println(payloadAsString);
			*/
		}
	}
}

void sendDataBT() {
	Serial.println(F("Sending over BT:"));
	tags_cont.printJSON(Serial);
	Serial.println();
	tags_cont.printJSON(bluetoothSerial);
	bluetoothSerial.println('#'); //printing "ln" causes the phone app to crash!
}

//void tryToPairViaNFC(uint16_t timeout) {
//	Serial.println(F("changing to pair state"));
//	nfc.init();//dont know why, but doesnt work without that here..
//	nfc.emulate(timeout);
//	
//	/* removed because we'll try dynamic pairing instead.
//	Serial.println(F("waiting for #"));
//	unsigned long temp = millis();
//	while (!bluetoothSerial.available()) { //waiting for some data to be sent from the phone to make sure there is a connection.
//		if (millis() - temp > BT_DATA_SEND_TIMEOUT) {
//			Serial.println(F("Timeout! Didnt get #!"));
//			break;
//		}
//	}
//	if (bluetoothSerial.available()) {
//		Serial.print(F("Got #"));
//	}
//	while (bluetoothSerial.available()) { //clearing the buffer.
//		bluetoothSerial.read();
//	}
//	sendDataBT();
//	*/
//}
