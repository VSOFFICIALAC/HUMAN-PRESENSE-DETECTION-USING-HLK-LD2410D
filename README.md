# Human Presence and Proximity Indicator — HLK-LD2410D

An Arduino project intended to turn human-presence and distance information into an animated, easy-to-read visual indicator using 16 WS2812B addressable RGB LEDs.

The intended display is **red for someone nearby**, **yellow for someone at an intermediate distance**, and **blinking green when no one is detected or the detected distance is beyond the warning thresholds**.

> **Current implementation status:** The supplied sketch implements LED animations and a permissive, line-based text parser. It does **not** decode the HLK-LD2410D's native binary radar protocol. Direct connection to a stock sensor therefore requires a compatible binary parser or an intermediate device that converts radar reports into the text format described below. No converter is included. Existing Nano build files are supplied, but hardware operation and a fresh compilation have not been verified as part of this documentation.

## Contents

- [Intended purpose](#intended-purpose)
- [Features and scope](#features-and-scope)
- [Project files](#project-files)
- [Hardware requirements](#hardware-requirements)
- [Wiring](#wiring)
- [Software setup and upload](#software-setup-and-upload)
- [How it works](#how-it-works)
- [LED states and distance boundaries](#led-states-and-distance-boundaries)
- [Serial input format](#serial-input-format)
- [Configuration](#configuration)
- [Testing and commissioning](#testing-and-commissioning)
- [Troubleshooting](#troubleshooting)
- [Known limitations](#known-limitations)
- [Possible improvements](#possible-improvements)
- [References and licensing](#references-and-licensing)

## Intended purpose

The project is intended as a local human-presence and proximity demonstration: a radar supplies presence/distance information, an Arduino classifies it, and an LED ring or strip makes the result visible without requiring a computer screen.

Possible uses include:

- A desk or doorway proximity indicator.
- A visual demonstration of distance-based embedded control.
- A classroom project combining serial communication, state management, and addressable LEDs.
- A prototype front end for a future occupancy or automation system.

The current firmware provides a visual indication only. It does not identify people, count occupants, record events, switch appliances, send notifications, or communicate over a network. It is a prototype and should not be relied on as a protective alarm or safety interlock.

## Features and scope

| Item | Current implementation |
| --- | --- |
| Intended sensor | Hi-Link HLK-LD2410D human-presence radar |
| Build target indicated by included files | Arduino AVR Nano |
| LED type | WS2812B, GRB color order |
| LED count | 16 |
| LED data pin | D6 |
| Sensor serial receive/transmit pins | D4 / D3 |
| USB diagnostic serial speed | 115200 baud |
| Sensor-side software serial speed | 115200 baud |
| Distance units used by the sketch | Centimeters |
| Near threshold | 150 cm |
| Far threshold | 250 cm |
| Startup display | White flashes, then green, yellow, and red |
| Normal display | Green blink or red/yellow loading animation |
| Sensor protocol support | Text input only; native radar binary parsing is absent |

## Project files

Paths below are relative to the folder containing this README. The repeated project folder name and the spelling `PRESENSE` are preserved from the supplied files.

```text
.
├── README.md
└── HUMAN DETECTION HLK-LD2410D PROJECT/
    └── HUMAN_PRESENSE_DETECTOR/
        ├── HUMAN_PRESENSE_DETECTOR.ino
        └── build/
            └── arduino.avr.nano/
                ├── HUMAN_PRESENSE_DETECTOR.ino.hex
                ├── HUMAN_PRESENSE_DETECTOR.ino.with_bootloader.hex
                ├── HUMAN_PRESENSE_DETECTOR.ino.with_bootloader.bin
                ├── HUMAN_PRESENSE_DETECTOR.ino.elf
                └── HUMAN_PRESENSE_DETECTOR.ino.eep
```

The `.ino` file is the editable source. The `build` folder contains previously generated firmware artifacts: application HEX, bootloader-inclusive images, an ELF executable/debug artifact, and an EEPROM image. Their presence does not establish that they match the current source or the bootloader on your board. Rebuild from source for normal development.

No schematic, PCB design, sensor converter firmware, dependency version lock, hardware test report, or license file was found in the supplied project.

## Hardware requirements

| Component | Quantity | Purpose / selection notes |
| --- | --- | --- |
| Arduino Nano compatible AVR board | 1 | The supplied build directory targets `arduino.avr.nano`; confirm your board's processor and bootloader |
| HLK-LD2410D sensor | 1 | Intended source of human-presence and distance reports |
| WS2812B LED ring or strip | 16 pixels | Visual output; a ring is optional because the code addresses a linear pixel sequence |
| USB data cable | 1 | Programming and serial diagnostics |
| Regulated power supply | As needed | Match the voltage and current requirements of the actual board, sensor, and LEDs |
| Jumper wires / breadboard or soldered wiring | As needed | Signal and power connections |
| Logic-level conversion | If required | Match the sensor UART voltage limits to the Arduino signals |
| Text conversion device or revised radar parser | Required for stock radar integration | The supplied code cannot directly decode native sensor reports |

Confirm the exact sensor module's supply voltage, pin labels, and UART logic levels using its own documentation. A supply pin's voltage rating does not establish that its UART pins tolerate the same voltage.

Size the LED supply for the actual pixel hardware and worst-case illumination. Startup includes white flashes, which can draw more current than a single-color display. Do not assume the Nano's regulator or USB supply can power the complete assembly. Use common grounds and avoid backfeeding competing power sources.

## Wiring

Disconnect power before changing connections. Use the labels on the actual module; this project does not provide a verified physical connector pin order.

| Arduino / supply connection | Destination | Explanation |
| --- | --- | --- |
| D6 | WS2812B `DIN` | LED data input, not the strip's `DOUT` end |
| D4 (`SENSOR_RX_PIN`) | Sensor or text-converter `TX` | Arduino receives reports here; voltage levels must be compatible |
| D3 (`SENSOR_TX_PIN`) | Sensor or text-converter `RX`, if needed | Configured as software serial TX, but the sketch sends no sensor commands; level conversion may be needed |
| GND | Sensor, LEDs, converter, and supply GND | All connected signal devices need a shared ground reference |
| Suitable regulated supply | LED power and sensor supply inputs | Use the ratings specified for each device |
| USB | Nano USB connector | Upload and diagnostic connection |

Conceptual signal path with the existing text parser:

```text
HLK-LD2410D native UART reports
            |
            v
External binary-to-text converter (not supplied)
            |  text, e.g. D:120 followed by newline
            v
Nano D4 RX --> parsing --> distance state --> Nano D6 --> 16 WS2812B LEDs
                                      |
                                      +--> USB serial diagnostics
```

Alternatively, implement the sensor's binary protocol in the Nano firmware and connect the sensor TX to D4 with appropriate electrical compatibility. The wiring alone does not resolve the current protocol mismatch.

The sensor's digital presence `OUT` signal is not used. That signal alone would not provide the distance information needed for both distance thresholds.

## Software setup and upload

1. Install the Arduino IDE and the **Arduino AVR Boards** platform if it is not already available.
2. Install **FastLED** through the IDE's Library Manager. The project does not specify a tested version; record the version you use when validating a build.
3. `SoftwareSerial` is supplied with the classic Arduino AVR platform; no separate radar library is included or used.
4. Open:

   ```text
   HUMAN DETECTION HLK-LD2410D PROJECT/HUMAN_PRESENSE_DETECTOR/HUMAN_PRESENSE_DETECTOR.ino
   ```

5. Select **Arduino Nano**, the processor/bootloader option matching your actual board, and its serial port. Some older Nano-compatible boards require the old bootloader option.
6. Click **Verify** to compile, then **Upload**.
7. Open Serial Monitor at **115200 baud** to read diagnostics.
8. Provide compatible text input on **D4**, or complete native binary protocol support before testing the radar directly.

Serial Monitor input from the computer goes to the hardware USB serial interface. The sketch never reads `Serial`, so typing `D:120` into Serial Monitor does **not** inject a sensor reading.

## How it works

### Startup

`setup()` initializes both serial interfaces at 115200 baud, registers 16 WS2812B LEDs on D6 with GRB ordering, and sets brightness to 50 on FastLED's 0–255 scale.

`startupAnimation()` then performs:

1. Three white flashes: 200 ms on and 100 ms off each.
2. Green for 500 ms.
3. Yellow for 500 ms.
4. Red for 500 ms.
5. LEDs off before normal operation.

The sequence takes approximately 2.4 seconds, excluding LED transfer and execution overhead. It blocks the main loop, so incoming sensor data is not processed during this sequence.

### Main loop and functions

Each pass calls `readSensorData()`, `updateState()`, and `updateLEDAnimation()`, followed by `delay(10)`.

| Function | Responsibility |
| --- | --- |
| `readSensorData()` | Accumulate software serial characters and process lines terminated by CR or LF |
| `parseData()` | Extract a distance or presence flag and update the global detection values |
| `updateState()` | Choose green, yellow, or red using the parsed values |
| `updateLEDAnimation()` | Dispatch the animation for the selected state |
| `showGreenBlink()` | Toggle all pixels between green and off approximately every 500 ms |
| `showLoadingAnimation()` | Advance a colored fill animation approximately every 80 ms |
| `startupAnimation()` | Run the power-on color sequence |

Red and yellow progressively fill pixels from index 0 with a fading tail, briefly remain filled, and repeat. The animation uses `NUM_LEDS + 4` steps; with 16 pixels its nominal cycle is about 1.6 seconds. It is not a smooth full-strip emptying animation. A state change resets `animationStep`, but does not reset the animation timers or green blink flag.

## LED states and distance boundaries

The actual comparisons in the source are strict `<` comparisons:

| Parsed condition | LED state | Meaning |
| --- | --- | --- |
| Person detected, `0 < distance < 150` | Red loading animation | Near zone |
| Person detected, `150 <= distance < 250` | Yellow loading animation | Intermediate zone |
| Person detected, `distance >= 250` | Green blinking | Beyond the configured warning zones |
| Person detected without a positive distance | Red loading animation | Presence flag with unknown distance |
| No person detected | Green blinking | No detection in the most recently processed input |

**Exactly 150 cm is yellow. Exactly 250 cm is green.** Green can therefore mean either no detection or a detected person beyond the warning thresholds. The startup serial legend simplifies this behavior by describing green only as “No person.”

The normal distance parser accepts values below 600 cm. This software filter is not a specification of the sensor's physical operating range.

## Serial input format

### Expected text input

The current firmware expects text at **115200 baud** on the software serial RX pin, terminated by `\n` or `\r`. A simple compatible message is:

```text
D:120
```

Send actual line-ending bytes after the text. Distance is interpreted in centimeters.

The parser processes each nonempty line in this order:

1. Look for `D:` followed immediately by digits. Values greater than 0 and less than 600 establish detection and distance.
2. If detection is still false, look for `M:` followed by text starting with `1`.
3. If still false, look for `P:` followed by text starting with `1`.
4. If still false, extract the line's first sequence of digits. A value greater than 10 and less than 600 establishes detection and distance.
5. Replace the global detection and distance values with the result.

The fields are case-sensitive. Avoid spaces after the colon, signed values, decimals, and unrelated numbers. For reliable demonstrations, send one plain `D:<integer>` field per line, or `P:0` to clear detection.

| Example line | Result in the supplied code |
| --- | --- |
| `D:100` | Detected at 100 cm; red |
| `D:149` | Red |
| `D:150` | Yellow |
| `D:249` | Yellow |
| `D:250` | Detected, but green |
| `D:599` | Detected, but green |
| `D:600` | No detection; green |
| `D:0` | No detection; green |
| `M:1` or `P:1` | Detected with distance 0; red |
| `P:0` | No detection; green |
| `120` | Fallback distance 120 cm; red |
| `status 42` | Also interpreted as distance 42 cm; red |

Presence flags are checked by prefix, not strict field validation: for example, `P:10` also starts with `1`. A valid `D:` detection takes priority over `P:0` on the same line. Avoid ambiguous multi-field messages.

### Native HLK-LD2410D compatibility

Hi-Link's **HLK-LD2410D Serial Communication Protocol V1.01** specifies a default UART rate of **115200 baud**, with one stop bit and no parity, and binary report frames beginning `F4 F3 F2 F1` and ending `F8 F7 F6 F5`. These frames are not newline-delimited `D:`, `M:`, or `P:` text. See the [manufacturer-authored protocol PDF, hosted by a distributor](https://www.rajguruelectronics.com/Product/33426/190126150232.01.pdf).

The baud rate matches this sketch, but the report format does not. Raw binary data may produce no useful result, accidental detections, or uncontrolled buffer growth in the existing text parser. Do not interpret an LED response to raw binary traffic as proof that the radar is decoded correctly.

For direct sensor integration, a future parser needs to validate frame headers, lengths, and endings; decode target state and distance fields; distinguish command acknowledgements from reports; and handle incomplete or invalid frames. Confirm documentation for the exact **LD2410D** variant rather than assuming other LD2410-family modules have identical settings.

## Configuration

Edit these definitions near the beginning of the sketch, then compile and upload again:

```cpp
#define LED_PIN         6
#define NUM_LEDS        16
#define LED_BRIGHTNESS  50

#define SENSOR_RX_PIN   4
#define SENSOR_TX_PIN   3

#define NEAR_THRESHOLD  150
#define FAR_THRESHOLD   250
```

| Setting | Effect |
| --- | --- |
| `LED_PIN` | Arduino pin connected to LED `DIN` |
| `NUM_LEDS` | Number of addressable pixels; also affects animation duration |
| `LED_BRIGHTNESS` | Overall FastLED brightness, normally 0–255 |
| `SENSOR_RX_PIN` / `SENSOR_TX_PIN` | Software serial pin assignments; arguments are RX first, TX second |
| `NEAR_THRESHOLD` | Upper exclusive boundary of the near red zone, in cm |
| `FAR_THRESHOLD` | Upper exclusive boundary of the yellow zone, in cm |

Keep `0 < NEAR_THRESHOLD < FAR_THRESHOLD`. Thresholds describe software zones; they do not configure the radar's sensing range or sensitivity.

Additional hardcoded settings are `Serial.begin(115200)`, `sensorSerial.begin(115200)`, the `WS2812B`/`GRB` template arguments, the 500 ms blink interval, the 80 ms animation interval, and the parser's `< 600` distance ceiling. Changing only the zone thresholds does not change that ceiling.

## Testing and commissioning

### 1. Verify the LED output

Power the assembly and check all startup colors and all 16 pixels. After startup, green should blink until a parsed message changes the state. A successful startup display checks the LED path; it does not prove radar communication.

### 2. Verify the parser independently

Disconnect the sensor TX before connecting another transmitter to D4. Use a UART text source with compatible logic levels, a shared ground, and 115200 baud. Send one newline-terminated message at a time.

Test `D:100`, `D:150`, `D:249`, `D:250`, `P:1`, and `P:0` against the table above. Watch the USB diagnostics at 115200 baud. The sketch prints parsed distances, the selected color, and a state-change message when the color changes.

### 3. Check stale-data behavior

Send `D:100`, then stop transmitting. The current implementation continues showing the last red state indefinitely. Send `P:0` to clear it. This confirms the present lack of a communication timeout.

### 4. Integrate and assess the actual radar

After adding a verified binary parser or external converter, compare reported distances with known positions around 1.5 m and 2.5 m. Test entry, exit, stationary presence, movement, obstacles, and the actual installation environment. Record the board, sensor firmware, library version, power supply, and observed behavior.

No physical measurements or hardware tests are included in this repository. Arduino CLI was not available in the documentation environment, so the sketch was not freshly compiled for this README.

## Troubleshooting

| Symptom | Likely checks / explanation |
| --- | --- |
| LEDs never light | Check LED supply, common ground, D6 wiring, `DIN` direction, LED count, and uploaded firmware |
| Incorrect colors | Verify that the LED hardware uses the configured `GRB` ordering |
| Startup works but radar has no useful effect | Check the native binary/text mismatch first, then UART wiring and baud rate |
| Typing in Serial Monitor changes nothing | Expected: input is read from D4 software serial, not USB `Serial` |
| Random detections | Unrelated numbers trigger the fallback parser; raw binary data is unsupported; also check signal integrity and power |
| Missed or corrupted reports | Check software serial timing, frequent LED refreshes, diagnostic output volume, and transmitter compatibility |
| Indicator remains red after disconnecting sensor | Expected: no stale-data timeout exists |
| Green while someone is present | Distances of 250 cm or more map to green; invalid or unsupported input can also result in green |
| Red with a reported distance of 0 cm | `M:1` or `P:1` establishes presence without a distance |
| Frequent red/yellow or yellow/green switching | There is no hysteresis or smoothing around thresholds |
| `FastLED.h: No such file or directory` | Install FastLED in the IDE used for compilation |
| Upload fails | Check board, processor/bootloader selection, USB cable, port, and whether another application holds the port |
| Memory instability over time | Check the unbounded text buffer and dynamic `String` allocation, especially with unsupported input |
| Diagnostic symbols appear garbled | The sketch contains Unicode symbols; check terminal encoding or replace them with plain text |

## Known limitations

- **Native radar decoding is missing.** No included dependency or converter supplies it.
- **No timeout or sensor-fault state.** Before any message the default is green; after communication stops the last parsed result persists.
- **Permissive parsing.** Unrelated digits can become a distance, and prefix-based flags can accept unintended values.
- **Unbounded line buffer.** Input without line endings keeps appending to an Arduino `String`; this can exhaust limited AVR RAM.
- **Serial timing sensitivity.** Software serial at 115200 baud, frequent LED updates, and heavy logging need validation together on the target board. WS2812 timing and serial reception can compete for interrupt timing on AVR.
- **Repeated LED refreshes and logs.** `FastLED.show()` runs on every loop even between animation steps, and state diagnostics print every loop rather than only when data or state changes.
- **No hysteresis or filtering.** Readings near thresholds can cause rapid state changes.
- **Blocking startup.** The firmware does not process reports during the startup animation.
- **Limited meaning of green.** It combines no detection with detection beyond the selected warning zones.
- **No separate moving/stationary display.** Input flags are combined into a single presence result.
- **No persisted settings or runtime controls.** Configuration changes require editing and uploading the sketch.
- **No verified accuracy, latency, or coverage figures.** These require actual hardware measurement.

## Possible improvements

These are future work, not features of the supplied firmware:

1. Implement and test the LD2410D binary protocol or provide a documented converter.
2. Prefer a hardware UART for radar reception where the chosen board permits it.
3. Add a bounded receive buffer, strict validation, and recovery after malformed frames.
4. Add a last-valid-report timestamp and a distinct sensor-fault indication.
5. Add threshold hysteresis and appropriate distance filtering.
6. Refresh LEDs only when needed and limit diagnostics to new readings or state changes.
7. Make startup nonblocking so reports can be processed immediately.
8. Distinguish no occupancy, distant occupancy, and sensor failure visually.
9. Document the verified wiring, power budget, dependency versions, and measured test results.

## References and licensing

- [HLK-LD2410D Serial Communication Protocol V1.01](https://www.rajguruelectronics.com/Product/33426/190126150232.01.pdf) — manufacturer-authored document hosted by a distributor; reference for the D variant's UART and binary protocol.
- [FastLED project and documentation](https://github.com/FastLED/FastLED) — the LED library used by the sketch.
- [Arduino SoftwareSerial documentation](https://docs.arduino.cc/learn/built-in-libraries/software-serial/) — software serial library background.

No project license or author attribution was supplied. Confirm ownership and add an appropriate license before redistribution. Third-party libraries retain their own licenses.
# Human Presence and Proximity Indicator — HLK-LD2410D

An Arduino project intended to turn human-presence and distance information into an animated, easy-to-read visual indicator using 16 WS2812B addressable RGB LEDs.

The intended display is **red for someone nearby**, **yellow for someone at an intermediate distance**, and **blinking green when no one is detected or the detected distance is beyond the warning thresholds**.

> **Current implementation status:** The supplied sketch implements LED animations and a permissive, line-based text parser. It does **not** decode the HLK-LD2410D's native binary radar protocol. Direct connection to a stock sensor therefore requires a compatible binary parser or an intermediate device that converts radar reports into the text format described below. No converter is included. Existing Nano build files are supplied, but hardware operation and a fresh compilation have not been verified as part of this documentation.

## Contents

- [Intended purpose](#intended-purpose)
- [Features and scope](#features-and-scope)
- [Project files](#project-files)
- [Hardware requirements](#hardware-requirements)
- [Wiring](#wiring)
- [Software setup and upload](#software-setup-and-upload)
- [How it works](#how-it-works)
- [LED states and distance boundaries](#led-states-and-distance-boundaries)
- [Serial input format](#serial-input-format)
- [Configuration](#configuration)
- [Testing and commissioning](#testing-and-commissioning)
- [Troubleshooting](#troubleshooting)
- [Known limitations](#known-limitations)
- [Possible improvements](#possible-improvements)
- [References and licensing](#references-and-licensing)

## Intended purpose

The project is intended as a local human-presence and proximity demonstration: a radar supplies presence/distance information, an Arduino classifies it, and an LED ring or strip makes the result visible without requiring a computer screen.

Possible uses include:

- A desk or doorway proximity indicator.
- A visual demonstration of distance-based embedded control.
- A classroom project combining serial communication, state management, and addressable LEDs.
- A prototype front end for a future occupancy or automation system.

The current firmware provides a visual indication only. It does not identify people, count occupants, record events, switch appliances, send notifications, or communicate over a network. It is a prototype and should not be relied on as a protective alarm or safety interlock.

## Features and scope

| Item | Current implementation |
| --- | --- |
| Intended sensor | Hi-Link HLK-LD2410D human-presence radar |
| Build target indicated by included files | Arduino AVR Nano |
| LED type | WS2812B, GRB color order |
| LED count | 16 |
| LED data pin | D6 |
| Sensor serial receive/transmit pins | D4 / D3 |
| USB diagnostic serial speed | 115200 baud |
| Sensor-side software serial speed | 115200 baud |
| Distance units used by the sketch | Centimeters |
| Near threshold | 150 cm |
| Far threshold | 250 cm |
| Startup display | White flashes, then green, yellow, and red |
| Normal display | Green blink or red/yellow loading animation |
| Sensor protocol support | Text input only; native radar binary parsing is absent |

## Project files

Paths below are relative to the folder containing this README. The repeated project folder name and the spelling `PRESENSE` are preserved from the supplied files.

```text
.
├── README.md
└── HUMAN DETECTION HLK-LD2410D PROJECT/
    └── HUMAN_PRESENSE_DETECTOR/
        ├── HUMAN_PRESENSE_DETECTOR.ino
        └── build/
            └── arduino.avr.nano/
                ├── HUMAN_PRESENSE_DETECTOR.ino.hex
                ├── HUMAN_PRESENSE_DETECTOR.ino.with_bootloader.hex
                ├── HUMAN_PRESENSE_DETECTOR.ino.with_bootloader.bin
                ├── HUMAN_PRESENSE_DETECTOR.ino.elf
                └── HUMAN_PRESENSE_DETECTOR.ino.eep
```

The `.ino` file is the editable source. The `build` folder contains previously generated firmware artifacts: application HEX, bootloader-inclusive images, an ELF executable/debug artifact, and an EEPROM image. Their presence does not establish that they match the current source or the bootloader on your board. Rebuild from source for normal development.

No schematic, PCB design, sensor converter firmware, dependency version lock, hardware test report, or license file was found in the supplied project.

## Hardware requirements

| Component | Quantity | Purpose / selection notes |
| --- | --- | --- |
| Arduino Nano compatible AVR board | 1 | The supplied build directory targets `arduino.avr.nano`; confirm your board's processor and bootloader |
| HLK-LD2410D sensor | 1 | Intended source of human-presence and distance reports |
| WS2812B LED ring or strip | 16 pixels | Visual output; a ring is optional because the code addresses a linear pixel sequence |
| USB data cable | 1 | Programming and serial diagnostics |
| Regulated power supply | As needed | Match the voltage and current requirements of the actual board, sensor, and LEDs |
| Jumper wires / breadboard or soldered wiring | As needed | Signal and power connections |
| Logic-level conversion | If required | Match the sensor UART voltage limits to the Arduino signals |
| Text conversion device or revised radar parser | Required for stock radar integration | The supplied code cannot directly decode native sensor reports |

Confirm the exact sensor module's supply voltage, pin labels, and UART logic levels using its own documentation. A supply pin's voltage rating does not establish that its UART pins tolerate the same voltage.

Size the LED supply for the actual pixel hardware and worst-case illumination. Startup includes white flashes, which can draw more current than a single-color display. Do not assume the Nano's regulator or USB supply can power the complete assembly. Use common grounds and avoid backfeeding competing power sources.

## Wiring

Disconnect power before changing connections. Use the labels on the actual module; this project does not provide a verified physical connector pin order.

| Arduino / supply connection | Destination | Explanation |
| --- | --- | --- |
| D6 | WS2812B `DIN` | LED data input, not the strip's `DOUT` end |
| D4 (`SENSOR_RX_PIN`) | Sensor or text-converter `TX` | Arduino receives reports here; voltage levels must be compatible |
| D3 (`SENSOR_TX_PIN`) | Sensor or text-converter `RX`, if needed | Configured as software serial TX, but the sketch sends no sensor commands; level conversion may be needed |
| GND | Sensor, LEDs, converter, and supply GND | All connected signal devices need a shared ground reference |
| Suitable regulated supply | LED power and sensor supply inputs | Use the ratings specified for each device |
| USB | Nano USB connector | Upload and diagnostic connection |

Conceptual signal path with the existing text parser:

```text
HLK-LD2410D native UART reports
            |
            v
External binary-to-text converter (not supplied)
            |  text, e.g. D:120 followed by newline
            v
Nano D4 RX --> parsing --> distance state --> Nano D6 --> 16 WS2812B LEDs
                                      |
                                      +--> USB serial diagnostics
```

Alternatively, implement the sensor's binary protocol in the Nano firmware and connect the sensor TX to D4 with appropriate electrical compatibility. The wiring alone does not resolve the current protocol mismatch.

The sensor's digital presence `OUT` signal is not used. That signal alone would not provide the distance information needed for both distance thresholds.

## Software setup and upload

1. Install the Arduino IDE and the **Arduino AVR Boards** platform if it is not already available.
2. Install **FastLED** through the IDE's Library Manager. The project does not specify a tested version; record the version you use when validating a build.
3. `SoftwareSerial` is supplied with the classic Arduino AVR platform; no separate radar library is included or used.
4. Open:

   ```text
   HUMAN DETECTION HLK-LD2410D PROJECT/HUMAN_PRESENSE_DETECTOR/HUMAN_PRESENSE_DETECTOR.ino
   ```

5. Select **Arduino Nano**, the processor/bootloader option matching your actual board, and its serial port. Some older Nano-compatible boards require the old bootloader option.
6. Click **Verify** to compile, then **Upload**.
7. Open Serial Monitor at **115200 baud** to read diagnostics.
8. Provide compatible text input on **D4**, or complete native binary protocol support before testing the radar directly.

Serial Monitor input from the computer goes to the hardware USB serial interface. The sketch never reads `Serial`, so typing `D:120` into Serial Monitor does **not** inject a sensor reading.

## How it works

### Startup

`setup()` initializes both serial interfaces at 115200 baud, registers 16 WS2812B LEDs on D6 with GRB ordering, and sets brightness to 50 on FastLED's 0–255 scale.

`startupAnimation()` then performs:

1. Three white flashes: 200 ms on and 100 ms off each.
2. Green for 500 ms.
3. Yellow for 500 ms.
4. Red for 500 ms.
5. LEDs off before normal operation.

The sequence takes approximately 2.4 seconds, excluding LED transfer and execution overhead. It blocks the main loop, so incoming sensor data is not processed during this sequence.

### Main loop and functions

Each pass calls `readSensorData()`, `updateState()`, and `updateLEDAnimation()`, followed by `delay(10)`.

| Function | Responsibility |
| --- | --- |
| `readSensorData()` | Accumulate software serial characters and process lines terminated by CR or LF |
| `parseData()` | Extract a distance or presence flag and update the global detection values |
| `updateState()` | Choose green, yellow, or red using the parsed values |
| `updateLEDAnimation()` | Dispatch the animation for the selected state |
| `showGreenBlink()` | Toggle all pixels between green and off approximately every 500 ms |
| `showLoadingAnimation()` | Advance a colored fill animation approximately every 80 ms |
| `startupAnimation()` | Run the power-on color sequence |

Red and yellow progressively fill pixels from index 0 with a fading tail, briefly remain filled, and repeat. The animation uses `NUM_LEDS + 4` steps; with 16 pixels its nominal cycle is about 1.6 seconds. It is not a smooth full-strip emptying animation. A state change resets `animationStep`, but does not reset the animation timers or green blink flag.

## LED states and distance boundaries

The actual comparisons in the source are strict `<` comparisons:

| Parsed condition | LED state | Meaning |
| --- | --- | --- |
| Person detected, `0 < distance < 150` | Red loading animation | Near zone |
| Person detected, `150 <= distance < 250` | Yellow loading animation | Intermediate zone |
| Person detected, `distance >= 250` | Green blinking | Beyond the configured warning zones |
| Person detected without a positive distance | Red loading animation | Presence flag with unknown distance |
| No person detected | Green blinking | No detection in the most recently processed input |

**Exactly 150 cm is yellow. Exactly 250 cm is green.** Green can therefore mean either no detection or a detected person beyond the warning thresholds. The startup serial legend simplifies this behavior by describing green only as “No person.”

The normal distance parser accepts values below 600 cm. This software filter is not a specification of the sensor's physical operating range.

## Serial input format

### Expected text input

The current firmware expects text at **115200 baud** on the software serial RX pin, terminated by `\n` or `\r`. A simple compatible message is:

```text
D:120
```

Send actual line-ending bytes after the text. Distance is interpreted in centimeters.

The parser processes each nonempty line in this order:

1. Look for `D:` followed immediately by digits. Values greater than 0 and less than 600 establish detection and distance.
2. If detection is still false, look for `M:` followed by text starting with `1`.
3. If still false, look for `P:` followed by text starting with `1`.
4. If still false, extract the line's first sequence of digits. A value greater than 10 and less than 600 establishes detection and distance.
5. Replace the global detection and distance values with the result.

The fields are case-sensitive. Avoid spaces after the colon, signed values, decimals, and unrelated numbers. For reliable demonstrations, send one plain `D:<integer>` field per line, or `P:0` to clear detection.

| Example line | Result in the supplied code |
| --- | --- |
| `D:100` | Detected at 100 cm; red |
| `D:149` | Red |
| `D:150` | Yellow |
| `D:249` | Yellow |
| `D:250` | Detected, but green |
| `D:599` | Detected, but green |
| `D:600` | No detection; green |
| `D:0` | No detection; green |
| `M:1` or `P:1` | Detected with distance 0; red |
| `P:0` | No detection; green |
| `120` | Fallback distance 120 cm; red |
| `status 42` | Also interpreted as distance 42 cm; red |

Presence flags are checked by prefix, not strict field validation: for example, `P:10` also starts with `1`. A valid `D:` detection takes priority over `P:0` on the same line. Avoid ambiguous multi-field messages.

### Native HLK-LD2410D compatibility

Hi-Link's **HLK-LD2410D Serial Communication Protocol V1.01** specifies a default UART rate of **115200 baud**, with one stop bit and no parity, and binary report frames beginning `F4 F3 F2 F1` and ending `F8 F7 F6 F5`. These frames are not newline-delimited `D:`, `M:`, or `P:` text. See the [manufacturer-authored protocol PDF, hosted by a distributor](https://www.rajguruelectronics.com/Product/33426/190126150232.01.pdf).

The baud rate matches this sketch, but the report format does not. Raw binary data may produce no useful result, accidental detections, or uncontrolled buffer growth in the existing text parser. Do not interpret an LED response to raw binary traffic as proof that the radar is decoded correctly.

For direct sensor integration, a future parser needs to validate frame headers, lengths, and endings; decode target state and distance fields; distinguish command acknowledgements from reports; and handle incomplete or invalid frames. Confirm documentation for the exact **LD2410D** variant rather than assuming other LD2410-family modules have identical settings.

## Configuration

Edit these definitions near the beginning of the sketch, then compile and upload again:

```cpp
#define LED_PIN         6
#define NUM_LEDS        16
#define LED_BRIGHTNESS  50

#define SENSOR_RX_PIN   4
#define SENSOR_TX_PIN   3

#define NEAR_THRESHOLD  150
#define FAR_THRESHOLD   250
```

| Setting | Effect |
| --- | --- |
| `LED_PIN` | Arduino pin connected to LED `DIN` |
| `NUM_LEDS` | Number of addressable pixels; also affects animation duration |
| `LED_BRIGHTNESS` | Overall FastLED brightness, normally 0–255 |
| `SENSOR_RX_PIN` / `SENSOR_TX_PIN` | Software serial pin assignments; arguments are RX first, TX second |
| `NEAR_THRESHOLD` | Upper exclusive boundary of the near red zone, in cm |
| `FAR_THRESHOLD` | Upper exclusive boundary of the yellow zone, in cm |

Keep `0 < NEAR_THRESHOLD < FAR_THRESHOLD`. Thresholds describe software zones; they do not configure the radar's sensing range or sensitivity.

Additional hardcoded settings are `Serial.begin(115200)`, `sensorSerial.begin(115200)`, the `WS2812B`/`GRB` template arguments, the 500 ms blink interval, the 80 ms animation interval, and the parser's `< 600` distance ceiling. Changing only the zone thresholds does not change that ceiling.

## Testing and commissioning

### 1. Verify the LED output

Power the assembly and check all startup colors and all 16 pixels. After startup, green should blink until a parsed message changes the state. A successful startup display checks the LED path; it does not prove radar communication.

### 2. Verify the parser independently

Disconnect the sensor TX before connecting another transmitter to D4. Use a UART text source with compatible logic levels, a shared ground, and 115200 baud. Send one newline-terminated message at a time.

Test `D:100`, `D:150`, `D:249`, `D:250`, `P:1`, and `P:0` against the table above. Watch the USB diagnostics at 115200 baud. The sketch prints parsed distances, the selected color, and a state-change message when the color changes.

### 3. Check stale-data behavior

Send `D:100`, then stop transmitting. The current implementation continues showing the last red state indefinitely. Send `P:0` to clear it. This confirms the present lack of a communication timeout.

### 4. Integrate and assess the actual radar

After adding a verified binary parser or external converter, compare reported distances with known positions around 1.5 m and 2.5 m. Test entry, exit, stationary presence, movement, obstacles, and the actual installation environment. Record the board, sensor firmware, library version, power supply, and observed behavior.

No physical measurements or hardware tests are included in this repository. Arduino CLI was not available in the documentation environment, so the sketch was not freshly compiled for this README.

## Troubleshooting

| Symptom | Likely checks / explanation |
| --- | --- |
| LEDs never light | Check LED supply, common ground, D6 wiring, `DIN` direction, LED count, and uploaded firmware |
| Incorrect colors | Verify that the LED hardware uses the configured `GRB` ordering |
| Startup works but radar has no useful effect | Check the native binary/text mismatch first, then UART wiring and baud rate |
| Typing in Serial Monitor changes nothing | Expected: input is read from D4 software serial, not USB `Serial` |
| Random detections | Unrelated numbers trigger the fallback parser; raw binary data is unsupported; also check signal integrity and power |
| Missed or corrupted reports | Check software serial timing, frequent LED refreshes, diagnostic output volume, and transmitter compatibility |
| Indicator remains red after disconnecting sensor | Expected: no stale-data timeout exists |
| Green while someone is present | Distances of 250 cm or more map to green; invalid or unsupported input can also result in green |
| Red with a reported distance of 0 cm | `M:1` or `P:1` establishes presence without a distance |
| Frequent red/yellow or yellow/green switching | There is no hysteresis or smoothing around thresholds |
| `FastLED.h: No such file or directory` | Install FastLED in the IDE used for compilation |
| Upload fails | Check board, processor/bootloader selection, USB cable, port, and whether another application holds the port |
| Memory instability over time | Check the unbounded text buffer and dynamic `String` allocation, especially with unsupported input |
| Diagnostic symbols appear garbled | The sketch contains Unicode symbols; check terminal encoding or replace them with plain text |

## Known limitations

- **Native radar decoding is missing.** No included dependency or converter supplies it.
- **No timeout or sensor-fault state.** Before any message the default is green; after communication stops the last parsed result persists.
- **Permissive parsing.** Unrelated digits can become a distance, and prefix-based flags can accept unintended values.
- **Unbounded line buffer.** Input without line endings keeps appending to an Arduino `String`; this can exhaust limited AVR RAM.
- **Serial timing sensitivity.** Software serial at 115200 baud, frequent LED updates, and heavy logging need validation together on the target board. WS2812 timing and serial reception can compete for interrupt timing on AVR.
- **Repeated LED refreshes and logs.** `FastLED.show()` runs on every loop even between animation steps, and state diagnostics print every loop rather than only when data or state changes.
- **No hysteresis or filtering.** Readings near thresholds can cause rapid state changes.
- **Blocking startup.** The firmware does not process reports during the startup animation.
- **Limited meaning of green.** It combines no detection with detection beyond the selected warning zones.
- **No separate moving/stationary display.** Input flags are combined into a single presence result.
- **No persisted settings or runtime controls.** Configuration changes require editing and uploading the sketch.
- **No verified accuracy, latency, or coverage figures.** These require actual hardware measurement.

## Possible improvements

These are future work, not features of the supplied firmware:

1. Implement and test the LD2410D binary protocol or provide a documented converter.
2. Prefer a hardware UART for radar reception where the chosen board permits it.
3. Add a bounded receive buffer, strict validation, and recovery after malformed frames.
4. Add a last-valid-report timestamp and a distinct sensor-fault indication.
5. Add threshold hysteresis and appropriate distance filtering.
6. Refresh LEDs only when needed and limit diagnostics to new readings or state changes.
7. Make startup nonblocking so reports can be processed immediately.
8. Distinguish no occupancy, distant occupancy, and sensor failure visually.
9. Document the verified wiring, power budget, dependency versions, and measured test results.

## References and licensing

- [HLK-LD2410D Serial Communication Protocol V1.01](https://www.rajguruelectronics.com/Product/33426/190126150232.01.pdf) — manufacturer-authored document hosted by a distributor; reference for the D variant's UART and binary protocol.
- [FastLED project and documentation](https://github.com/FastLED/FastLED) — the LED library used by the sketch.
- [Arduino SoftwareSerial documentation](https://docs.arduino.cc/learn/built-in-libraries/software-serial/) — software serial library background.

No project license or author attribution was supplied. Confirm ownership and add an appropriate license before redistribution. Third-party libraries retain their own licenses.
