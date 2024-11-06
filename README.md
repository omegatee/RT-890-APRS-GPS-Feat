** THIS REPO IS NOT WORKING **
Just used as backup



## Radtel RT-890 Custom APRS&GPS Firmware

This project is an effort to improve the firmware of the Radtel RT-890 in terms of features and radio performance.

It is based on [DualTachyon's OEFW](https://github.com/OEFW-community/radtel-rt-890-oefw) which is reversed from the original Radtel 1.34 firmware.  
Thanks to him for making this possible!

And on [Wiki in this repository](https://github.com/OEFW-community/RT-890-custom-firmware)

## Disclaimer
This firmware is a work in progress and could be unstable; it could alter your radio and its data.  
Use at your own risk and remember to [back up your SPI memory](https://github.com/OEFW-community/RT-890-custom-firmware/wiki/SPI) before installing any custom firmware.  DO NOT SKIP THIS STEP.

## Changes and new Features
- RX & TX frequency can be set from 10 to 1300 MHz (results may vary).
	- ### NOTE: Is your responsability to use this ability in accordance to your country laws ###
- Removed unuseful functions such as FLashlight, Local Alrarm, etc.
- Removed display icons hidding modulation mode indicators
- Changed WorkMode to WorkModeA and WorkModeB, for future splitting of VFO/CHAN mode on both dials
- Full rework of UART functions
  - Added serial command prompt at UART1
  - Added UART2 to manage the GPS Receiver communication
- Modified "roll-on" selection (instead of calling menu) for keyactions Modulation, TX Power...
- Personal ID (editable with CHIRP) used as source address in APRS (ssid fixed to 7)
- Startup Label (editable with CHIRP) used as device serial number
- Added channel templates for standard APRS frequencies in EUR and USA
- Added GPS Time presentation on display (previous indicators displaced to get room)
- Added keyaction to manually send position by APRS
- Added task to implement APRS Beacon

## Removed Bugs
- If you set ENABLE_NOAA to 0, linker fails
- If CurrentDial ("CurrentVfo" in the repos) is "B" and the incoming signal enters on "A", the AM fix does not apply
- Removed annoying "[DISABLED]" items from menu options
- Corrected modulation index to comply with 12,5/25 kHz bandwidth

## Usage and feature instructions
See the [Wiki in this repository](https://github.com/OEFW-community/RT-890-custom-firmware/wiki) for detailed usage instructions.

## Pre-built firmware
You can find pre-built firmwares in the [Actions](https://github.com/OEFW-community/RT-890-custom-firmware/actions)

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

