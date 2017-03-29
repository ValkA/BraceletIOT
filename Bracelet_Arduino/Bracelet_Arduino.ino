/*
 Name:		Bracelet_Arduino.ino
 Created:	3/22/2017 11:16:11 PM
 Author:	Michael
*/

#include "Known_Tags_Container.h"
#include "JSON_Container.h"
#include <Wire.h>
#include "SPI.h"
#include "PN532.h"
#include "PN532_SPI.h"
#include "emulatetag.h"
#include <MemoryFree.h>
#include "NFC_Tags_Container.h"
#include "NFCPairingProtocol.h"
#include "JSON_Container.h"


PN532_SPI pn532spi(SPI, 10);
EmulateTag nfc(pn532spi);
uint8_t emulatedUID[3] = { 0x12, 0x34, 0x56 };  //this is the UID of the emulated tag (that holds the message above for pairing)
uint8_t bluetoothPairNFCMessage[] = BLUETOOTH_PAIR_NFC_MESSAGE;

static constexpr int NFC_READ_TIMEOUT = 1000; //in ms.
static constexpr int NFC_PAIR_TIMEOUT = 5000; //in ms.

NFC_Tags_Container tags_cont;

void setup(void) {
  //serial setup
	Serial.begin(115200);
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
    tryToPairViaNFC(2000);
    readTag(0);
}

void readTag(uint16_t timeout){
  Serial.println(F("Waiting for an ISO14443A Card ..."));
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);
    if (success) {
      Serial.println(F("Found an ISO14443A card"));
      tags_cont.tags.push_back(Tag_Data(uid));
      //nfc.PrintHex(uid, uidLength);
      // For testing:
      Serial.println(F("All saved tags:"));
      //Serial.println(tags_cont.toString()); //doesnt work. Gets corrupted when String too long.
      tags_cont.print();
      Serial.println(F("Memory Left:"));
      Serial.println(freeMemory());
	  delay(NFC_READ_TIMEOUT);
    }
}

void tryToPairViaNFC(uint16_t timeout){
  Serial.println(F("changing to pair state"));
  nfc.init();//dont know why, but doesnt work without that here..
  nfc.emulate(timeout);
}
