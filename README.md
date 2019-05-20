
Based on the ESP8266 black magic port,

It provides a wifi based, debug probe for i.e. ST32L1 cortex processors

https://github.com/markrages/blackmagic/tree/a1d5386ce43189f0ac23300bea9b4d9f26869ffb/src/platforms/esp8266

Now it seems to work, I use the RAK811 target.
The pins are defined here,
http://docs.rakwireless.com/en/RAK811%20TrackerBoard/Software%20Development/RAK811%20TrackerBoard%20User%20manual%20V1.1.pdf

This is the debug compiled source code I use,
https://github.com/Ebiroll/RAK811_BreakBoard

So
```
GND on ESP32 connects to GND on the RAK board, opposite to the boot pins
PIN 22 on ESP32 connects to SWD_CLK
NOTE CHANGED TO 23 HERE!!!
PIN 17 on ESP32 connects to SWD_TMS
```
Pins are changed in platform.h

I (3119) event: sta ip: 192.168.1.117, mask: 255.255.255.0, gw: 192.168.1.1
I (3119) blackmagic: Connected to AP
I (3119) gpio: GPIO[17]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (3129) gpio: GPIO[23]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 


# Start the debugger,
```
arm-none-eabi-gdb .pioenvs/rak811/firmware.elf

target  extended-remote 192.168.1.136:2345

(gdb) monitor help

(gdb) monitor swdp_scan
Target voltage: not supported
Available Targets:
No. Att Driver
 1      STM32L1x

https://github.com/blacksphere/blackmagic/wiki/Frequently-Asked-Questions

(gdb) attach 1

```

Works like charm.

# Trace SWO
It is possible to use trace swo if you configure it to use UART mode and 115200.
You must also define thses in platform.h.

```
#define PLATFORM_HAS_TRACESWO 1
#define TRACESWO_PIN 13
// Workaround for driver
#define TRACESWO_DUMMY_TX 19
```
    
Note that the debugger needs to be attached in order to get output on the serial device.
Here is an example of how to set up the swo for UART mode trace,
https://github.com/Ebiroll/beer_tracker/blob/master/RAK811-Tracker/src/swo.c
Add this,
```
   SWO_Init(0x1, CPU_CORE_FREQUENCY_HZ);
```

Here are some more useful information ow what is possible.
https://github.com/orbcode/orbuculum

To start trace thead, do
```
(gdb) monitor traceswo
```


# Quicker download
```
arm-none-eabi-gdb .pioenvs/rak811/firmware.elf -ex 'target  extended-remote 192.168.1.136:2345'

(gdb) monitor swdp_scan
(gdb) attach 1
(gdb) load
(gdb) b main
(gdb) c


```