- setup of the coding environment: see https://github.com/jerabaul29/OpenMetBuoy-v2021a/blob/main/development_environment/setup_platformio_environment/Instructions.md
- compiling on:
  - linux: in platformio.ini, use the linux-only subsection flags for the build_flags
  - other: in platformio.ini, use the dummy subsection flags for the build_flags
- overview of the wiring (from https://github.com/jvoermans/Wind_Input_V1/issues/14):

![logger_wiring](https://user-images.githubusercontent.com/8382834/211499048-af84b316-dbda-4347-968a-b339c1cd982c.png) : note: this is partially out of date

GNSS wiring: GNSS | SFA
- TX | RX1
- RX | TX1
- EN | D2
- GND | GND
- VIN | 3.3V
GNSS pull down: 1kOhm (min), 2kOhm (max), GNSS EN | GNSS GND

NOTE: the GNSS gets fully turned off, so to get fixes fast enough, it MUST have the battery!

SD card: format to FAT

NOTE: for some reason I ignore, sometimes the files are not visible to my default file explorer GUI, but they are well visible to the ls command.

SD wiring (I use an Adafruit SD breakout, SD pin names may be a bit different with other breakouts, but the idea is the same): SD | SFA
- GND | GND
- 3v | 3.3V
- CS | D8
- CLK | SCK
- DO | MISO
- DI | MOSI

TODO:
- frequency of measurements?
- resolution of each kind of data?
