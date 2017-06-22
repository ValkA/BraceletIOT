/*
 Name:		Bracelet_Arduino.ino
 Created:	3/22/2017 11:16:11 PM
 Author:	Michael
*/

#include <Wire.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "PN532.h"
#include "PN532_SPI.h"
#include "Notes.h"
#include "Parser.h"
#include "Logs_Container.h"
#include "TagReader.h"

//deprecated:
//#include "emulatetag.h"
//#include "Known_Tags_Container.h"
//#include "NFCPairingProtocol.h"
//#include "NfcAdapter.h"
//#include <MemoryFree.h>

//pin defines
#define BUZZER_PIN 9
#define LED1_PIN 5 //for success
#define LED2_PIN 4 //for error
#define PN532_SS 10
#define PN532_MOSI 11
#define PN532_MISO 12
#define PN532_SCK 12
#define HC_06_TX 2
#define HC_06_RX 3

//timeout defines in milliseconds
#define DELAY_BETWEEN_NFC_READS_TIMEOUT 250
#define TAG_PRESENT_TIMEOUT 500
#define WAIT_FOR_ACK_TIMEOUT 200

#define MAX_REPEATS_WHEN_NO_ACK 2

//Buffers (to avoid allocating memory on the stack, which should be avoided as much as possible):
char tagIDBuffer[TAGID_BUFFER_SIZE] = { 0 };

//Structures
PN532_SPI pn532spi(SPI, PN532_SS);
PN532 nfc(pn532spi);
TagReader nfcReader(nfc);
SoftwareSerial bluetoothSerial(HC_06_TX, HC_06_RX);
LogsContainer tags_cont;
Notes note = Notes(BUZZER_PIN, LED1_PIN, LED2_PIN);


void setup(void) {
	Serial.begin(115200);
	bluetoothSerial.begin(9600);

	nfc.begin();
	uint32_t versiondata = nfc.getFirmwareVersion();
	if (!versiondata) {
		Serial.print(F("ERROR: No PN53x nfc board!"));
		note.buzzerPlay(ScanningFailed); //todo: add different sound here.
		note.led2Play(ScanningFailed);
		while (1); // halt
	}
	nfc.SAMConfig();

	pinMode(BUZZER_PIN, OUTPUT);
	Serial.println(F("Setup done..."));
	note.buzzerPlay(TurnOnSuccess);
	note.led1Play(TurnOnSuccess);
}

/*
 * The loop cycles through 3 phases:
 * 1. Waiting for tag on the NFC reader (longest phase, exact time is determined by TAG_PRESENT_TIMEOUT)
 * 2. Checking whether there is a message in the bluetooth buffer.
 * 3. Checking whether there is a debug message on the Serial buffer.
 */
void loop(void) {
	readTag(TAG_PRESENT_TIMEOUT);
	if (bluetoothSerial.available()) {
		switch (bluetoothSerial.peek()) {
		case '[':
			changeNoteSettings(bluetoothSerial);
			break;
		case '<':
			handleDoctorMessage(bluetoothSerial);
			break;
		default:
			handleDoctorMessage(bluetoothSerial);
		}
	}
	if (Serial.available()) {
		handleDebugMessage();
	}
}

void changeNoteSettings(Stream& dataString) {
	uint8_t command = dataString.parseInt();
	NoteType typeNumber = (NoteType)dataString.parseInt();
	uint16_t param1 = dataString.parseInt();
	uint16_t param2 = dataString.parseInt();
	uint8_t param3 = dataString.parseInt();
	uint8_t param4 = dataString.parseInt();
	if (dataString.read() != ']') {
		clearStreamBufferUntilNextMessage(dataString);
	}
	switch (command) {
	case 0: //change buzzer configuration
		note.setToneForNote(typeNumber, param1, param2, param3, param4); //frequncy, freqParam , delay,  repeats
		note.buzzerPlay(typeNumber); //plays buzzer to hear the changes.
		break;
	case 1: //change led1 configuration
		note.setLed1ForNote(typeNumber, param1, param2, param3); //  delayOn,  delayOff,  repeates
		note.led1Play(typeNumber); //plays led to see the changes
		break;
	case 2: //change led2 configuration
		note.setLed2ForNote(typeNumber, param1, param2, param3); //  delayOn,  delayOff,  repeates
		note.led2Play(typeNumber); //plays led to see the changes
		break;
	}
	return;
}

//returns true when there is an error
bool handleRecordError(const LogRecord& record) {
	if (record.type == record_error) {
		Serial.println(F("ERROR: OUT OF MEMORY!"));
		note.buzzerPlay(ScanningFailed); //todo: Add special memroy full buzz,
		note.led2Play(ScanningFailed);
		delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
		return true;
	}
	return false;
}

void readTag(uint16_t timeout) {
	int16_t success;
	if (nfcReader.tagPresent(timeout)) {
		success = nfcReader.read(tagIDBuffer, MAX_TAGID_LENGTH + 2);
		if (success <= 0) {
			Serial.println(F("ERROR: failed NFC read!"));
			note.buzzerPlay(ScanningFailed);
			note.led2Play(ScanningFailed);
		}
		else {
			Data tagData;
			tagData.tagId = atol((char*)tagIDBuffer);

			//add the tag and send it via bluetooth
			LogRecord newRecord = tags_cont.addNewRecord(tag_scan, tagData);
			if (handleRecordError(newRecord)) {
				return;
			}
			recordAddedDebugMessage(newRecord);
			sendRecordBluetooth(newRecord);
			note.buzzerPlay(ScanningSuccess);
			note.led1Play(ScanningSuccess);

			Serial.print(F("DB Size: "));
			Serial.println(tags_cont.getSize());
		}
		delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
		return;
	}
}

void handleDoctorMessage(Stream& stream) {
	LogRecord logRecord;
	if (stream >> logRecord) {
		logRecord = tags_cont.addNewRecord(logRecord.type, logRecord.data);
		if (!handleRecordError(logRecord)) {
			recordAddedDebugMessage(logRecord);
		}

		//do stuff for relevant logRecord.type
		respondToRecordType(stream, logRecord);

		//This shouldnt be here? Already done in respondToRecordType(...).
		//note.buzzerPlay(NewAppMessage);
		//note.led1Play(NewAppMessage);
	}
	else {//ERROR, handle it by clearing the buffer until the next '<'
		clearStreamBufferUntilNextMessage(stream);
	}
}

//stuff for debugging from Serial
void handleDebugMessage() {
	char c = Serial.peek();
	LogRecord debugRecord;
	switch (c) {
	case 'd': //print all the tags:
		Serial << tags_cont;
		break;
	case 'm': //print memory info:
		Serial.print(F("DB Size: "));
		Serial.println(tags_cont.getSize());
		break;
	case 'b': //play buzzer:
		note.buzzerPlay(BeepFromApp);
		break;
	case '<': //add new record manually through the serial:
		if (Serial >> debugRecord) {
			debugRecord = tags_cont.addNewRecord(debugRecord);
			if (handleRecordError(debugRecord)) {
				return;
			}
			recordAddedDebugMessage(debugRecord);
			if (debugRecord.type == tag_scan) {
				sendRecordBluetooth(debugRecord);
				note.buzzerPlay(ScanningSuccess);
				note.led1Play(ScanningSuccess);
			}
			else { //if it's a different type of message, a buzzer or new doctor connection for example.
				respondToRecordType(bluetoothSerial, debugRecord);
			}
		}
		else {
			Serial.println(F("ERROR: Wrong container format!"));
			note.buzzerPlay(ScanningFailed);
			note.led2Play(ScanningFailed);
		}
		break;
	default:
		Serial.println(F("ERROR: Unknown command!"));
		note.buzzerPlay(ScanningFailed);
		note.led2Play(ScanningFailed);
	}
	clearStreamBufferUntilNextMessage(Serial);
}

void sendRecordBluetooth(LogRecord& newRecord) {
	bluetoothSerial << newRecord;
	bluetoothSerial.println(); //This is needed because HC-06/05 doesnt send unless it gets /n
	bool success = resendIfNoAck(newRecord);
	if (!success) {
		Serial.println(F("ERROR: Did not get ack from Android!"));
	}
	else {
		Serial.println(F("SUCCESS: got ack!"));
	}
}

bool resendIfNoAck(LogRecord& newRecord) {
	for (int i = 0; i < MAX_REPEATS_WHEN_NO_ACK; i++) {
		delay(WAIT_FOR_ACK_TIMEOUT);
		if (bluetoothSerial.read() == '#') {
			return true;
		}
		bluetoothSerial << newRecord;
		bluetoothSerial.println(); //needed to actually send...
	}
	return false;
}

void recordAddedDebugMessage(const LogRecord& newRecord)
{
	Serial.print(F("Added: "));
	Serial << newRecord;
	Serial.println();
}

void respondToRecordType(Stream& stream, LogRecord& logRecord) {
	switch (logRecord.type) {
	case app_command:
		stream.println("#");//Ack
		note.buzzerPlay(BeepFromApp);
		break;
	case mobile_device_id:
		stream << tags_cont;//No need for Ack, we are sending data - should *we* wait for an ack here ?!
		note.buzzerPlay(ConnectingSuccess);
		note.led1Play(ConnectingSuccess);
		break;
	case app_location_lat:
		//will beep and send ack after <lon_type, lon>
		break;
	case app_location_lon:
		stream.println("#");//Ack
		note.buzzerPlay(NewAppMessage);
		note.led1Play(NewAppMessage);
		break;
	case record_error:
		stream.println("!");//Error (probably memory full)
		break;
	default:
		//just a beep that message was recieved
		stream.println("#");//Ack
		note.buzzerPlay(NewAppMessage);
		note.led1Play(NewAppMessage);
		break;
	}
}

void clearStreamBufferUntilNextMessage(Stream& stream) {
	while (stream.available() && (stream.peek() != '<' || stream.peek() != '[')) {
		stream.read();
	}
}

//old method
//void readTag(uint16_t timeout) {
//	if (nfc.tagPresent(timeout)) {
//		Serial.println(F("NFC tag detected."));
//		NfcTag tag = nfc.read(); //todo: change library to handle cases where the read fails.
//		if (!tag.hasNdefMessage()) {
//			Serial.println(F("ERROR: No NDEF message!"));
//			note.buzzerPlay(ScanningFailed);
//			note.led2Play(ScanningFailed);
//			delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
//			return;
//		}
//		else {
//			NdefMessage message = tag.getNdefMessage();
//			uint8_t recordCount = message.getRecordCount();
//			if (recordCount > 1) {
//				Serial.println(F("ERROR: More than 1 record!"));
//				note.buzzerPlay(ScanningFailed);
//				note.led2Play(ScanningFailed);
//				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
//				return;
//			}
//			else if (recordCount == 0) {
//				Serial.println(F("ERROR: No record!"));
//				note.buzzerPlay(ScanningFailed);
//				note.led2Play(ScanningFailed);
//				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
//				return;
//			}
//			NdefRecord record = message.getRecord(0); //we assume the message is in the 1st record.
//			uint8_t payloadLength = record.getPayloadLength();
//			if (payloadLength > MAX_PAYLOAD_LENGTH) {
//				Serial.println(F("ERROR: Payload is too large!"));
//				note.buzzerPlay(ScanningFailed);
//				note.led2Play(ScanningFailed);
//				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
//				return;
//			}
//
//			record.getPayload(nfcPayloadBuffer);
//
//			if (!extractNum(tagIDBuffer, (char*)nfcPayloadBuffer, payloadLength)) {
//				Serial.println(F("ERROR: A number couldnt be extracted from Tag!"));
//				note.buzzerPlay(ScanningFailed);
//				note.led2Play(ScanningFailed);
//				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
//				return;
//			}
//
//			Data tagData;
//			tagData.tagId = atol((char*)tagIDBuffer);
//
//			//add the tag and send it via bluetooth
//			LogRecord newRecord = tags_cont.addNewRecord(tag_scan, tagData);
//			recordAddedDebugMessage(newRecord);
//			sendRecordBluetooth(newRecord);
//			note.buzzerPlay(ScanningSuccess);
//			note.led1Play(ScanningSuccess);
//
//			//debugging the payload format:
//			//Serial.print(F("TagID = "));
//			//Serial.println(tagData.tagId);
//
//			Serial.print(F("DB Size: "));
//			Serial.println(tags_cont.getSize());
//
//			//For memory Debugging:
//			//Serial.print(F("Free Memory: "));
//			//Serial.println(freeMemory());
//
//			delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
//		}
//	}
//}
