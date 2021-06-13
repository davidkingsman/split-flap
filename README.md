# split-flap
code for split-flap display. 
3D-files here: https://www.prusaprinters.org/social/2440-david-kingsman/prints

## General
The display's electronics use one esp01 as the master and up to 16 arduinos as slaves. The esp handles the webinterface and communicates to the units via I2C. Each unit is resposible for setting the zero position of the drum on startup and displaying any letter the master send its way.

Assemble everything according to the instruction manual.

### PCB
Gerber files are in the pcb folder. You need one per unit. Populate components according to the instruction manual.
### Unit
Each split-flap unit consists of an arduino nano mounted on a custom pcb. It controls a 28BYJ-48 stepper motor via a ULN2003 driver chip. The drum with the flaps is homed with a KY003 hall sensor and a magnet mounted to the drum.

Upload the arduino sketch "unit.ino" in the unit folder to each unit's arduino nano. Before that set the offset with the "eeprom write offset" sketch. 

#### set zero position offset
The zero position (or blank flaps position in this case) is attained by driving the stepper to the hall sensor and step a few steps forward. This offset is individual to every unit and needs to be saved to the arduino nano's EEPROM.
I wrote a simple sketch to set the offset. Upload the "EEPROM_Write_Offset.ino" sketch and open the serial monitor with 9600 baudrate. It will tell you the current offset and you can enter a new offset. It should be around 100. You may need to upload the "unit.ino" sketch and see if the offset is correct. Repeat until the blank flap is showing every time the unit homes.

#### set unit address
Every units address is set by a DIP switch. They need to be set ascending from zero in binary.
This is how my 10 units are set, 1 means switch is in the up-position:
| Unit 1  | Unit 2 | Unit 3 | Unit 4 | Unit 5 | Unit 6 | Unit 7 | Unit 8 | Unit 9 | Unit 10 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0000 | 0001 | 0010 | 0011 | 0100 | 0101 | 0110 | 0111 | 1000 | 1001 |


#### ESP01

