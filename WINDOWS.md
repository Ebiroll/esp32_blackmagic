How to Edit and Upload the code without a J-Link on Win10 and Atom on WINDOWS

Download Atom.

A hackable text editor for the 21st Century

https://atom.io/


Once Download, install and open Atom up.


Then go to settings > install, search for platformio

Install the following.
platformio-ide-terminal
platformio-ide
platformio-ide-debugger

Once done, download and install the comm drivers for the RAK811
http://docs.rakwireless.com/en/RAK811%20TrackerBoard/Tool/CP210x_Windows_Drivers.zip1

Then download the git from OlofAst
GitHub5
Ebiroll/RAK811_BreakBoard

RAK811_BreakBoard for platformio from https://github.com/RAKWireless/RAK811_BreakBoard

open Atom, on the PlatformIO Home Tab, go to Platform

Search ST STM32, Install
Then click Reveal

It will open up a Explorer window, then open folder ststm32/boards
copy the file from OlofAst’s git, rak811.json to the ststm32/boards folder.

Then in Atom, click open folder.

Open the folder RAK811_BreakBoard from OlofAst’s git.

Then open the Terminal in Atom

To edit the DevEUI and etc… open the file src\Commissioning.h

Save, and close the tab.

Now to Compile.

In the Terminal window in Atom type: pio run

if you see SUCCESS, compiling went ok

Calculating size .pioenvs\rak811\firmware.elf
text       data     bss     dec     hex filename
53236      1628    5244   60108    eacc .pioenvs\rak811\firmware.elf
===[SUCCESS] Took 12.25 seconds 

Now to Upload .bin file to the board

Change the jumper on the board.
unnamed
unnamed.jpg4032x3024 593 KB

Connect the usb, go to Device manager and check what is you comm port number for the board.

Then in the terminal window In Atom, run the following

stm32flash_src/stm32flash.exe -w .pioenvs/rak811/firmware.bin COM5

This will upload the .bin file, the result should be

stm32flash 0.5

http://stm32flash.sourceforge.net/

Using Parser : Raw BINARY
Interface serial_w32: 115200 8E1
Version      : 0x31
Option 1     : 0x00
Option 2     : 0x00
Device ID    : 0x0429 (STM32L1xxx6(8/B)A)
- RAM        : 32KiB  (4096b reserved by bootloader)
- Flash      : 128KiB (size first sector: 16x256)
- Option RAM : 32b
- System RAM : 4KiB
Write to memory
Data size: 54868 bytes
Erasing memory
Wrote address 0x0800d654 (100.00%) Done.

You should all be done. Unplug, change the jumper back to original position.

Power up the board and it should connect to the AP.

To monitor the board, download Uart Terminal, Connect to the Comm port of the board, at baud rate 115200
SourceForge2
UART Terminal

Download UART Terminal for free. None

RAK811 BreakBoard soft version: 1.0.2

Selected LoraWAN 1.0.2 Region: EU868

OTAA:
Dev_EUI: XX XX XX XX XX XX XX XX
AppEui: XX XX XX XX XX XX XX XX
AppKey: XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX
OTAA Join Start…
OTAA Join Success