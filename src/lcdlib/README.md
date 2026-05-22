# LCDLib
This library provides an interface to control an HD44780U LCD driver, using a PCF8574 I/O expander, from a Raspberry Pico over I2C.

## Hardware Description

The HD44780U LCD controller accepts commands and data through an 8-bit parallel bus (DB0–DB7)
along with three control signals:

| Signal | Function                                                          |
| ------ | ----------------------------------------------------------------- |
| RS     | Register select: LOW = instruction register, HIGH = data register |
| R/W    | Direction: LOW = write, HIGH = read                               |
| E      | Enable: data is latched on the falling edge                       |

Because the Raspberry Pi Pico communicates over I2C rather than a parallel bus, a **PCF8574
remote 8-bit I/O expander** is used as an intermediary ("backpack"). The PCF8574 presents a
single-register, 8-bit parallel port (P0–P7) to the LCD while accepting commands from the MCU
over the two-wire I2C bus (SDA + SCL). Its 7-bit I2C slave address is determined by three
hardware address pins (A0–A2), giving up to eight uniquely addressable devices on the same bus.

### Pin mapping (PCF8574 → HD44780U)

A typical backpack wiring maps the expander's output port to the LCD as follows:

| PCF8574 pin | LCD signal | Notes                                    |
| ----------- | ---------- | ---------------------------------------- |
| P0          | RS         | Register select                          |
| P1          | R/W        | Read/Write (held LOW for write-only use) |
| P2          | E          | Enable                                   |
| P3          | BL         | Backlight control                        |
| P4          | DB4        | Data bit 0 (4-bit mode)                  |
| P5          | DB5        | Data bit 1 (4-bit mode)                  |
| P6          | DB6        | Data bit 2 (4-bit mode)                  |
| P7          | DB7        | Data bit 3 (4-bit mode)                  |

Because only four data lines are wired, the HD44780U is operated in **4-bit mode**: each byte is
split into two nibbles and sent high-nibble-first, each nibble accompanied by an E pulse.

### Sending a command or data byte

1. The MCU composes a single byte that encodes the nibble on DB4–DB7, the RS flag, the
   E flag and the backlight flag into the 8-bit PCF8574 port value.
2. The MCU writes this byte to the PCF8574 over I2C
   (`<START> <slave_addr + W> <ACK> <data_byte> <ACK> <STOP>`).
3. The PCF8574 drives its P0–P7 outputs to match the byte, which in turn drives the LCD pins.
4. The MCU pulses E (sends the byte twice — once with E high, once with E low) to latch the
   nibble into the HD44780U.
5. Steps 1–4 are repeated for the low nibble to complete the full 8-bit transfer.

Because the PCF8574 is purely an I/O expander with no internal buffering or protocol awareness,
every change to the LCD bus (including the E pulse) requires a separate I2C write transaction.
