# Radtel RT-890 APRS-GPS Custom Firmware

This project is an effort to improve the firmware of the Radtel RT-890 in terms of features and radio performance.

It is based on [DualTachyon's OEFW](https://github.com/OEFW-community/radtel-rt-890-oefw) which is reversed from the original Radtel 1.34 firmware.  
Thanks to him for making this possible!

## Disclaimer
This firmware is a work in progress and could be unstable; it could alter your radio and its data.  
Use at your own risk and remember to [back up your SPI memory](https://github.com/OEFW-community/RT-890-custom-firmware/wiki/SPI) before installing any custom firmware.  DO NOT SKIP THIS STEP.

Transmission on this devices is very bad filtered (HW issue). Harmonics go to air almost freely.

    --        [IS YOUR RESPONSABILITY TO TAKE THIS ON ACCOUNT TO COMPLY WITH LOCAL REGULATIONS IN YOUR COUNTRY]

There is no "free radio" but GMRS, and this devices do not comply whith GMRS specifications.

    --        [DO NOT TRANSMIT IF YOU ARE NOT LICENSED TO]

## omegatee Features / Modifications
- All OEFW features plus:
    - Full rewrite of UART code to support the new command shell and the GPS Receiver
    - Added channel templates for standard APRS frequencies on EUR and USA
    - New keyaction to send Position by APRS (not working yet)
    - Personal ID (editable with CHIRP) used as "myCALL" for APRS. SSID is fixed to 7
    - Startup Label (editable with CHIRP) used as Device Serial Number
    - GPS Time shown on display

- Removed / unbugged:
    - Removed "[DISABLE]" options in menu
    - Fixed "If CurrentDial ("CurrentVfo" in the repos) is "B" and the incoming signal enters on "A", the AM fix does not apply"
    - Corrected FM modulation depth for Wide/Narrow modes
    - Corrected TX Power levels High and Low (set to 5 and 1 W)
    - Removed unuseful functions such as Flashlight, Local Alarm...
    - Removed icons hiding modulation mode indicators
    - Modulation mode is allways shown as "FM" on TX
    - Keyaction for "Modulation" and others set to "roll-on" instead of calling menu

## GPS Receiver HW Implementation

![image](https://github.com/user-attachments/assets/ff4816d5-8ab2-4709-805b-d65616095407)

Do it as you can

## Command Shell
At boot, the device works as usual.
But after receiving two semicolons, enters the SCPI-like shell mode. In this mode, CHIRP will fail. Type EXIT to return to normal operation.
While in Shell mode, device operates normally.

Implemented commands:
- *IDN?        Returns Manufacturer, Model, Serial Number and FW Version.
- *RST          Reboots the device
- RADIO:FREQ?  Returns current RX Frequency, TX Frequency
- RADIO:TX 1    Puts the device on TX (just a millisecond... by now)
- RADIO:TX 0    Puts the device on RX
- GPS:TIME?    Returns GPS Time
- GPS:LAT?      Returns GPS Latitude
- GPS:LON?      Returns GPS Longitude
- EXIT           Exits Shell mode


## Telegram group
If you want to discuss this project, you can join the [Telegram group](https://t.me/RT890_OEFW).


---
_Original OEFW readme_

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

