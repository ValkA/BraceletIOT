# BraceletIOT

# Bluetooth protocol:
The bracelet reads commands via bluetooth serial with the format <$type,$data>
where $type an int that represents the Enum:
- tag_scan = 1,
- mobile_device_id = 2,
- app_new_data = 3,
- app_update_data = 4,
- app_headquarter_communication = 5,
- blood_pressure = 6,
- app_command = 7,//buzzer
- app_soldier_status = 8,
- app_location = 9,
- custom = 10

The char '#' will be sent back as an acknowledgment.
The command <2,$doctor_device_id> will send back all the data that it contains instead of '#'.
($doctor_device_id is an int that represents the ID of the connected phone)
the output will be [<$type,$time,$tsid,$data>, <$type,$time,$tsid,$data>, ..., <$type,$time,$tsid,$data>]
# Wiring:
![alt tag](https://raw.githubusercontent.com/ValkA/BraceletIOT/master/bracelet_bb.png)

# IDE:
Don't forget to include libraries from "Libraries for IDE" folder
TI Documentation about NFC pairing - http://www.ti.com/lit/an/sloa187a/sloa187a.pdf
