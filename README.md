# Split-Flap
<img src="https://media.prusaprinters.org/media/prints/69464/images/757807_e77ddd2d-72e7-44ac-a8ae-af2382483195/thumbs/inside/1920x1440/jpg/titlepic.webp" alt="Image of Split-Flap" width="600"/>

This project has been forked from the brilliant [Split Flap Project](https://github.com/Dave19171/split-flap) by [David Königsmann](https://github.com/Dave19171). None of this would have been possible without the great foundations that have been put in place.

This project has built on the original project to add extra features such as:
- Message Splittng
  - If a message is longer then the number of units there are, the message will be split up and displayed in sequence with a delay between each message
  - Also messages can be split up by adding a `\n`
- Reworked UI
  - Can see messages scheduled to be displayed and option to remove them
  - See extra information on the UI such as:
    - Last Message Received
    - Number of Flaps registered
    - Version Number running
    - How many characters are in the textbox for text
    - Add newline button (as typing `\n` is a pain on a mobile keyboard)
- Message Scheduling
  - Ability to send a message and display it at a later date. If the clock was in another mode such as `Clock` mode, it will show the message for a duration, then return to that mode.
- Arduino OTA
  - Over the Air updates to the display
- Updated `README.md` to add scenarios of problems encountered 

Also the code has been refactored to try facilitate easier development:
- Changed serial prints to one central location so don't have to declare serial enable checks when a new one is required
- Renamed files and functions
- Ping endpoint
- Updated `data` so can test out project locally without having to call a webserver via a `localDevelopment` flag

3D-files here on [Printables](https://www.prusaprinters.org/prints/69464-split-flap-display)!

## General
The display's electronics use one ESP01 as the master and up to 16 Arduinos as slaves. The esp handles the web interface and communicates to the units via I2C. Each unit is resposible for setting the zero position of the drum on startup and displaying any letter the master send its way.

Assemble everything according to the instruction manual.

### PCB
Gerber files are in the `PCB` folder. You need one per unit. Populate components according to the instruction manual on PrusaPrinters. 

Options to potentially get boards created for you:
- [PCB Way](https://www.pcbway.com/)
- [JLC PCB](https://jlcpcb.com/)

> Note: Services are offered by these companies to assembly the boards for you. There are surface mounted components to these devices that you might not be able to do yourself like small resistors for instance, which must be flow soldered. It could be worth having the company do this aspect for you.

### Unit
Each split-flap unit consists of an Arduino Nano mounted on a custom pcb. It controls a 28BYJ-48 stepper motor via a ULN2003 driver chip. The drum with the flaps is homed with a KY003 hall sensor and a magnet mounted to the drum.

Upload the Arduino sketch `unit.ino` in the unit folder to each unit's arduino nano. Before that set the offset with the "eeprom write offset" sketch. 

Inside `unit.ino`, there is a setting for testing the units so that a few
letters are cycled through. At the top of the file once you have opened the project, you will find a line that is commented out:
```c++
#define serial 	// uncomment for serial debug communication
#define test 	//uncomment for Test mode. 
```

> Note: If you experience any problems uploading the unit sketch, you may have to change your `Processor` to use the old bootloader, called `ATmega328p (Old Bootloader)`.

Remove the comment characters to help with your testing for the next step of Setting the Zero Position Offset.

#### Set Zero Position Offset
The zero position (or blank flaps position in this case) is attained by driving the stepper to the hall sensor and step a few steps forward. This offset is individual to every unit and needs to be saved to the arduino nano's EEPROM.

A simple sketch has been written to set the offset. Upload the `EEPROM_Write_Offset.ino` sketch and open the serial monitor with 115200 baudrate. It will tell you the current offset and you can enter a new offset. It should be around 100 but yours may vary. You may need to upload the `unit.ino` sketch and see if the offset is correct. Repeat until the blank flap is showing every time the unit homes.

#### Set Unit Address
Every units address is set by a DIP switch. They need to be set ascending from zero in binary.
This is how my 10 units are set, 1 means switch is in the up-position:
| Unit 1  | Unit 2 | Unit 3 | Unit 4 | Unit 5 | Unit 6 | Unit 7 | Unit 8 | Unit 9 | Unit 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0000 | 0001 | 0010 | 0011 | 0100 | 0101 | 0110 | 0111 | 1000 | 1001 |

### ESP01
To upload the sketch to the ESP01 you need to install a few things to your arduino IDE.
- Install the ESP8266 board to your Arduino IDE 
	- https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/
- Install the arduino esp8266 littleFS plugin to use the file system of the ESP01, you can follow this tutorial: 
	- https://randomnerdtutorials.com/install-esp8266-nodemcu-littlefs-arduino/
- Install the following libraries via Library Manager:
  - Arduino_JSON
  - NTPClient
  - ezTime
- Install the following libraries via including the included `.zip` folders in the `ArduinoLibraries` in this repository in your Arduino Libraries IDE libaries folder:
	- ESPAsyncWebServer
    	- Downloaded From: https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
    	- This library was modified to add a namespace to the `LinkedList` used within its internals to avoid conflicts with `LinkedList` library
	- ESPAsyncTCP 
    	- Downloaded From: https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip


To upload sketches to the ESP01 you can either use an [Arduino Uno](https://create.arduino.cc/projecthub/pratikdesai/how-to-program-esp8266-esp-01-module-with-arduino-uno-598166) or you can buy a dedicated programmer. I highly recommend getting a programmer as it makes uploading programs onto the ESP01 much faster. Examples can be found in the customer reviews of [Amazon](https://www.amazon.co.uk/gp/product/B078J7LDLY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1)

> Note: Be wary of ESP01 programmers that are available which allow USB connection to your PC which may not have programming abilities. Typically extra switches are available so that the ESP01 can be put in programming mode, although you can modify the programmer through a simple solder job to allow it to enter programming mode.

Open the sketch `ESPMaster.ino` in the `ESPMaster` folder, change your board to "Generic ESP8266 Module", choose the correct COM-port and click Tools->ESP8266 LittleFS Data Upload. This uploads the website onto the ESP01's file system.

#### Change Sketch Values
Modify the sketch where it matches the below and update your Wifi Credentials:
```c++
// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "SSID";
const char* password = "12345678901234567890";
```

Also change the `timezoneString` to your time zone. You can find the TZ database names here: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones

You can also modify the date and clock format easily by using this table: https://github.com/ropg/ezTime#datetime

There are several helper `define` variables to help during debugging/running:
- **SERIAL_ENABLE**
  - Use this to enable Serial output lines for tracking executing code
- **OTA_ENABLE**
  - Use this to enable OTA updates from the Arduino IDE
  - Subsequently, you can set a password for OTA via the `otaPassword` variable
- **UNIT_CALLS_DISABLE**
  - Use this to disable the communication with the Arduino Nano Units. This will mean you can check code over function for the ESP module.

#### Final Upload
So far we've only uploaded static fiels to the ESP01. You now need to `Upload` the sketch to the ESP01. Click on Upload and the ESP01 will be upadted with the sketch and you are done. Stick the ESP01 onto the first unit's PCB and navigate to the IP-address the ESP01 is getting assigned from your router.

### Common Mistakes
- If the ESP is not talking to the units correctly, check the UNITSAMOUNT in the `ESPMaster.ino`. The amount of units connected has to match.
- Ensure you upload the sketch and the LittleFS sketch upload to the ESP01. 
- When the system is powered, your Hall Sensor should only light up when a magnet is nearby. 
