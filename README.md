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
  
