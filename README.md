
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
PIN 17 on ESP32 connects to SWD_TMS
```


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

# Quicker download
```
arm-none-eabi-gdb .pioenvs/rak811/firmware.elf -ex 'target  extended-remote 192.168.1.136:2345'

(gdb) monitor swdp_scan
(gdb) attach 1
(gdb) load
(gdb) b main
(gdb) c


```