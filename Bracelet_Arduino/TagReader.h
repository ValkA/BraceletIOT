#ifndef _TAGREADER_h
#define _TAGREADER_h

#include <Arduino.h>
#include "PN532.h"

#define ERROR_READ -1
#define ERROR_PARSE -2
//The Maximum length (in chars) of the number that can be stored in an NFC tag:
#define MAX_TAGID_LENGTH 10 // = ceil(log10(pow(2, DEFAULT_DATA_BITS))) + 1 (for null terminator)
#define MIFAREULTRALIGHT_PAGE_SIZE 4
#define MAX_TAGID_PAGES 3 // = ceil(MAX_TAGID_LENGTH/MIFAREULTRALIGHT_PAGE_SIZE)
#define TAGID_BUFFER_SIZE MAX_TAGID_PAGES*MIFAREULTRALIGHT_PAGE_SIZE

class TagReader {
private:
	PN532& nfc;
	uint8_t data[16] = { 0 }; //data buffer
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
public:
	TagReader(PN532& nfc) : nfc(nfc) {};
	bool tagPresent(long timeout) {
		uint8_t success = 0;
		uint8_t uidLength;
		if (timeout == 0) {
			success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, data, (uint8_t*)&uidLength);
		}
		else {
			success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, data, (uint8_t*)&uidLength, timeout);
		}
		return success;
	}
	//returns the number of chars read.
	int16_t read(char* buffer, uint8_t bufferSize) {
		uint8_t success;
		uint8_t uidLength;
		success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, data, &uidLength);
		if (success) {
			if (uidLength == 4) { //mifareclassic tag
				// The data should be in block 4. Did not have this type of tags so this wasnt tested. todo: test.
				// With this type the blocks should be 16 bytes in length, so there shouldnt be a need to iterate over them.
				success = nfc.mifareclassic_ReadDataBlock(4, data);
				if (success)
				{
					//Debugging:
					Serial.println("Reading Block 4:");
					Serial.println((char*)data);
				}
				else
				{
					return ERROR_READ;
				}
			}
			else if (uidLength == 7) { //mifareultralight tag
				uint8_t count = 0;
				// Out tag info always starts at page 7.
				for (int i = 7; i <= 7 + MAX_TAGID_PAGES; i++) {
					success = nfc.mifareultralight_ReadPage(i, data + count * MIFAREULTRALIGHT_PAGE_SIZE);
					if (success)
					{
						//Debugging:
						//Serial.print("Reading page : ");
						//Serial.println(i);
						//Serial.println((char*)data);
					}
					else
					{
						if (!count)
							return ERROR_READ;
						else
							break;
					}
					count++;
				}
				if (extractNum(buffer, (char*)data, bufferSize)) {
					return count*MIFAREULTRALIGHT_PAGE_SIZE;
				}
				else {
					return ERROR_PARSE;
				}
			}
			else {
				return ERROR_READ;
			}
		}
		else {
			return ERROR_READ;
		}
	}
};


#endif
