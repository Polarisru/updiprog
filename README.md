# UPDI program tools

Utility and library with CLI (Pure-C) and GUI (Lazarus) modes for programming AVR devices with UPDI interface using a standard TTL serial port

#### Screeshots

![updi_gui](https://raw.githubusercontent.com/iLya2IK/updiprog/master/updigui/screenshots/Screenshot_20231018_221738.png)

## updiprog

This is C version of UPDI interface utility with improvements, you could refer to the Python version: [pyupdi](https://github.com/mraardvark/pyupdi.git)
	
pyupdi is a Python utility for programming AVR devices with UPDI interface using a standard TTL serial port.

The main purpose is the possibility to use UPDI to flash the new TinyAVR at any PC. No external libraries were used, so no dependencies. I have some problems during pyupdi installation because of absence of the Internet connection, so I had to copy all necessary packages and wheels first and to install them manually. Yes, I know about the possibility to compile the executable from the Python script, but pyupdi has also some disadvantages like lackage of reading the flash content, slow speed of programming and probably no error handling at all. So I have decided to improve it and get it working with standard C language.

[Code::Blocks IDE](http://www.codeblocks.org/) was used to write code and compile it.

I have tried to write it portable for Windows and Linux but can't really test it on Linux because of the driver bug for CH340 USB to serial converter, it can't work with parity bits.

I am using [CH340 USB to TTL converter](https://www.elektor.de/ch340-usb-to-ttl-converter-uart-module-ch340g-3-3-v-5-5-v) to program Atmel Tiny devices, you just need to connect TX and RX lines, they are actually already have a 1.5k output resistor on my connector, so just connect them and don't forget to connect GND of the converter with GND of the PCB with Tiny MCU. There is also a possibility to supply Tiny with the voltage from the CH340 converter, so this adapter seems to be a good choice to start with programming of AVR devices.

<pre>
                        Vcc                     Vcc
                        +-+                     +-+
                         |                       |
 +---------------------+ |                       | +--------------------+
 | CH340 converter     +-+                       +-+  AVR Tiny device   |
 |                     |                           |                    |
 |                  TX +---+-----------------------+ UPDI               |
 |                     |   |                       |                    |
 |                     |   |                       |                    |
 |                  RX +---+                       |                    |
 |                     |                           |                    |
 |                     +---+-----------------------+                    |
 +---------------------+   |                       +--------------------+
                          +-+
                         GND

</pre>
**Warning!!!** If you don't have any output resistors on your CH340 board, please mount them!

## Additional features were added:
	- reading content of the flash memory from the MCU
	- reading fuses
	- different levels of logging
	- faster programming/reading (about 6 seconds for the whole Tiny1616)
	- many additional error messages
	- locking/unlocking MCU

## A brief description of all available options.

	-b BAUDRATE - set COM baudrate (default=115200)
	-d DEVICE   - target device (tinyXXX)
	-c COM_PORT - COM port to use (Win: COMx | *nix: /dev/ttyX)
	-e          - erase device
	-fw X:0xYY  - write fuses (X - fuse number, 0xYY - hex value)
	-fr         - read all fuses
	-h          - show this help screen
	-ls         - lock device
	-lr         - unlock device
	-mX         - set logging level (0-all/1-warnings/2-errors)
	-r FILE.HEX - Hex file to read MCU flash into
	-w FILE.HEX - Hex file to write to MCU flash
	
  
#### Examples:

    Erase Flash memory:
        updiprog.exe -c COM10 -d tiny81x -e
    
    Program Flash memory from file tiny_fw.hex:
        updiprog.exe -c COM10 -d tiny81x -w tiny_fw.hex
		
    Read Flash memory to file tiny_fw.hex:
        updiprog.exe -c COM10 -d tiny81x -r tiny_fw.hex
		
	Read all fuses:
		updiprog.exe -c COM10 -d tiny81x -fr
		
	Write 0x04 to fuse number 1 and 0x1b to fuse number 5:
		updiprog.exe -c COM10 -d tiny81x -fw 1:0x04 5:0x1b

## UPDIlib and UPDIgui

UPDIlib is a dynamically linking library and API for working with the UPDInterface. As an example of using the library, UPDIgui is presented - a program written in Lazarus for changing fuses, writing, reading and verifying the memory of AVR devices.
