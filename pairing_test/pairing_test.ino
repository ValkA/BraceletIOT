
#include "SPI.h"
#include "PN532_SPI.h"
#include "emulatetag.h"
#include "NdefMessage.h"

PN532_SPI pn532spi(SPI, 10);
EmulateTag nfc(pn532spi);

uint8_t bluetoothPairNFCMessage[] = {    
    0xDA,    /* NDEF Header MB=1, ME=1, CF=0, SR=1, IL=1, TNF=2 */
    0x20,    /* Type Length 1 byte */
    0x1D,     /* Payload length bytes */
    0x01,     /* ID length 1byte */
    /* Type Name: application/vnd.bluetooth.ep.oob */
    0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,     
    0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,     
    0x62, 0x6C, 0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74,     
    0x68, 0x2E, 0x65, 0x70, 0x2E, 0x6F, 0x6F, 0x62,     
    0x01,     /* ID : 0x01 */
    /* Payload start */
    /* Bluetooth OOB Data */
    0x1D, 0x00,    /* OOB Data Length */ 
    0xDE, 0x6E, 0x20, 0x31, 0xD3, 0x98,     /* Device Address 98:D3:31:20:6E:DE */
    /* Local Name Field Start */
    0x10,     /* Local Name Length: 15bytes plus Data Type 1byte */
    0x09,     /* EIR Data Type: Local Name 0x09 */
    /* Local Name ArduinoBracelet */
    0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x42,     
    0x72, 0x61, 0x63, 0x65, 0x6C, 0x65, 0x74,     
    /* UUID Field Start */
    0x03,     /* UUID Field Length 3bytes */
    0x03,     /* UUID Field EIR Type 0x03 */
    0x01, 0x11,     /* UUID 1 SerialPort */
};

uint8_t uid[3] = { 0x12, 0x34, 0x56 };

void setup()
{
  Serial.begin(115200);
  Serial.println("------- Emulate Tag --------");
  Serial.print(" ");

  // comment out this command for no ndef message
  nfc.setNdefFile(bluetoothPairNFCMessage, sizeof(bluetoothPairNFCMessage));

  // uid must be 3 bytes!
  nfc.setUid(uid);

  nfc.init();
}

void loop(){
    // uncomment for overriding ndef in case a write to this tag occured
    //nfc.setNdefFile(ndefBuf, messageSize);

    // start emulation (blocks)
    nfc.emulate();

    // or start emulation with timeout
    /*if(!nfc.emulate(1000)){ // timeout 1 second
      Serial.println("timed out");
    }*/

    // deny writing to the tag
    // nfc.setTagWriteable(false);

    if(nfc.writeOccured()){
       Serial.println("\nWrite occured !");
       uint8_t* tag_buf;
       uint16_t length;

       nfc.getContent(&tag_buf, &length);
       NdefMessage msg = NdefMessage(tag_buf, length);
       msg.print();
    }

    delay(1000);
}
