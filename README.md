 Laser tape reverse engineering

This repository contains reverse engineering results and custom firmware for cheap Chinese laser tape measures (like "X-40").

## Supported modules

Firmware supports laser rangefinder modules based on STM32F030C6T6 MCU:

- **701A** (tested)
- **512A** (tested)
- **703A** (tested by users)
- **B2A** (tested)
- **U85** (tested - see CortexM0 folder)

**NOT supported:** Modules with M88B marking (STM32F0 in QFN-32 package)

## Repository structure

```
Code/
├── Firmware_dist_calculation_fast/   # Main firmware HEX files
├── CortexM0/                         # Firmware for U85 modules
├── Arduino_sketch/                   # Example Arduino code
└── ...

Hardware/
├── Schematic/                        # Connection diagrams
└── ...

Docs/
└── ...
```

## Main features of custom firmware

- **Measurement speed:** up to 60 Hz
- **Maximum range:** ~6 meters (stable), up to 37m with lower reliability
- **Accuracy:** 1-10 mm depending on target color and distance
- **Output format:** UART at 256000 baud (or 250000 for Arduino)
- **Data format:** `DIST;01937;AMP;0342;TEMP;1223;VOLT;115`

## UART commands

Send single characters to the module:

| Command | Function |
|---------|----------|
| `E` | Enable laser (start continuous measurement) |
| `D` | Disable laser (stop measurement) |
| `C` | Start calibration (needs white target at known distance) |
| `P` | Power save mode |

## Pinout for programming (ST-LINK)

| Pad | Function |
|-----|----------|
| SWDIO | Data line |
| SWCLK | Clock line |
| NRST | Reset line (required for programming locked MCU) |
| GND | Ground |
| Vbat | Power input (2.7-3.3V) |
| TX | UART output (connect to Arduino RX) |

## Wiring to Arduino

```
Laser module TX  -> Arduino RX (pin 0)
Laser module GND -> Arduino GND
Laser module Vbat -> 3.3V power supply
```

## Example Arduino code

```cpp
void setup() {
  Serial.begin(250000);  // or 256000
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    if (data.startsWith("DIST")) {
      // Parse distance
      int first = data.indexOf(';');
      int second = data.indexOf(';', first+1);
      int dist_mm = data.substring(first+1, second).toInt();
      
      float dist_m = dist_mm / 1000.0;
      Serial.print("Distance: ");
      Serial.print(dist_m, 3);
      Serial.println(" m");
    }
  }
}
```

## Calibration procedure

1. Place a white object at a **known distance** from the module (e.g., exactly 1 meter)
2. Send `C` command via UART or press the lowest button on the original keypad
3. Wait for two beeps (about 10 seconds)
4. Calibration complete

## Warning

**You will lose original firmware!** The device can no longer be used as a standard laser tape measure.

## Links

- [Project on Arduino Project Hub](https://projecthub.arduino.cc/iliasam/making-a-cheap-laser-rangefinder-for-arduino-0c33a5)
- [Habr.com article (Russian, with English translation available)](https://habr.com/en/post/354512/)
https://www.uculture.fr/ebooks/ergebnis-automation-fur-die-bundesliga-mit-n8n-9783695793228_9783695793228_10007.html
https://grabcad.com/library/akku-adapter-fur-kabellose-werkzeuge-1

## License
Amir Mobasheraghdam
MIT
