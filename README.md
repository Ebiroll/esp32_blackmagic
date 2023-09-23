# Blackmagic Probe for ARM running on esp32 hardware

Based on the ESP8266 black magic port, only provides SWD ARM Cortex-M Debug Interface

It provides a wifi based, debug probe for ARM i.e. ST32L1 cortex processors
https://github.com/markrages/blackmagic/tree/a1d5386ce43189f0ac23300bea9b4d9f26869ffb/src/platforms/esp8266


# Changes
 I merged JTAG support for riscv-esp32c3 however this is not tested.



 If you connect an STM32 board and put it in boot mode, then you might be able to query some information 
 with uart_scan.

 You need to connect the UART pins


# Merges latest from black magic main repo 
Sept 23 2023

# Platform IO
The latest changes are tested with ESP32-C3

I also managed to build with an esp32-S3

Then I had to preform a workaround, by setting board to
board = esp32s3-qio

If you have problems, Use Platform, Run Menuconfig
Here you can change settings,
Component config → ESP System Settings ,  Initialize Task Watchdog Timer on startup
Dsiable the watchdog or find a way to prevent it from triggereing.

# Using this as  
You can use this software as one click debug, for a platform io project

debug_tool = blackmagic
debug_port = 192.168.4.1:2345

Example config 
```
[platformio]
src_dir = Src
include_dir = Inc

[env:blackpill_f411ce]
platform = ststm32
board = blackpill_f411ce

; change microcontroller
board_build.mcu = stm32f411ceu6

; change MCU frequency
board_build.f_cpu = 84000000L
framework =  stm32cube

debug_tool = blackmagic
debug_port = 192.168.4.1:2345
```
# Up to date BMP
This repository is not updated with latest changes in BMP
This other repository contains the latest version of BMP source https://github.com/Ebiroll/blackmagic

In order to build this repository in linux, do.
```
      > . ~/esp/esp-idf/setup.sh
      > cd src/platforms/esp32
      #Check the platform.h files amd make sure that the pins are OK.
      # check main.c for password and SSID of your wifi
      #Run build script.
      > build-esp32.sh
      #Upload the
```
However espressif might have changed this behaviour and it is might not be possible to build now.
You must also pull the changes from upstream repo.


# Status

Now it seems to work, I tried a RAK811 target. And the targets found in my arm_test repo
The pins are defined here,
http://docs.rakwireless.com/en/RAK811%20TrackerBoard/Software%20Development/RAK811%20TrackerBoard%20User%20manual%20V1.1.pdf

This is the debug compiled source code I use,
https://github.com/Ebiroll/RAK811_BreakBoard

So
```
GND on ESP32 connects to GND on the RAK board, opposite to the boot pins
PIN 8 on ESP32-C3 connects to SWD_CLK
PIN 10 on ESP32-c3 connects to SWD_TMS
```
Pins are changed in platform.h

```
I (3119) event: sta ip: 192.168.1.117, mask: 255.255.255.0, gw: 192.168.1.1
I (3119) blackmagic: Connected to AP
I (19827) gpio: GPIO[8]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
I (19827) gpio: GPIO[10]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 
```



# Start the debugger,
```
arm-none-eabi-gdb .pioenvs/rak811/firmware.elf

target  extended-remote 192.168.1.125:2345

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
#define TRACESWO_PIN 6
// Workaround for driver, and to try STM info polling,
// It also allows you to use the UART from the debugger.
#define TRACESWO_DUMMY_TX 4
```
    
Note that the debugger needs to be attached in order to get output on the serial device.
Here is an example of how to set up the swo for UART mode trace,
https://github.com/Ebiroll/beer_tracker/blob/master/RAK811-Tracker/src/swo.c
Add this,
```
#define CPU_CORE_FREQUENCY_HZ 16000000 /* CPU core frequency in Hz 32Mhz */
   SWO_Init(0x1, CPU_CORE_FREQUENCY_HZ);
```
However now it is more useful, as extra uart port if you do
```
(gdb) mon traceswo 115200
(gdb) mon uart_send Hello there

The uart data received within 500 ms will be printed in the debugger.
```



Here are some more useful information ow what is possible.
https://github.com/orbcode/orbuculum

https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/

To start trace , do
```
(gdb) monitor traceswo 115200
```


# Quicker download
```
arm-none-eabi-gdb .pioenvs/rak811/firmware.elf -ex 'target  extended-remote 192.168.4.1:2345'

(gdb) monitor swdp_scan
(gdb) attach 1
(gdb) load
(gdb) b main
(gdb) c


``` Succesfull boot
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x1 (POWERON),boot:0xd (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5820,len:0x1704
load:0x403cc710,len:0x968
load:0x403ce710,len:0x2f68
entry 0x403cc710
I (30) boot: ESP-IDF 5.0.2 2nd stage bootloader
I (30) boot: compile time Aug 31 2023 08:03:44
I (30) boot: chip revision: v0.3
I (33) boot.esp32c3: SPI Speed      : 80MHz
I (38) boot.esp32c3: SPI Mode       : DIO
I (43) boot.esp32c3: SPI Flash Size : 4MB
I (47) boot: Enabling RNG early entropy source...
I (53) boot: Partition Table:
I (56) boot: ## Label            Usage          Type ST Offset   Length
I (64) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (71) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (79) boot:  2 factory          factory app      00 00 00010000 00100000
I (86) boot: End of partition table
I (90) esp_image: segment 0: paddr=00010020 vaddr=3c0b0020 size=2aa68h (174696) map
I (126) esp_image: segment 1: paddr=0003aa90 vaddr=3fc91600 size=02a40h ( 10816) load
I (129) esp_image: segment 2: paddr=0003d4d8 vaddr=40380000 size=02b40h ( 11072) load
I (134) esp_image: segment 3: paddr=00040020 vaddr=42000020 size=aea44h (715332) map
I (255) esp_image: segment 4: paddr=000eea6c vaddr=40382b40 size=0e9b8h ( 59832) load
I (272) boot: Loaded app from partition at offset 0x10000
I (272) boot: Disabling RNG early entropy source...
I (283) cpu_start: Unicore app
I (284) cpu_start: Pro cpu up.
I (292) cpu_start: Pro cpu start user code
I (292) cpu_start: cpu freq: 160000000 Hz
I (293) cpu_start: Application information:
I (295) cpu_start: Project name:     c3_blackmagic
I (301) cpu_start: App version:      1
I (305) cpu_start: Compile time:     Aug 31 2023 08:03:31
I (311) cpu_start: ELF file SHA256:  000bed43aadaae46...
I (317) cpu_start: ESP-IDF:          5.0.2
I (322) cpu_start: Min chip rev:     v0.3
I (327) cpu_start: Max chip rev:     v0.99 
I (332) cpu_start: Chip rev:         v0.3
I (337) heap_init: Initializing. RAM available for dynamic allocation:
I (344) heap_init: At 3FC98F40 len 000437D0 (269 KiB): DRAM
I (350) heap_init: At 3FCDC710 len 00002950 (10 KiB): STACK/DRAM
I (357) heap_init: At 50000020 len 00001FE0 (7 KiB): RTCRAM
I (364) spi_flash: detected chip: winbond
I (368) spi_flash: flash io: dio
I (372) sleep: Configure to isolate all GPIO pins in sleep state
I (378) sleep: Enable automatic switching of GPIO sleep configuration
I (385) app_start: Starting scheduler on CPU0
I (390) main_task: Started on CPU0
I (390) main_task: Calling app_main()
I (400) pp: pp rom version: 9387209
I (400) net80211: net80211 rom version: 9387209
I (410) wifi:wifi driver task: 3fca1d8c, prio:23, stack:6656, core=0
I (420) wifi:wifi firmware version: b2f1f86
I (420) wifi:wifi certification version: v7.0
I (420) wifi:config NVS flash: enabled
I (420) wifi:config nano formating: disabled
I (420) wifi:Init data frame dynamic rx buffer num: 32
I (430) wifi:Init management frame dynamic rx buffer num: 32
I (430) wifi:Init management short buffer num: 32
I (440) wifi:Init dynamic tx buffer num: 32
I (440) wifi:Init static tx FG buffer num: 2
I (450) wifi:Init static rx buffer size: 1600
I (450) wifi:Init static rx buffer num: 10
I (450) wifi:Init dynamic rx buffer num: 32
I (460) wifi_init: rx ba win: 6
I (460) wifi_init: tcpip mbox: 32
I (470) wifi_init: udp mbox: 6
I (470) wifi_init: tcp mbox: 6
I (470) wifi_init: tcp tx win: 5744
I (480) wifi_init: tcp rx win: 5744
I (480) wifi_init: tcp mss: 1440
I (490) wifi_init: WiFi IRAM OP enabled
I (490) wifi_init: WiFi RX IRAM OP enabled
I (500) phy_init: phy_version 970,1856f88,May 10 2023,17:44:12
I (540) wifi:mode : softAP (7c:df:a1:b4:91:45)
I (540) wifi:Total power save buffer number: 16
I (540) wifi:Init max length of beacon: 752/752
I (540) wifi:Init max length of beacon: 752/752
I (550) esp_netif_lwip: DHCP server started on interface WIFI_AP_DEF with IP: 192.168.4.1
I (560) blackmagic: wifi_init_softap finished. SSID:blackmagic password:sesam1234 channel:7
```


# Broken devkitc-02
I never got it to work, probably due to Chip rev:   v0.2
Disabling Nano did not work
https://github.com/espressif/esp-idf/issues/9631
```
ESP-ROM:esp32c3-20200918
Build:Sep 18 2020
rst:0x3 (RTC_SW_SYS_RST),boot:0xc (SPI_FAST_FLASH_BOOT)
Saved PC:0x400483a0
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5820,len:0x1704
load:0x403cc710,len:0x968
load:0x403ce710,len:0x2f68
entry 0x403cc710
I (34) boot: ESP-IDF 5.0.2 2nd stage bootloader
I (35) boot: compile time Sep 23 2023 01:26:08
I (35) boot: chip revision: v0.2
I (37) boot.esp32c3: SPI Speed      : 80MHz
I (42) boot.esp32c3: SPI Mode       : DIO
I (47) boot.esp32c3: SPI Flash Size : 4MB
I (52) boot: Enabling RNG early entropy source...
I (57) boot: Partition Table:
I (61) boot: ## Label            Usage          Type ST Offset   Length
I (68) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (75) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (83) boot:  2 factory          factory app      00 00 00010000 00100000
I (90) boot: End of partition table
I (95) esp_image: segment 0: paddr=00010020 vaddr=3c0b0020 size=27970h (162160) map
I (129) esp_image: segment 1: paddr=00037998 vaddr=3fc91600 size=02a40h ( 10816) load
I (131) esp_image: segment 2: paddr=0003a3e0 vaddr=40380000 size=05c38h ( 23608) load
I (139) esp_image: segment 3: paddr=00040020 vaddr=42000020 size=ad268h (709224) map
I (256) esp_image: segment 4: paddr=000ed290 vaddr=40385c38 size=0b8c0h ( 47296) load
I (271) boot: Loaded app from partition at offset 0x10000
I (271) boot: Disabling RNG early entropy source...
I (282) cpu_start: Unicore app
I (283) cpu_start: Pro cpu up.
I (291) cpu_start: Pro cpu start user code
I (291) cpu_start: cpu freq: 160000000 Hz
I (291) cpu_start: Application information:
I (294) cpu_start: Project name:     esp32_blackmagic
I (300) cpu_start: App version:      a35d2ae-dirty
I (305) cpu_start: Compile time:     Sep 23 2023 01:25:52
I (311) cpu_start: ELF file SHA256:  5c97ecc1d60d5d0c...
I (317) cpu_start: ESP-IDF:          5.0.2
I (322) cpu_start: Min chip rev:     v0.3
I (327) cpu_start: Max chip rev:     v0.99 
I (332) cpu_start: Chip rev:         v0.2
I (337) heap_init: Initializing. RAM available for dynamic allocation:
I (344) heap_init: At 3FC98F40 len 000437D0 (269 KiB): DRAM
I (350) heap_init: At 3FCDC710 len 00002B50 (10 KiB): STACK/DRAM
I (357) heap_init: At 50000020 len 00001FE0 (7 KiB): RTCRAM
I (364) spi_flash: detected chip: generic
I (368) spi_flash: flash io: dio
I (372) sleep: Configure to isolate all GPIO pins in sleep state
I (378) sleep: Enable automatic switching of GPIO sleep configuration
I (385) app_start: Starting scheduler on CPU0
I (390) main_task: Started on CPU0
I (390) main_task: Calling app_main()
I (400) blackmagic: Soft AP mode
I (400) pp: pp rom version: 8459080
I (410) net80211: net80211 rom version: 8459080
I (420) wifi:wifi driver task: 3fca1bc0, prio:23, stack:6656, core=0
I (420) wifi:wifi firmware version: b2f1f86
I (420) wifi:wifi certification version: v7.0
I (420) wifi:config NVS flash: enabled
I (420) wifi:config nano formating: disabled
I (430) wifi:Init data frame dynamic rx buffer num: 32
I (430) wifi:Init management frame dynamic rx buffer num: 32
I (440) wifi:Init management short buffer num: 32
I (440) wifi:Init dynamic tx buffer num: 32
I (450) wifi:Init static tx FG buffer num: 2
I (450) wifi:Init static rx buffer size: 1600
I (460) wifi:Init static rx buffer num: 10
I (460) wifi:Init dynamic rx buffer num: 32
I (460) wifi_init: rx ba win: 6
I (470) wifi_init: tcpip mbox: 32
I (470) wifi_init: udp mbox: 6
I (470) wifi_init: tcp mbox: 6
I (480) wifi_init: tcp tx win: 5744
I (480) wifi_init: tcp rx win: 5744
I (490) wifi_init: tcp mss: 1440
I (490) wifi_init: WiFi IRAM OP enabled
I (490) wifi_init: WiFi RX IRAM OP enabled
I (500) phy_init: phy_version 970,1856f88,May 10 2023,17:44:12
W (510) phy_init: failed to load RF calibration data (0x1102), falling back to full calibration
Guru Meditation Error: Core  0 panic'ed (Illegal instruction). Exception was unhandled.

Core  0 register dump:
MEPC    : 0x40001be4  RA      : 0x4209e29c  SP      : 0x3fca1a10  GP      : 0x3fc91e00  
TP      : 0x3fc7aa58  T0      : 0x40057fa6  T1      : 0x0000000f  T2      : 0xffffffff  
S0/FP   : 0x3fc988e4  S1      : 0x3fc99000  A0      : 0x3fc928c8  A1      : 0x00000000  
A2      : 0x00000000  A3      : 0x3fca1a90  A4      : 0x00000042  A5      : 0x00000001  
A6      : 0x00000000  A7      : 0x0000000a  S2      : 0x3fc99000  S3      : 0x00000002  
S4      : 0x3fca7308  S5      : 0x3c0d3524  S6      : 0x00000002  S7      : 0x3fce0000  
S8      : 0x3ff1b000  S9      : 0x3fce0000  S10     : 0x3fcdf8d4  S11     : 0x00000000  
T3      : 0x00000000  T4      : 0x00000000  T5      : 0x00006369  T6      : 0x67616d6b  
MSTATUS : 0x00000081  MTVEC   : 0x40380001  MCAUSE  : 0x00000002  MTVAL   : 0x00000000  
MHARTID : 0x00000000  

Stack memory:
3fca1a10: 0x52520002 0x484c4c50 0x4648484c 0x4446464a 0x00000000 0x00000000 0x00000000 0x00000000
3fca1a30: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1a50: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1a70: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1a90: 0x00000042 0x3fce0000 0x3fce0000 0x00000002 0x00001102 0x3c0d3524 0x3fca7308 0x420a8a20
3fca1ab0: 0x00000000 0xffffffff 0x76a1df7c 0x4038cc05 0x00000001 0x3fc96000 0x3fce0000 0x3fc967e4
3fca1ad0: 0x00000001 0x3fc96000 0x3fce0000 0x420a8bfe 0x00000001 0x3fc96000 0x00000000 0x42073770
3fca1af0: 0x3fc967e4 0xffffffff 0x00000000 0x00000001 0x3fc967e4 0x00000002 0x00000000 0x4207404c
3fca1b10: 0x3fc967e4 0x00000000 0x3fc98520 0x3ff1b594 0x3fc967e4 0xffffffff 0x3fca72ec 0x420725a4
3fca1b30: 0x00000000 0x3fcdf918 0x3fce0000 0x4003fe8a 0x00000000 0x00000000 0x00000006 0x3fca72ec
3fca1b50: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1b70: 0x00000000 0x00000000 0x00000000 0x4038a622 0x00000000 0x00000000 0x00000000 0x00000000
3fca1b90: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5
3fca1bb0: 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5 0x00000154 0x3fca14c0 0x3fc98f54 0x3fc95100 0x3fc95100
3fca1bd0: 0x3fca1bc0 0x3fc950f8 0x00000002 0x3fc9faf8 0x3fc9faf8 0x3fca1bc0 0x00000000 0x00000017
3fca1bf0: 0x3fca01bc 0x69666977 0x40b5e300 0x70ee1973 0x000ef421 0x00000000 0x3fca1bb0 0x00000017
3fca1c10: 0x00000001 0x00000000 0x00000000 0x00000000 0x3fc99940 0x3fc999a8 0x3fc99a10 0x00000000
3fca1c30: 0x00000000 0x00000001 0x00000000 0x00000000 0x00000000 0x4208e6d8 0x00000000 0x00000000
3fca1c50: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1c70: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1c90: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1cb0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1cd0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1cf0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fca1d10: 0x3f000000 0x00000054 0x3fca1d18 0x3fca1d18 0x3fca1d18 0x3fca1d18 0x00000000 0x3fca1d30
3fca1d30: 0xffffffff 0x3fca1d30 0x3fca1d30 0x00000000 0x3fca1d44 0xffffffff 0x3fca1d44 0x3fca1d44
3fca1d50: 0x00000000 0x00000001 0x00000000 0x7500ffff 0x00000000 0xb33fffff 0x00000000 0x00000bfc
3fca1d70: 0x6f6d706f 0x00006564 0x2e617473 0x64697373 0x00000000 0x2e617473 0x68747561 0x65646f6d
3fca1d90: 0x00010000 0x00000000 0x00000004 0x00000002 0x3fc96c8c 0x2e617473 0x64697373 0x00000000
3fca1db0: 0x2e617473 0x68747561 0x65646f6d 0x00000000 0x2e617473 0x00240701 0x00000000 0x00000000
3fca1dd0: 0x00000000 0x3fc96c90 0x2e617473 0x68747561 0x65646f6d 0x00000000 0x2e617473 0x64777370
3fca1df0: 0x00000000 0x2e617473 0x00010002 0x00000000 0x00000009 0x00000000 0x3fc96cba 0x2e617473



Rebooting...
ESP-ROM:esp32c3-20200918
Build:Sep 18 2020
rst:0x3 (RTC_SW_SYS_RST),boot:0xc (SPI_FAST_FLASH_BOOT)
Saved PC:0x400483a0
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5820,len:0x1874
load:0x403cc710,len:0xb34
load:0x403ce710,len:0x305c
entry 0x403cc710
␛[0;32mI (34) boot: ESP-IDF 5.0.2 2nd stage bootloader␛[0m
␛[0;32mI (35) boot: compile time Sep 23 2023 01:36:38␛[0m
␛[0;32mI (35) boot: chip revision: v0.2␛[0m
␛[0;32mI (37) qio_mode: Enabling default flash chip QIO␛[0m
␛[0;32mI (43) boot.esp32c3: SPI Speed      : 80MHz␛[0m
␛[0;32mI (48) boot.esp32c3: SPI Mode       : QIO␛[0m
␛[0;32mI (52) boot.esp32c3: SPI Flash Size : 4MB␛[0m
␛[0;32mI (57) boot: Enabling RNG early entropy source...␛[0m
␛[0;32mI (62) boot: Partition Table:␛[0m
␛[0;32mI (66) boot: ## Label            Usage          Type ST Offset   Length␛[0m
␛[0;32mI (73) boot:  0 nvs              WiFi data        01 02 00009000 00006000␛[0m
␛[0;32mI (81) boot:  1 phy_init         RF data          01 01 0000f000 00001000␛[0m
␛[0;32mI (88) boot:  2 factory          factory app      00 00 00010000 00100000␛[0m
␛[0;32mI (96) boot: End of partition table␛[0m
␛[0;32mI (100) esp_image: segment 0: paddr=00010020 vaddr=3c0b0020 size=27970h (162160) map␛[0m
␛[0;32mI (131) esp_image: segment 1: paddr=00037998 vaddr=3fc91600 size=02a40h ( 10816) load␛[0m
␛[0;32mI (133) esp_image: segment 2: paddr=0003a3e0 vaddr=40380000 size=05c38h ( 23608) load␛[0m
␛[0;32mI (141) esp_image: segment 3: paddr=00040020 vaddr=42000020 size=ad268h (709224) map␛[0m
␛[0;32mI (245) esp_image: segment 4: paddr=000ed290 vaddr=40385c38 size=0b8c0h ( 47296) load␛[0m
␛[0;32mI (258) boot: Loaded app from partition at offset 0x10000␛[0m
␛[0;32mI (258) boot: Disabling RNG early entropy source...␛[0m
␛[0;32mI (270) cpu_start: Unicore app␛[0m
␛[0;32mI (270) cpu_start: Pro cpu up.␛[0m
␛[0;32mI (278) cpu_start: Pro cpu start user code␛[0m
␛[0;32mI (278) cpu_start: cpu freq: 160000000 Hz␛[0m
␛[0;32mI (278) cpu_start: Application information:␛[0m
␛[0;32mI (281) cpu_start: Project name:     esp32_blackmagic␛[0m
␛[0;32mI (287) cpu_start: App version:      a35d2ae-dirty␛[0m
␛[0;32mI (293) cpu_start: Compile time:     Sep 23 2023 01:36:22␛[0m
␛[0;32mI (299) cpu_start: ELF file SHA256:  216438bb39bcf38f...␛[0m
␛[0;32mI (305) cpu_start: ESP-IDF:          5.0.2␛[0m
␛[0;32mI (309) cpu_start: Min chip rev:     v0.3␛[0m
␛[0;32mI (314) cpu_start: Max chip rev:     v0.99 ␛[0m
␛[0;32mI (319) cpu_start: Chip rev:         v0.2␛[0m
␛[0;32mI (324) heap_init: Initializing. RAM available for dynamic allocation:␛[0m
␛[0;32mI (331) heap_init: At 3FC98F40 len 000437D0 (269 KiB): DRAM␛[0m
␛[0;32mI (337) heap_init: At 3FCDC710 len 00002B50 (10 KiB): STACK/DRAM␛[0m
␛[0;32mI (344) heap_init: At 50000020 len 00001FE0 (7 KiB): RTCRAM␛[0m
␛[0;32mI (351) spi_flash: detected chip: generic␛[0m
␛[0;32mI (355) spi_flash: flash io: qio␛[0m
␛[0;32mI (359) sleep: Configure to isolate all GPIO pins in sleep state␛[0m
␛[0;32mI (365) sleep: Enable automatic switching of GPIO sleep configuration␛[0m
␛[0;32mI (373) app_start: Starting scheduler on CPU0␛[0m
␛[0;32mI (378) main_task: Started on CPU0␛[0m
␛[0;32mI (378) main_task: Calling app_main()␛[0m
␛[0;32mI (388) blackmagic: Soft AP mode␛[0m
␛[0;32mI (388) pp: pp rom version: 8459080␛[0m
␛[0;32mI (388) net80211: net80211 rom version: 8459080␛[0m
I (408) wifi:wifi driver task: 3fca1bc0, prio:23, stack:6656, core=0
I (408) wifi:wifi firmware version: b2f1f86
I (408) wifi:wifi certification version: v7.0
I (408) wifi:config NVS flash: enabled
I (408) wifi:config nano formating: disabled
I (418) wifi:Init data frame dynamic rx buffer num: 32
I (418) wifi:Init management frame dynamic rx buffer num: 32
I (428) wifi:Init management short buffer num: 32
I (428) wifi:Init dynamic tx buffer num: 32
I (438) wifi:Init static tx FG buffer num: 2
I (438) wifi:Init static rx buffer size: 1600
I (448) wifi:Init static rx buffer num: 10
I (448) wifi:Init dynamic rx buffer num: 32
␛[0;32mI (448) wifi_init: rx ba win: 6␛[0m
␛[0;32mI (458) wifi_init: tcpip mbox: 32␛[0m
␛[0;32mI (458) wifi_init: udp mbox: 6␛[0m
␛[0;32mI (458) wifi_init: tcp mbox: 6␛[0m
␛[0;32mI (468) wifi_init: tcp tx win: 5744␛[0m
␛[0;32mI (468) wifi_init: tcp rx win: 5744␛[0m
␛[0;32mI (478) wifi_init: tcp mss: 1440␛[0m
␛[0;32mI (478) wifi_init: WiFi IRAM OP enabled␛[0m
␛[0;32mI (478) wifi_init: WiFi RX IRAM OP enabled␛[0m
␛[0;32mI (488) phy_init: phy_version 970,1856f88,May 10 2023,17:44:12␛[0m
␛[0;33mW (498) phy_init: failed to load RF calibration data (0x1102), falling back to full calibration␛[0m
Guru Meditation Error: Core  0 panic'ed (Illegal instruction). Exception was unhandled.

```


# C3 schematics for test of JTAG
![C# Schematics](c3-schematics.png)
