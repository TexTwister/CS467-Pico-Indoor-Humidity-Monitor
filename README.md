# Pico Indoor Humidity Monitor

## Getting Started

### Setting Up Hardware for Blink Test
Before beginning the blink test, gather the following components and follow along with the Wiring Diagram.

**Reference Files:**
* Image of Wiring Diagram: [pico_blink_test_wiring.png](docs/pico_blink_test_wiring.png)
* Fritzing File: [cs467_pico_blink_test_wiring.fzz](docs/cs467_pico_blink_test_wiring.fzz)
  
**Bill of Materials (BOM)**
* Raspberry Pi Pico x 1
* Full size breadboard x 1
* 220 Ω resistor x 6
* LEDs
    * Red x 2
    * Yellow x 2
    * Green x 2
* Jumpers
    * Black  (short x 2)
    * Orange (short x 1)
    * Yellow (medium x 1, long x 1)
    * Green (medium x 1, long x 1)
    * Blue (long x 1)

### Flashing the Pico for the Blink Test
Verify functionality:
1. **Download the given pre-compiled file** [LEDarray.uf2](bin/LEDarray.uf2) where UF2 = USB Flashing Format, in this repository and download it to your local device.
2. **Enter into Bootloader Mode:**
    * Connect micro USB cord into the Raspberry Pi Pico's micro USB port.
    * Hold down the BOOTSEL button on the Raspberry Pi Pico microcontroller.
    * While holding down the button, connect the USB cord to your local device.
    * Release the BOOTSEL button once a new drive pops up. Note: At first the computer may "recognize the microcontroller as a thumb drive. Double-click the pop-up menu and open in File Explorer. This should open a new drive named **RPI-RP2**.
3. **Flash the Microcontroller:**
    * Copy and paste the `LEDarray.uf2` file into the **RPI-RP2** drive.
    * The drive will disappear, and the Pico will reboot.
4. **Verify Successful Flashing:**
     * If successful, the LED array on the breadboard should begin their sequential blinking.  

## DHT20 Humidity Sensor Addition

The repo now includes a minimal Raspberry Pi Pico SDK project that talks to an Adafruit DHT20 over I2C before any LCD or LED logic is added.

**Reference Files:**
* DHT20 Fritzing File (sensor was missing from Fritzing): [DHT20_Sensor.fzpz](docs/DHT20_Sensor.fzpz)

### Current code layout

* `CMakeLists.txt`
* `pico_sdk_import.cmake`
* `src/main.c`
* `src/dht20lib/dht20.c`
* `src/dht20lib/include/dht20.h`

### Current hardware assumptions

* `i2c0`
* `GP20` = SDA
* `GP21` = SCL
* DHT20 address = `0x38`
* DHT20 powered from Pico `3V3(OUT)` and `GND`
* `GP4` is used by the LED array, so do not wire DHT20 SDA there unless you
  also move `LED_6_PIN` in `src/ledlib/led_logic.c`.

### Build

If your Pico SDK is installed at a custom path, export it first:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
```

Then configure and build:

```bash
cmake -B build -S . -DPICO_BOARD=pico_w
cmake --build build -j
```

Or use the checked-in CMake preset:

```bash
cmake --preset pico-debug
cmake --build --preset pico-debug -j
```

If CMake reports that `/usr/bin/gcc` cannot compile a test program and shows `-mthumb` as an unrecognized option, the build directory was configured earlier with the host compiler instead of the Pico ARM cross-compiler. Clear the stale cache and configure again:

```bash
rm -rf build
cmake -B build -S . -DPICO_BOARD=pico_w
cmake --build build -j
```

The generated UF2 will be:

```bash
build/pico_indoor_humidity_monitor.uf2
```

### VS Code include troubleshooting

If `main.c` shows `#include` errors for `pico/stdlib.h`, `hardware/i2c.h`, or `dht20.h`, the usual cause is that VS Code has not configured CMake yet.

1. Install the VS Code extensions:
   * `C/C++`
   * `CMake Tools`
2. In your WSL shell, make sure the Pico SDK path is exported before opening VS Code:

```bash
export PICO_SDK_PATH=/home/cadav/pico-sdk
code .
```

3. This workspace is set to use `CMakePresets.json`, so in VS Code select the `pico-debug` or `pico-release` configure preset instead of a host GCC kit.

If you still see `/usr/bin/gcc` in the CMake output, clear the stale configure results with `CMake: Delete Cache and Reconfigure`, then select the `pico-debug` preset.

### Serial output

On successful bring-up, USB serial should print a repeating message similar to:

```text
Starting DHT20 Humidity Sensor (SDA=GP20, SCL=GP21)
DHT20 found at 0x38
Temp: 23.41 C  Humidity: 46.82 %RH
```

If the sensor is not wired correctly, the firmware keeps retrying and prints a wiring hint instead of silently failing.
  
