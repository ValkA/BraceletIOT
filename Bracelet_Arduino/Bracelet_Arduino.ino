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
#include "Notes.h"
#include "Parser.h"
#include "Logs_Container.h"
//#include "emulatetag.h"
//#include "Known_Tags_Container.h"
//#include "NFCPairingProtocol.h"

//pin defines
#define BUZZER_PIN 9
#define LED_PIN 5
#define PN532_SS 10
#define PN532_MOSI 11
#define PN532_MISO 12
#define PN532_SCK 12
#define HC_06_TX 2
#define HC_06_RX 3

//timeout defines in milliseconds
#define NFC_READ_TIMEOUT 1000
#define NFC_PAIR_TIMEOUT 7000
#define BT_DATA_SEND_TIMEOUT 30000
#define TAG_PRESENT_TIMEOUT 500

//Structures
PN532_SPI pn532spi(SPI, PN532_SS);
NfcAdapter nfc = NfcAdapter(pn532spi);
SoftwareSerial bluetoothSerial(HC_06_TX, HC_06_RX);
LogsContainer tags_cont;


void setup(void) {
	Serial.begin(115200);
	bluetoothSerial.begin(9600);
	nfc.begin();
	pinMode(BUZZER_PIN, OUTPUT);
	Serial.println(F("Setup done..."));
	//For memory Debugging:
	Serial.print(F("Free Memory: "));
	Serial.println(freeMemory());
}

void loop(void) {
	readTag(TAG_PRESENT_TIMEOUT);
	if (bluetoothSerial.available()) {
		handleDoctorMessage(bluetoothSerial);
	}
	if (Serial.available()) {
		handleDebugMessage();
	}
}

void readTag(uint16_t timeout) {
	if (nfc.tagPresent(timeout)) {
		Serial.print(F("Found tag: "));
		NfcTag tag = nfc.read();
		if (!tag.hasNdefMessage()) {
			playErrorTone(BUZZER_PIN);
			Serial.println(F("ERROR: No NDEF message!"));
		}
		else {
			NdefMessage message = tag.getNdefMessage();
			uint8_t recordCount = message.getRecordCount();
			if (recordCount > 1) {
				playErrorTone(BUZZER_PIN);
				Serial.println(F("ERROR: More than 1 record!"));
			}
			else if (recordCount == 0) {
				playErrorTone(BUZZER_PIN);
				Serial.println(F("ERROR: No record!"));
			}
			NdefRecord record = message.getRecord(0); //we assume the message is in the 1st record.
			uint8_t payloadLength = record.getPayloadLength(); //todo: probably should add some check here that payloadLength isnt too large.
			byte payload[payloadLength + 1];
			record.getPayload(payload);
			payload[payloadLength] = '\0';// null terminator for atoi
			Data tagData;
			tagData.tagId = atoi(payload);
			//add the tag and send it via bluetooth
			bluetoothSerial << tags_cont.addNewRecord(tag_scan, tagData);
			bluetoothSerial.println();//needed to actually send...

			//for debug
			Serial.print(F("TagID = "));
			Serial.println((char*)payload);
			playNewTagTone(BUZZER_PIN);
			delay(NFC_READ_TIMEOUT);
			//For memory Debugging:
			Serial.print(F("Free Memory: "));
			Serial.println(freeMemory());
		}
	}
}

void handleDoctorMessage(Stream& stream) {
	LogRecord logRecord;
	if (stream >> logRecord) {
		tags_cont.addNewRecord(logRecord.type, logRecord.data);
		Serial.print(F("Got Doctor message type "));
		Serial.println(logRecord.type);

		//do stuff for relevant logRecord.type
		switch (logRecord.type) {
		case app_command:
			playBuzzTone(BUZZER_PIN);
			stream.println("#");//Ack
			break;
		case mobile_device_id:
			playDoctorConnectedTone(BUZZER_PIN);
			stream << tags_cont;//No need for Ack, we are sending data - should *we* wait for an ack here ?!
			break;
		default:
			//just a beep that message was recieved
			playDoctorMessageTone(BUZZER_PIN);
			stream.println("#");//Ack
			break;
		}
	}
	else {//ERROR, handle it by clearing the buffer until the next '<'
		playErrorTone(BUZZER_PIN);
		while (stream.available() && stream.peek() != '<') {
			stream.read();
		}
	}
}

//stuff for debugging from Serial
void handleDebugMessage() {
	char c = Serial.read();
	switch (c) {
	case 'd':
		Serial << tags_cont;
		break;
	case 'm':
		Serial.print(F("Heap memory left: "));
		Serial.println(freeMemory());
		Serial.print(F("DB Size: "));
		Serial.println(tags_cont.getSize());
		break;
	case 'b':
		playBuzzTone(BUZZER_PIN);
		break;
	case 't':
		Data tagData;
		tagData.tagId = Serial.parseInt();
		bluetoothSerial << tags_cont.addNewRecord(tag_scan, tagData);
		bluetoothSerial.println();
		playNewTagTone(BUZZER_PIN);
		break;
	default:
		playErrorTone(BUZZER_PIN);
		Serial.println(F("Unknown command, use 'd' to print LogsContainer or 'm' to see how much memory left"));
	}
}
