#ifndef _NOTES_H
#define _NOTES_H

#include <SoftwareSerial.h>

#define DIFFERENT_NOTES_NUM 8

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978


enum NoteType {
	TurnOnSuccess = 0,
	TurnOnFailed = 1,
	ScanningSuccess = 2,
	ScanningFailed = 3,
	ConnectingSuccess = 4,
	NewAppMessage = 5,
	BeepFromApp = 6,
	UnknownTag = 7	
};

class Leds {
	private:
		uint8_t pinLed1;
		uint16_t delayLed1On;
		uint16_t delayLed1Off;
		uint8_t repeatsLed1;
		uint8_t pinLed2;
		uint16_t delayLed2On;
		uint16_t delayLed2Off;
		uint8_t repeatsLed2;
		NoteType type;
		
	public:
    Leds() {};
		Leds(uint8_t led1, uint16_t delay1on, uint16_t delay1off, uint8_t repeats1, uint8_t led2, uint16_t delay2on, uint16_t delay2off, uint8_t repeats2, NoteType type) {
			pinMode(led1, OUTPUT);
			pinMode(led2, OUTPUT);
			this->pinLed1 = led1;
			this->delayLed1On = delay1on;
			this->delayLed1Off = delay1off;
			this->repeatsLed1 = repeats1;
			this->pinLed2 = led2;
			this->delayLed2On = delay2on;
			this->delayLed2Off = delay2off;
			this->repeatsLed2 = repeats2;
			this->type = type;
		}
		
		void updateLed1(uint8_t delayOn, uint8_t delayOff, uint8_t repeats) {
			this->delayLed1On = delayOn;
			this->delayLed1Off = delayOff;
			this->repeatsLed1 = repeats;
			return;
		}
		
		void updateLed2(uint8_t delayOn, uint8_t delayOff, uint8_t repeats) {
			this->delayLed2On = delayOn;
			this->delayLed2Off = delayOff;
			this->repeatsLed2 = repeats;
			return;
		}
		
		void blinkLed1(){
			for (uint8_t i=0; i<repeatsLed1; i++) {
				digitalWrite(pinLed1, HIGH);
				delay(delayLed1On);
				digitalWrite(pinLed1, LOW);
				delay(delayLed1Off);
			}
			digitalWrite(pinLed1, LOW); //For beeing sure the led is turnning off.
		}
		
		void blinkLed2(){
			for (uint8_t i=0; i<repeatsLed2; i++) {
				digitalWrite(pinLed2, HIGH);
				delay(delayLed2On);
				digitalWrite(pinLed2, LOW);
				delay(delayLed2Off);
			}
			digitalWrite(pinLed2, LOW); //For beeing sure the led is turnning off.
		}
		
		NoteType getNoteTypeForLeds() {
			return this->type;
		}
};

class Tone {
	private:
		uint8_t pin;
		uint8_t freq;
		uint8_t freqParam;
		uint8_t delayTime;
		uint8_t repeats;
    NoteType type;
		
	public:
		Tone() {};
		Tone(uint8_t pinNumber, uint8_t frequncy, uint8_t freqParamNumber, uint8_t delay, uint8_t repeatsNumber, NoteType type) {
			this->pin = pinNumber;
			this->freq = frequncy;
			this->freqParam = freqParamNumber;
			this->delayTime = delay;
			this->repeats = repeatsNumber;
			this->type = type;
		}
		void playTone() {
			for (uint8_t i=0; i<repeats -1; i++) {
				tone(pin, freq, freqParam);
				delay(delayTime);
			}
			tone(pin, freq, freqParam);
		}
		void updateFrequncy(uint8_t frequncy) {
			this->freq = frequncy;
		}
		void updateFrequncyParam(uint8_t freqParamNumber) {
			this->freqParam = freqParamNumber;
		}
		void updateDelay(uint8_t delay) {
			this->delayTime = delay;
		}
		void updateRepeats(uint8_t repeatsNumber) {
			this->repeats = repeatsNumber;
		}		
		NoteType getNoteTypeForBuzzer() {
			return this->type;
		}
};

class Notes {
	private:
		uint8_t buzzerPin;
		uint8_t led1Pin;
		uint8_t led2Pin;
		Tone tonesArray[DIFFERENT_NOTES_NUM];
		Leds ledsArray[DIFFERENT_NOTES_NUM];
	public:
		Notes(uint8_t buzzerPin, uint8_t led1Pin, uint8_t led2Pin) {
			//Initiate buzzer settings
			tonesArray[TurnOnSuccess] = Tone(buzzerPin, NOTE_F7, 50, 100, 3, TurnOnSuccess); //pin, freq, freqParam, delay, repeats
			tonesArray[TurnOnFailed] = Tone(buzzerPin, NOTE_C7, 200, 200, 2, TurnOnFailed); //pin, freq, freqParam, delay, repeats
			tonesArray[ScanningSuccess] = Tone(buzzerPin, NOTE_F5, 50, 50, 2, ScanningSuccess); //pin, freq, freqParam, delay, repeats
			tonesArray[ScanningFailed] = Tone(buzzerPin, NOTE_G5, 200, 200, 2, ScanningFailed); //pin, freq, freqParam, delay, repeats
			tonesArray[ConnectingSuccess] = Tone(buzzerPin, NOTE_C7, 50, 100, 3, ConnectingSuccess); //pin, freq, freqParam, delay, repeats
			tonesArray[NewAppMessage] = Tone(buzzerPin, NOTE_F5, 50, 0, 1, NewAppMessage); //pin, freq, freqParam, delay, repeats
			tonesArray[BeepFromApp] = Tone(buzzerPin, NOTE_GS6, 1000, 100, 1, BeepFromApp); //pin, freq, freqParam, delay, repeats
			tonesArray[UnknownTag] = Tone(buzzerPin, NOTE_G4, 50, 50, 4, UnknownTag); //pin, freq, freqParam, delay, repeats
			
			//Initiate leds settings
			for (uint8_t i=0; i<DIFFERENT_NOTES_NUM; i++) {
				ledsArray[i] = Leds(led1Pin, 500, 500, 2, led2Pin, 750, 750, 2, i); //led1, delayOn, delayOff, repeates, led2, delayOn, delayOff, repeates,
			}
		}
		
		void setToneForNote(NoteType typeNumber, uint8_t frequncy, uint8_t freqParam ,uint8_t delay, uint8_t repeats) {
			if (typeNumber < 0 || typeNumber > DIFFERENT_NOTES_NUM) {
				Serial.print(F("Type number should be between 0 to "));
				Serial.println(DIFFERENT_NOTES_NUM);
				return;
			}
			tonesArray[typeNumber].updateFrequncy(frequncy);
			tonesArray[typeNumber].updateFrequncyParam(freqParam);
			tonesArray[typeNumber].updateDelay(delay);
			tonesArray[typeNumber].updateRepeats(repeats);
			Serial.println(F("Tone changed successfully."));
			return;
		}
		
		void setLed1ForNote(NoteType typeNumber, uint16_t delayOn, uint16_t delayOff, uint8_t repeates) {
			if (typeNumber < 0 || typeNumber > DIFFERENT_NOTES_NUM) {
				Serial.print(F("Type number should be between 0 to "));
				Serial.println(DIFFERENT_NOTES_NUM);
				return;
			}
			ledsArray[typeNumber].updateLed1(delayOn, delayOff, repeates);
			Serial.println(F("Led changed successfully."));
			return;
		}
		
		void setLed2ForNote(NoteType typeNumber, uint16_t delayOn, uint16_t delayOff, uint8_t repeates) {
			if (typeNumber < 0 || typeNumber > DIFFERENT_NOTES_NUM) {
				Serial.print(F("Type number should be between 0 to "));
				Serial.println(DIFFERENT_NOTES_NUM);
				return;
			}
			ledsArray[typeNumber].updateLed2(delayOn, delayOff, repeates);
			Serial.println(F("Led changed successfully."));
			return;
		}
		
		void updateBuzzerPinNumber(uint8_t pinNumber) {
			this->buzzerPin = pinNumber;
		}
		
		void updateLed1PinNumber(uint8_t pinNumber) {
			this->led1Pin = pinNumber;
		}
		
		void updateLed2PinNumber(uint8_t pinNumber) {
			this->led2Pin = pinNumber;
		}
		
		void buzzerPlay(NoteType type) {
			for (uint8_t i=0; i< DIFFERENT_NOTES_NUM; i++) {
				if (tonesArray[i].getNoteTypeForBuzzer() == type) {
					tonesArray[i].playTone();
					return;
				}
			}
		}
		
		void led1Play(NoteType type) {
			for (uint8_t i=0; i< DIFFERENT_NOTES_NUM; i++) {
				if (ledsArray[i].getNoteTypeForLeds() == type) {
					ledsArray[i].blinkLed1();
					return;
				}
			}
		}
		
		void led2Play(NoteType type) {
			for (uint8_t i=0; i< DIFFERENT_NOTES_NUM; i++) {
				if (ledsArray[i].getNoteTypeForLeds() == type) {
					ledsArray[i].blinkLed2();
					return;
				}
			}			
		}	
		
	};



#endif
