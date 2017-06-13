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

//deprecated:
//#include "emulatetag.h"
//#include "Known_Tags_Container.h"
//#include "NFCPairingProtocol.h"

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
#define DELAY_BETWEEN_NFC_READS_TIMEOUT 1000
// #define NFC_PAIR_TIMEOUT 7000
// #define BT_DATA_SEND_TIMEOUT 30000
#define TAG_PRESENT_TIMEOUT 500
#define WAIT_FOR_ACK_TIMEOUT 200

#define MAX_REPEATS_WHEN_NO_ACK 2

//Memory defines in bytes
#define MAX_PAYLOAD_LENGTH 64 // Maximum length of NFC message (chars, since it's a string).
//The Maximum length (in chars) of the number that can be stored in an NFC tag:
#define MAX_TAGID_LENGTH 10 // = ceil(log10(pow(2, DEFAULT_DATA_BITS))) + 1 (for null terminator)

//Buffers (to avoid allocating memory on the stack, which should be avoided as much as possible):
byte nfcPayloadBuffer[MAX_PAYLOAD_LENGTH] = { 0 };
char tagIDBuffer[MAX_TAGID_LENGTH] = { 0 };

//Structures
PN532_SPI pn532spi(SPI, PN532_SS);
NfcAdapter nfc = NfcAdapter(pn532spi);
SoftwareSerial bluetoothSerial(HC_06_TX, HC_06_RX);
LogsContainer tags_cont;
Notes note = Notes(BUZZER_PIN, LED1_PIN, LED2_PIN);


void setup(void) {
	Serial.begin(115200);
	bluetoothSerial.begin(9600);
	nfc.begin(); //todo: change library to add indication whether the init was successful.
	pinMode(BUZZER_PIN, OUTPUT);
	Serial.println(F("Setup done..."));
	note.buzzerPlay(TurnOnSuccess);
	note.led1Play(TurnOnSuccess);
	//For memory Debugging:
	Serial.print(F("Free Memory: "));
	Serial.println(freeMemory());
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
	NoteType typeNumber =  dataString.parseInt();	
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
			note.buzzerPlay(typeNumber); //play buzzer for hear the changes.
		break;	
		case 1: //change led1 configuration
			note.setLed1ForNote(typeNumber, param1,  param2,  param3); //  delayOn,  delayOff,  repeates
      note.led1Play(typeNumber); //play led for see the changes
		break;	
		case 2: //change led2 configuration
			note.setLed2ForNote(typeNumber, param1,  param2,  param3); //  delayOn,  delayOff,  repeates
      note.led2Play(typeNumber); //play led for see the changes
		break;
	}
	return;
}

void readTag(uint16_t timeout) {
	if (nfc.tagPresent(timeout)) {
		Serial.println(F("NFC tag detected."));
		NfcTag tag = nfc.read(); //todo: change library to handle cases where the read fails.
		if (!tag.hasNdefMessage()) {
			note.buzzerPlay(ScanningFailed);
			Serial.println(F("ERROR: No NDEF message!"));
			delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
			return;
		}
		else {
			NdefMessage message = tag.getNdefMessage();
			uint8_t recordCount = message.getRecordCount();
			if (recordCount > 1) {
          note.buzzerPlay(ScanningFailed);
				Serial.println(F("ERROR: More than 1 record!"));
				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
				return;
			}
			else if (recordCount == 0) {
				note.buzzerPlay(ScanningFailed);
				Serial.println(F("ERROR: No record!"));
				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
				return;
			}
			NdefRecord record = message.getRecord(0); //we assume the message is in the 1st record.
			uint8_t payloadLength = record.getPayloadLength();
			if (payloadLength > MAX_PAYLOAD_LENGTH) {
				note.buzzerPlay(ScanningFailed);
				Serial.println(F("ERROR: Payload is too large!"));
				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
				return;
			}

			record.getPayload(nfcPayloadBuffer);

			if (!extractNum(tagIDBuffer, (char*)nfcPayloadBuffer, payloadLength)) {
				Serial.println(F("ERROR: A number couldnt be extracted from Tag!"));
				delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
				return;
			}

			Data tagData;
			tagData.tagId = atol((char*)tagIDBuffer);

			//add the tag and send it via bluetooth
			LogRecord newRecord = tags_cont.addNewRecord(tag_scan, tagData);
			recordAddedDebugMessage(newRecord);
			note.buzzerPlay(ScanningSuccess);
			note.led1Play(ScanningSuccess);
			sendRecordBluetooth(newRecord);

			//debugging the payload format:
			//Serial.print(F("TagID = "));
			//Serial.println(tagData.tagId);

			//For memory Debugging:
			Serial.print(F("Free Memory: "));
			Serial.println(freeMemory());

			delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
		}
	}
}

void handleDoctorMessage(Stream& stream) {
	LogRecord logRecord;
	if (stream >> logRecord) {
		note.buzzerPlay(NewAppMessage);
		note.led1Play(NewAppMessage);
		logRecord = tags_cont.addNewRecord(logRecord.type, logRecord.data);
		recordAddedDebugMessage(logRecord);

		//do stuff for relevant logRecord.type
		respondToRecordType(stream, logRecord);
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
		Serial.print(F("Heap memory left: "));
		Serial.println(freeMemory());
		Serial.print(F("DB Size: "));
		Serial.println(tags_cont.getSize());
		break;
	case 'b': //play buzzer:
		note.buzzerPlay(BeepFromApp);
		break;
	case '<': //add new record manually through the serial:
		if (Serial >> debugRecord) {
			debugRecord = tags_cont.addNewRecord(debugRecord);
			recordAddedDebugMessage(debugRecord);
			if (debugRecord.type == tag_scan) {
				note.buzzerPlay(ScanningSuccess);
				note.led1Play(ScanningSuccess);
				sendRecordBluetooth(debugRecord);
			}
			else { //if it's a different type of message, a buzzer or new doctor connection for example.
				respondToRecordType(bluetoothSerial, debugRecord);
			}
		}
		else {
			note.buzzerPlay(ScanningFailed);
			note.led2Play(ScanningFailed);
			Serial.println(F("ERROR: Wrong container format!"));
		}
		break;
	default:
		note.buzzerPlay(ScanningFailed);
		note.led2Play(ScanningFailed);
		Serial.println(F("ERROR: Unknown command, use 'd' to print LogsContainer or 'm' to see how much memory left"));
	}
	clearStreamBufferUntilNextMessage(Serial);
}

void sendRecordBluetooth(LogRecord& newRecord) {
	bluetoothSerial << newRecord;
	bluetoothSerial.println();//needed to actually send...
	bool success = resendIfNoAck(newRecord);
	if (!success) {
		// playErrorTone(BUZZER_PIN); //Should we add an error buzz here?
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

//extracts the first number from the sourceBuffer into the targetBuffer.
bool extractNum(char* targetBuffer, const char* sourceBuffer, uint8_t sourceLength) {
	int i = 0, j = 0;
	for (; i < sourceLength; i++) {
		if (isDigit(sourceBuffer[i])) {
			break;
		}
	}
	for (; i < sourceLength; i++) {
		if (isDigit(sourceBuffer[i])) {
			targetBuffer[j] = sourceBuffer[i];
			j++;
		}
		else {
			break;
		}
	}
	if (j == 0) {
		return false;
	}
	else {
		targetBuffer[j + 1] = 0;
		return true;
	}
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
//		playBuzzTone(BUZZER_PIN);
		note.buzzerPlay(BeepFromApp);
		stream.println("#");//Ack
		break;
	case mobile_device_id:
	//	playDoctorConnectedTone(BUZZER_PIN);
		note.buzzerPlay(ConnectingSuccess);
		note.led1Play(ConnectingSuccess);
		stream << tags_cont;//No need for Ack, we are sending data - should *we* wait for an ack here ?!
		break;
	case app_location_lat:
		//will beep and send ack after <lon_type, lon>
		break;
	case app_location_lon:
		stream.println("#");//Ack
	//	playDoctorMessageTone(BUZZER_PIN);
		note.buzzerPlay(NewAppMessage);
		note.led1Play(NewAppMessage);
		break;
	default:
		//just a beep that message was recieved
	//	playDoctorMessageTone(BUZZER_PIN);
		note.buzzerPlay(NewAppMessage);
		note.led1Play(NewAppMessage);
		stream.println("#");//Ack
		break;
	}
}

void clearStreamBufferUntilNextMessage(Stream& stream) {
	while (stream.available() && (stream.peek() != '<' || stream.peek() != '[')) {
		stream.read();
	}
}
