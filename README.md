# BraceletIOT

# Bluetooth protocol:
The bracelet reads commands via Bluetooth serial with the format <$type,$data>.
The following types can be sent to it:
- <1,Integer> on connection this is the first thing should be sent to the bracelet.
$data represents a unique number of the phone.
as an acknowledgement the bracelet will send back its database with the following format:
[<$type,$time,$tsid,$data>, <$type,$time,$tsid,$data>, ..., <$type,$time,$tsid,$data>]
also the format <3,$time,$tsid,$pointer_time,$pointer_tsid,$data> may be in the container,
which represents an update record (that was added with <3,$time,$tsid,Integer> message from doctor)
(records can be tags, app_data or any other things as defined in this doc https://docs.google.com/document/d/1-qHOoyWK5xLiwJvJ40AK0ONJ1xDmyQk-YbECwnUxzqk/edit)
- <2,Integer> - adds new data (such as temperature measurement by doctor).
As an acknowledge bracelet sends # back.
- <3,$time,$tsid,Integer> - update record [$time,$tsid] with new data (Integer 14bit max)
- <4,Integer> - adds a record of headquarters communication (such as evacuation notification),
$data can be an ID of something, better ask course staff
As an acknowledge bracelet sends # back.
- <5,Integer> - adds a record of blood pressure.
$data = blood pressure
As an acknowledge bracelet sends # back.
- <6,Integer> - Turns on buzzer,
$data has no meaning, but should be some kind of Integer.
0 can be put there.
As an acknowledge bracelet sends # back.
- <13,Integer> - adds soldier status (severity of injury)
$data is id of the status
As an acknowledge bracelet sends # back.
- <15,Integer> - adds a custom record
As an acknowledge bracelet sends # back.

For example, the command <1,123> will add a record into the bracelet that represents a new connection by a phone that is represented by the number 123. As a response you will get the database.

- LOCATION:
To register location you should send <10,(double/float)latitude><11,(double/float)longitude>
you will see the location when you will send <1,doctor_id>. the location will be among the tags in the container.
for example, [<1,3,0,1><10,32.2343><11,7.23456>]. fields 2 and 3 represent the latitude and longitude.

# Debugging:
It is possible to send debugging commands to the Arduino through the Serial (physical connection to the PC). There are currently 3 types of debugging commands:
- d - print the current database to the Serial.
- m - print memory information to the Serial (size of free memory, number of records currently stored)
- <$type,$data> - add a specific record to the Arduino. The record would be parsed in the exact same manner as a record sent to the Arduino from Bluetooth. If the record is not recognized as one of the above legal records, you'll get an error and the record would not be added.

The Arduino also sends debugging information to the Serial during normal operation, such as detailed error messages. Therefore, if you encounter unexpected behaviour when interfacing through Bluetooth, connect the Arduino through the serial to see detailed debug messages.

# Wiring:
![alt tag](https://raw.githubusercontent.com/ValkA/BraceletIOT/master/bracelet_bb.png)

# IDE:
Don't forget to include libraries from "Libraries for IDE" folder

TI Documentation about NFC pairing - http://www.ti.com/lit/an/sloa187a/sloa187a.pdf

# Notes
You can change the buzzer sound or leds light with bluetooth terminal.
The notes are configured for different cases and you can change configuration for any type separetlly. The types are:
  TurnOnSuccess = 0,
  TurnOnFailed = 1,
  ScanningSuccess = 2,  
  ScanningFailed = 3,
  ConnectingSuccess = 4,
	NewAppMessage = 5,
	BeepFromApp = 6,
	UnknownTag = 7	
  
The command should starts with '[' and ends with ']'.
For change sound insert:
[0, type, frequncy, freqParam , delay,  repeats]

*"type" between 0-7 as explained above.

For example:
[0,2,31,50,200,6] - will change the sound for ScanningSuccess.

For change led i insert: (i={1,2})
[i, type, delayOn,  delayOff,  repeates]

*Delay is in ms.

For example:
[0,2,500,500, 3] - will change the first/second led blinks for ScanningSuccess.


