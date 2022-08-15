# SEATALK_WIFI
The hardware and the included software of this project create a
bridge between a Raymarine SeaTalk1 instrument and NMEA. The NMEA
sentences generated are sent via a WiFi connection to a remote
client. This connection can be directly used with chart plotters such as
[OpenCPN](https://opencpn.org/).

## Hardware
The hardware is made up of a simple board that will connect to to a 12V source
on the right side and to a Seatalk connector on the other (see PCB layout
pictures below). The board provides 12V to power the target instrument so that
the only connection needed for the system to work is a 12V from the boat
electrical panel.

The circuit is not fused and it is expected that a 1A fused be provided by
the installer.

### Schematic
![Seatalk_wifi Schematic](pictures/seatalk_wifi.svg)

During the development of the board it was noticed that the Raymarine
instrument that was used for development (an ST40 wind instrument) was very
finnicky about loading of the SeaTalk serial line. To isolate the Raymarine
instrument from the rest of the circuitry Q2 acts as a buffer.

U1 implemnts a switching regulator rated at least to 2A. An original version
of this board designed with a linear regulator showed that the regulator
tended to overheat, which prompted a redesign with a switching one.

### PCB Layout

The board has been designed with KiCad and the fabrication files are provided
in the fab/ directory.

Board dimensions are 31 mm x 22 mm.

#### Front Side
![PCB Front Side](pictures/seatalk_wifi_front.png)

On the right side of the board the pads, from top to bottom, are:

- GND
- 12V (unfused, use a 1A fuse)

These pads are connected to the boat battery via a fused connection. A better
installation would require powering this board from the same switched
connection that powers the navigation station.

On the left side of the board the pads are meant to be connected to a Raymarine
SeaTalk instrument. The color of the insulator in a typical SeaTalk cable is
shown in parenthesis. From top to bottom:

- GND (black or no insulator)
- SeaTalk (yellow)
- 12V (red)

On the top side of the board the pads, from left to right, are:

- Serial Logger (unmarked on the PCB) This pin is connected to the TX pin
of serial port 2 of the ESP 12 and emits serial data that is mainly
used for debugging of software. If enabled by the user it can also output a
pretty print of each SeaTalk sentence as they are handled by the software

- Programming cable RX line (marked with R in the layout). This is connected
to the RX pin of serial port 1 of the ESP12 and is primarily used to program
the device. Serial port 1 is used for both programming the board as
well as to receive the SeaTalk sentences from the instrument. This means that
this pin must be disconnected from the programmer before resetting the board
to to prevent interference between the programmer hardware and the SeaTalk
instrument. Failure to disconnect this cable during regular use will prevent
the SeaTalk sentences from being received by the MCU

- Programming cable TX line (marked with T in the layout) is connected to the
TX pin of the ESP12 and is only used during programming

Since the board is run at 3.3V the typical programming is performed with a
USB to serial converter connected to pads R, T and GND. If no further software
development is expected the programming cable should be disconnected from the
board and the pads left unconnected.

#### Back Side
![PCB Back Side](pictures/seatalk_wifi_back.png)

The two switches on the left side are only used to program the board and are
typically removed before the board is installed on the boat.

In order to program the board the RST switch (the top one in the picture)
should be pulsed **while the BOOT switch is kept pressed**. At this point
the board is ready for the programmer to write the new software.

## Software
The software can be compiled using the 
[Arduino GUI](https://www.arduino.cc/en/software). The only library required
is [Lennart Hennings' ESPTelnet library](https://github.com/LennartHennigs/ESPTelnet)
which is used for both the Telnet server and the NMEA server.

### Configuration via Web Page
The software runs a very simple web server that serves a single web page used
to configure a few parameters of the bridge. Among other things the page allows
enabling or disabling of the serial and telnet logging windows and the port
at which the NMEA server is listening. During the first start the software
creates a configuration space in EEPROM and stores a few default parameters.
The defaults have been chosen to aid in the debgging of the board during initial
installation and can be changed accordingly after full operation has been
achieved. The built in defaults are:

- Serial logger enabled
- Telnet logger enabled
- The default NMEA port is 3030
- Activity LED enabled

An example of the web page is shown in the following picture.

![Configuration Web Page](pictures/webpage.png)

The default hostname for the instrument is "wind" and can be changed in the
software before programming (see seatalk_wifi.h file) or at a later point
via the configuration web page.

The web page also lists the SeaTalk sentences that are supported by the bridge.

### Serial Port and Telnet Logger
The software outputs a pretty print of each sentence processed to a serial
port or/and to a telnet port. The information output is not the actual NMEA
data but rather descriptive information of each Seatalk command in the order
that it was processed.

### Activity LED
The activity LED lights up for 100 milliseconds each time the software
is in the ```SM_STATE_SEND``` state. This state is used to send This is useful
in quickly determining if messages are being processed without connecting
either a serial port cable or using the tenelt logger.

## Further Developmnent
The board can be improved in a number of ways.

### Handling Additional SeaTalk Sentences
The only instrument the author has is an ST40 wind instrument and the messages
that are emitted by it are those bridged in the current release. The software
processes the Seatalk sentences using a state machine and it makes rather
trivial job to add new commands.

### Adding the Ability to Send SeaTalk Sentences
The current release does not allow transmitting of Seatalk sentences simply
because there was no need for it in the only ST40 instrument owned by the
author. 

In receive mode, the parity bit is extracted using a gimmick that involves
checking for RX errors and then assuming that any such error is due to parity
violation, but when sending data the parity bit has to be handled
as a 9th bit independent of the others which makes it impossible to use the
hardware serial port TX pin to do this. When configured to use serial parity
the ESP12 core sets parity based on the bit pattern to send and does not allow
independently setting the value of the parity bit. For this reason
the SeaTalk TX line is connected to a GPIO in the expectation that a the TX
signal can be bit-banged out of it.

Since SeaTalk is a multiple access protocol the ability to transmit sentences is
made more challenging by the need to detect possible interfence from other
instruments trying to talk over each other.

