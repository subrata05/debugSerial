/*
 * debugSerial.h
 *
 * Created: 10-07-2025 20:17:39
 * Author: Subrata
 *
 * A lightweight UART1 debugging library for ATmega328PB micro-controllers.
 * Provides functions to print strings, integers, and floating-point numbers via UART1.
 * Note: This library is transmit-only and does not support receiving data.
 *
 * Usage in Microchip Studio:
 * 1. Define F_CPU in Project Properties > Toolchain > AVR/GNU C Compiler > Symbols.
 *    Example: Add F_CPU=8000000UL or F_CPU=16000000UL.
 * 2. Include this header: #include "debugSerial.h"
 * 3. Call debugSerialBegin(baud) to initialize UART1, then use debugPrint* functions.
 *
 * For ATmega328P (which has only UART0):
 * - Modify debugSerial.cpp to use UART0 registers (e.g., UBRR0H, UBRR0L, UCSR0A, etc.).
 * - See README.md for detailed instructions.
 *
 * Note: F_CPU must match your micro-controller's clock frequency for correct baud rates.
 */

#ifndef DEBUGSERIAL_H_
#define DEBUGSERIAL_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef F_CPU
#error "F_CPU must be defined (e.g., F_CPU=8000000UL or F_CPU=16000000UL) in project settings or source file."
#endif

#define DEBUG_BUFFER_SIZE 100

// Ring buffer structure for UART1 transmission
typedef struct {
    char debugBuffer[DEBUG_BUFFER_SIZE];
    volatile uint8_t debugHead;
    volatile uint8_t debugTail;
} debugRingBuffer_t;

// Function prototypes
void debugSerialBegin(int32_t baud);
void uart1_print_char(char data);
void debugPrint(const char *str);
void debugPrintln(const char *str);
void debugPrintInt(int32_t value);
void debugPrintIntln(int32_t value);
void debugPrintFloat(float value, uint8_t decimalPlaces);
void debugPrintFloatln(float value, uint8_t decimalPlaces);

#endif /* DEBUGSERIAL_H_ */