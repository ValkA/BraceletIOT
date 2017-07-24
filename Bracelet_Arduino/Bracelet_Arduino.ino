#include <Wire.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "PN532.h"
#include "PN532_SPI.h"

#include "Notes.h"
#include "Parser.h"
#include "Logs_Container.h"
#include "TagReader.h"

//pin defines
#define BUZZER_PIN 9
#define LED1_PIN 5 //success
#define LED2_PIN 4 //error
#define PN532_SS 10
#define PN532_MOSI 11
#define PN532_MISO 12
#define PN532_SCK 13
#define HC_06_TX 2
#define HC_06_RX 3

//timeout defines in milliseconds
#define DELAY_BETWEEN_NFC_READS_TIMEOUT 250
#define TAG_PRESENT_TIMEOUT 500
#define WAIT_FOR_ACK_TIMEOUT 200

//Bracelet will send maximum MAX_REPEATS_WHEN_NO_ACK repeats when ack is not recieved.
#define MAX_REPEATS_WHEN_NO_ACK 2

//Buffers (to avoid allocating memory on the stack, which should be avoided as much as possible):
char tagIDBuffer[TAGID_BUFFER_SIZE] = { 0 };

//Structures
PN532_SPI pn532spi(SPI, PN532_SS);
PN532 nfc(pn532spi);
TagReader nfcReader(nfc);
SoftwareSerial bluetoothSerial(HC_06_TX, HC_06_RX);
Notes note = Notes(BUZZER_PIN, LED1_PIN, LED2_PIN);
//The Databse
LogsContainer tags_cont;


void setup(void) {
	//init serial ports
	Serial.begin(115200);
	bluetoothSerial.begin(9600);

	//init nfc
	nfc.begin();
	//check that the NFC scanner is connected:
	uint32_t versiondata = nfc.getFirmwareVersion();
	if (!versiondata) {
		Serial.print(F("ERROR: No PN53x nfc board!"));
		note.buzzerPlay(ScanningFailed); //Instead of ScanningFailed it's possible to add a special boot failed full buzz here.
		note.led2Play(ScanningFailed);
		while (1); // halt - DOES NOT CONTINUE IF CANT FIND AN NFC SCANNER
	}
	nfc.SAMConfig();

	//init leds and buzzer and play successful boot tone
	pinMode(BUZZER_PIN, OUTPUT);
	note.buzzerPlay(TurnOnSuccess);
	note.led1Play(TurnOnSuccess);

	Serial.println(F("Setup done..."));
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
		note.buzzerPlay(ScanningFailed); //Instead of ScanningFailed it's possible to add a special memroy full buzz here.
		note.led2Play(ScanningFailed);
		delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
		return true;
	}
	return false;
}

/**
 * busy wait for nfc tag.
 * if found then reads its data and if valid adds relevant record
 * @param timeout [ms]
 */
void readTag(uint16_t timeout) {
	int16_t success;
	char* pTagIdBuffer = tagIDBuffer;
	//busy wait for tag
	if (nfcReader.tagPresent(timeout)) {
		//tag found now read its data
		success = nfcReader.read(pTagIdBuffer, TAGID_BUFFER_SIZE);
		if (success <= 0) {
			Serial.print(F("ERROR: failed NFC read! "));
			Serial.println(success);
			note.buzzerPlay(ScanningFailed);
			note.led2Play(ScanningFailed);
		}
		else {
			Data tagData;
			Data_Type tagType;
			//its a soldier ID tag
			if (pTagIdBuffer[0] == SOLDIER_ID_START_CHAR) {
				pTagIdBuffer = pTagIdBuffer + 1;
				tagType = soldier_id;
				tagData.rawData = atol((char*)pTagIdBuffer);
			} else { //its a treatment tag
				tagType = tag_scan;
				tagData.tagId = atol((char*)pTagIdBuffer);
			}

			//add the tag record and send it via bluetooth
			LogRecord newRecord = tags_cont.addNewRecord(tagType, tagData);
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
		//delay necessary to avoid reading the same NFC Tag twice.
		delay(DELAY_BETWEEN_NFC_READS_TIMEOUT);
		return;
	}
}

/**
 * handles aa message from the doctor (that comes via stream)
 * @param stream - in our case its bluetoothSerial, but can be any stream.
 */
void handleDoctorMessage(Stream& stream) {
	LogRecord logRecord;
	if (stream >> logRecord) {//if there is a valid record
		if (logRecord.type != get_db) {
			logRecord = tags_cont.addNewRecord(logRecord.type, logRecord.data);
			if (!handleRecordError(logRecord)) {
				recordAddedDebugMessage(logRecord);
			}
		}
		//do stuff for relevant logRecord.type
		respondToRecordType(stream, logRecord);
	}
	else {//ERROR, handle it by clearing the buffer until the next '<'
		clearStreamBufferUntilNextMessage(stream);
	}
}

/**
 * handles debug messages (that comes from Serial)
 */
void handleDebugMessage() {
	char c = Serial.peek();
	//handle according to message type (its first char)
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
			if (debugRecord.type != get_db) {
				debugRecord = tags_cont.addNewRecord(debugRecord);
				if (handleRecordError(debugRecord)) {
					return;
				}
				recordAddedDebugMessage(debugRecord);
			}
			if (debugRecord.type == tag_scan) {
				sendRecordBluetooth(debugRecord); //this is not handled in respondToRecordType.
			}
			respondToRecordType(bluetoothSerial, debugRecord);
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

/**
 * sends log message to bluetooth with ack handling
 * @param newRecord - the record we want to send
 */
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

/**
 * handles acks from Android
 * @param  newRecord - the record we send
 * @return - true iff ack recieved
 */
bool resendIfNoAck(LogRecord& newRecord) {
	for (int i = 0; i < MAX_REPEATS_WHEN_NO_ACK; i++) {
		delay(WAIT_FOR_ACK_TIMEOUT);
		if (bluetoothSerial.read() == '#') {
			return true;
		}
		bluetoothSerial << newRecord;
		bluetoothSerial.println();
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
		stream << tags_cont;
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
	case get_db:
		stream << tags_cont;
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
