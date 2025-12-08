How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-intel_mcs51/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to example
$ cd platform-intel_mcs51/examples/stc-header

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e stc15w408as

# Upload firmware for the specific environment
$ pio run -e stc15w408as --target upload

# Clean build files
$ pio run --target clean
```



PIN mapping for STC15W201S
-------------------------
| STC15W201S Pin | Function        |
|----------------|-----------------|
| Pin 1          | P3.0 (RXD)                      |
| Pin 2          | P3.1 (TXD)                      |
| Pin 3          | P5.4 Reset                      |
| Pin 4          | GND                             |
| Pin 5          | P5.5 Relay                      |
| Pin 6          | P3.2 Input analog for Relay on, off |
| Pin 7          | P3.3 Kurzschluss-Erkennung      |
| Pin 8          | VCC                             |
Pin 7: P3.3 Kurzschluss-Erkennung
Pin 8: VCC