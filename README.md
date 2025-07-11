# debugSerial Library for AVR

A lightweight, transmit-only UART debugging library designed for the **ATmega328PB** microcontroller using **UART1**. It supports printing strings, integers, and floating-point numbers. This library is intended for bare-metal development in **Microchip Studio** and is compatible with **AVR-GCC**.

---

## Features

- Buffered UART1 transmission using a ring buffer (size: 100 bytes).
- Functions: `debugPrint`, `debugPrintln`, `debugPrintInt`, `debugPrintIntln`, `debugPrintFloat`, `debugPrintFloatln`.
- Configurable baud rate.
- Transmit-only: Does not support receiving data.
- Designed for ATmega328PB (which has two UARTs: UART0 and UART1).
- Adaptable for ATmega328P (which has only UART0) by modifying register names.

---

## How UART Transmission Works

The library uses the ATmega328PB’s UART1 module to transmit data serially. **UART** (Universal Asynchronous Receiver/Transmitter) sends data as a series of bits over a single wire (**TX pin, PD3** on ATmega328PB). The library:

- Configures UART1 with a specified baud rate, 8-bit data, no parity, and 1 stop bit.
- Uses a ring buffer to store outgoing data, allowing non-blocking writes.
- Employs interrupts (`USART1_UDRE_vect`) to transmit data when the UART data register (`UDR1`) is empty.
- Does not support receiving data, as it only enables the transmitter (`TXEN1`).

**Connect the TX pin to a serial-to-USB adapter or similar device to view output in a serial monitor** (e.g., PuTTY, Tera Term or Microchip Studio’s Data Visualizer).

---

## Installation in Microchip Studio

### Step 1: Download the Library

Clone this repository or download the ZIP from GitHub Releases.

### Step 2: Add Library to Project

1. Create a new project in Microchip Studio:  
   `File > New > Project > GCC C Executable Project`.
2. Select your microcontroller (e.g., ATmega328PB).
3. Copy the `src` folder (containing `debugSerial.h` and `debugSerial.cpp`) to your project directory.
4. In **Solution Explorer**, right-click the project > `Add > Existing Item`.
5. Select `debugSerial.h` and `debugSerial.cpp` from the `src` folder.

### Step 3: Define F_CPU

The library requires `F_CPU` to match your microcontroller’s clock frequency (e.g., `8000000UL` for 8 MHz or `16000000UL` for 16 MHz) for accurate baud rate calculations.

#### Option 1 (Recommended): Define in project settings

- Go to `Project > Properties > Toolchain > AVR/GNU C Compiler > Symbols`.
- Add a symbol: `F_CPU=16000000UL` (or `F_CPU=8000000UL`).

#### Option 2: Define in your source file

```c
#define F_CPU 16000000UL
#include "debugSerial.h"
```

## Adapting for ATmega328P

The ATmega328PB has two UARTs (UART0 and UART1), but the ATmega328P has only one (UART0). To use this library with ATmega328P:

1. Open `debugSerial.cpp`
2. Replace the following UART1 registers with UART0 equivalents:
   - `UBRR1H → UBRR0H`
   - `UBRR1L → UBRR0L`
   - `UCSR1A → UCSR0A`
   - `UCSR1B → UCSR0B`
   - `UCSR1C → UCSR0C`
   - `UDR1 → UDR0`
   - `USART1_UDRE_vect → USART_UDRE_vect`
3. Update the TX pin to PD1 (UART0 TX on ATmega328P).
4. Recompile and upload.

## Limitations

- **Transmit-Only:** The library does not support receiving data.
- **UART1-Specific:** Designed for ATmega328PB’s UART1. Modification required for other UARTs or microcontrollers.
- **Buffer Size:** Fixed at 100 bytes. Overflows are silently ignored.
