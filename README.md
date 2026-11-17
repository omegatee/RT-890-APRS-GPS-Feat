# Radtel RT-890 APRS-GPS Custom Firmware

This project is an effort to improve the firmware of the Radtel RT-890 in terms of features and radio performance.

Modifications are oriented to improve hand-held use in the field. So, no register showing, spectrum, screen gadgets, etc. are managed.

It is based on [DualTachyon's OEFW] (https://github.com/OEFW-community/radtel-rt-890-oefw) which is reversed from the original Radtel 1.34 firmware.  
Thanks to him for making this possible!

## Disclaimer
This firmware is a work in progress and could be unstable; it could alter your radio and its data.  
Use at your own risk and remember to [back up your SPI memory](https://github.com/OEFW-community/RT-890-custom-firmware/wiki/SPI) before installing any custom firmware.  DO NOT SKIP THIS STEP.

Transmission on this devices is very bad filtered (HW issue). Harmonics go to air almost freely.

> [!WARNING]
> IS YOUR RESPONSABILITY TO TAKE THIS ON ACCOUNT TO COMPLY WITH LOCAL REGULATIONS IN YOUR COUNTRY

There is no "free radio" but FRS, PMR446... but this devices do not comply with required specifications.

> [!CAUTION]
> DO NOT TRANSMIT IF YOU ARE NOT LICENSED TO [IN ANY CONTRY]

# omegatee Features / Modifications

## 2026 Commit
- Independent Dial A and Dial B for VFO or MEM mode
- MENU key now switches between VFO and MEM modes
    - Menu comes from MENU key long press
- Improved GNSS precision
- Corrected APRS Beacon timeouts
- Added SCPI commands:
    - APRS:SEND POS
    - APRS:SEND MSG
    - APRS:SEND STS
<img width="565" height="478" alt="image" src="https://github.com/user-attachments/assets/d4852c91-04fc-4e4f-9edf-0c01d8216bbc" />

  
## First Commit
- All OEFW features plus:
    - Full rewrite of UART code to support the new command shell and the GNSS Receiver
	- Added functions to manage APRS and AFSK modulation.
    - Added channel templates for standard APRS frequencies on EUR and USA
    - New keyaction to send Test Message by APRS
    - New Menu to activate APRS Position Beacon
    - Personal ID (editable with CHIRP) used as "myCALL" for APRS. SSID is fixed to 7
	- Channel 899 used for APRS operation
    - Startup Label (editable with CHIRP) used as Device Serial Number
    - GNSS Time shown on display
	- Four TX power levels, 6W, 2.5W, 500mW and zero.
	- Three bandwith values, 12.5k, 25k and 125k (125k only on TX yet)
	- Improved AM Fix algorithm, faster, smaller and not altering RSSI indication
	- Added "1" for frequencies higher than 999 MHz
	- Removed "0" for frequencies lower than 100 MHz
	- Added live decode of incoming CTCSS
	- Added decoding of incoming FSK GNSS Position frames
	- RSSI indicator operates on RX even if no incoming signal detected

- Removed / unbugged:
    - Removed "[DISABLE]" options in menu
    - Fixed "If CurrentDial ("CurrentVfo" in the repos) is "B" and the incoming signal enters on "A", the AM fix does not apply"
    - Removed unuseful functions such as Flashlight, Local Alarm...
    - Removed icons hiding modulation mode indicators
    - Modulation mode is allways shown as "FM" on TX
    - Keyaction for "Modulation" and others set to "roll-on" instead of calling menu

## Flashing
Recommended flashing tools:
- https://github.com/OEFW-community/radtel-rt-890-flasher/releases/tag/first-release
- https://github.com/omegatee/RT-4D_Flasher/releases/tag/v2.1.0-beta

## GNSS Receiver HW Implementation
<img width="565" height="478" alt="image" src="https://github.com/user-attachments/assets/ff4816d5-8ab2-4709-805b-d65616095407" />

Do it as you can.
My option is to install the module on top, with a small hole for the wires:
<img width="447" height="424" alt="image" src="https://github.com/user-attachments/assets/c12338fb-fee0-4279-bf1d-1e88a7bddaca" />

[NOTE:] GNSS board baudrate is fixed on FW. Trim line 203 in radio/hardware.c as your need.


## Command Shell
At boot, the device works as usual.
But after receiving two semicolons, enters the SCPI-like shell mode.
While in Shell mode, device operates normally, except CHIRP that will fail. Type EXIT to return to normal operation.
.

Implemented commands:
- *IDN?                  Returns Manufacturer, Model, Serial Number and FW Version.
- *RST                   Reboots the device
- RADIO:FREQ?            Returns current RX Frequency, TX Frequency
- RADIO:TX 1             Puts the device on TX (just a millisecond... by now)
- RADIO:TX 0             Puts the device on RX
- RADIO:AGC n            Sets AGC to level n (0=auto)
- APRS:SEND POS          Sends APRS Position Packet
- GNSS:TIME?             Returns GNSS Time
- GNSS:LAT?              Returns GNSS Latitude
- GNSS:LON?              Returns GNSS Longitude
- REG XX?                Return XX register value (in hex, zero-padded)
- REG XX YYYY            Writes YYYY in register XX (in hex, zero-padded)
- EXIT                   Exits Shell mode


## Telegram group
If you want to discuss this project, you can join the [Telegram group](https://t.me/RT890_OEFW).

## Other recommended FW for RT-890
https://github.com/M7OCM/mono890

==============================================================================================================
# _Original DualTachyon OEFW readme_

# Support

* If you like my work, you can support me through https://ko-fi.com/DualTachyon

# Open reimplementation of the Radtel RT-890 v1.34 firmware

This repository is a preservation project of the Radtel RT-890 v1.34 firmware.
It is dedicated to understanding how the radio works and help developers making their own customisations/fixes/etc.
It is by no means fully understood or has all variables/functions properly named, as this is best effort only.
As a result, this repository will not include any customisations or improvements over the original firmware.

# Compiler

arm-none-eabi GCC version 10.3.1 is recommended, which is the current version on Ubuntu 22.04.03 LTS.
Other versions may generate a flash file that is too big.
You can get an appropriate version from: https://developer.arm.com/downloads/-/gnu-rm

# Building

To build the firmware, you need to fetch the submodules and then run make:
```
git submodule update --init --recursive --depth=1
make
```

# Flashing

* Use the firmware.bin file with either [RT-890-Flasher](https://github.com/OEFW-community/radtel-rt-890-flasher) or [RT-890-Flasher-CLI](https://github.com/OEFW-community/radtel-rt-890-flasher-cli)

# License

Copyright 2023 Dual Tachyon
https://github.com/DualTachyon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

