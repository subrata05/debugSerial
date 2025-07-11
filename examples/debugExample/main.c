/*
 * main.c
 * Example for using debugSerial library in Microchip Studio with ATmega328PB.
 *
 * This example demonstrates printing strings, integers, and floating-point numbers
 * using the debugSerial library via UART1 (TX pin: PD3).
 *
 * Setup Instructions:
 * 1. Define F_CPU in Project Properties > Toolchain > AVR/GNU C Compiler > Symbols
 *    (e.g., F_CPU=16000000UL for 16 MHz or F_CPU=8000000UL for 8 MHz).
 *    Alternatively, define F_CPU in this file before including debugSerial.h.
 * 2. Connect UART1 TX (PD3) to a serial-to-USB adapter.
 * 3. Open a serial monitor (e.g., PuTTY or Microchip Studio’s Data Visualizer) at 9600 baud.
 *
 * For ATmega328P: Modify debugSerial.cpp to use UART0 registers (e.g., UBRR0H, UDR0)
 * and use TX pin PD1. See README.md for details.
 */

#define F_CPU 16000000UL
#include "debugSerial.h"

int main(void) {
    debugSerialBegin(9600); // Initialize UART1 at 9600 baud
    
    // Example usage
    debugPrintln("Debug Serial Library Test"); // Print string with newline
    debugPrint("Integer: ");
    debugPrintIntln(12345); // Print positive integer with newline
    debugPrint("Negative Integer: ");
    debugPrintIntln(-6789); // Print negative integer with newline
    debugPrint("Float: ");
    debugPrintFloatln(3.14159, 3); // Print float with 3 decimal places and newline
    debugPrint("No Decimals: ");
    debugPrintFloatln(42.718, 0); // Print float with no decimals and newline
    
    while (1) {
        // Main loop - keep the program running
    }
    
    return 0;
}
