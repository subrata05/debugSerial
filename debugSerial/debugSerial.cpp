/*
 * debugSerial.cpp
 *
 * Created: 10-07-2025 20:17:54
 * Author: Subrata
 *
 * Implementation of the debugSerial library for ATmega328PB using UART1.
 * Uses a ring buffer for buffered serial transmission (transmit-only).
 *
 * For ATmega328P:
 * - Replace UART1 registers (UBRR1H, UBRR1L, UCSR1A, UCSR1B, UCSR1C, UDR1, USART1_UDRE_vect)
 *   with UART0 registers (UBRR0H, UBRR0L, UCSR0A, UCSR0B, UCSR0C, UDR0, USART_UDRE_vect).
 * - See README.md for details.
 */

#include "debugSerial.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static debugRingBuffer_t debugTxBuffer;

// -----------------------------------------------------------------------------------
// Ring buffer initialization procedure
// -----------------------------------------------------------------------------------
// Input : debugRingBuffer_t *buf - Pointer to the ring buffer structure to initialize
// Output: void
// Initializes the ring buffer by setting the head and tail indices to zero,
// preparing it for data storage and retrieval.
// -----------------------------------------------------------------------------------
static void debug_buffer_init(debugRingBuffer_t *buf) {
    buf->debugHead = 0;
    buf->debugTail = 0;
}

// -----------------------------------------------------------------------------------
// Ring buffer empty check procedure
// -----------------------------------------------------------------------------------
// Input : debugRingBuffer_t *buf - Pointer to the ring buffer structure to check
// Output: bool - Returns true if the buffer is empty, false otherwise
// Checks if the ring buffer is empty by comparing the head and tail indices.
// Returns true when head equals tail, indicating no data is queued.
// -----------------------------------------------------------------------------------
static bool debug_buffer_is_empty(debugRingBuffer_t *buf) {
    return (buf->debugHead == buf->debugTail);
}

// -----------------------------------------------------------------------------------
// Ring buffer full check procedure
// -----------------------------------------------------------------------------------
// Input : debugRingBuffer_t *buf - Pointer to the ring buffer structure to check
// Output: bool - Returns true if the buffer is full, false otherwise
// Checks if the ring buffer is full by determining if the next head position
// (head + 1, modulo buffer size) equals the tail, indicating no space for new data.
// -----------------------------------------------------------------------------------
static bool debug_buffer_is_full(debugRingBuffer_t *buf) {
    return ((buf->debugHead + 1) % DEBUG_BUFFER_SIZE) == buf->debugTail;
}

// -----------------------------------------------------------------------------------
// Ring buffer data insertion procedure
// -----------------------------------------------------------------------------------
// Input : debugRingBuffer_t *buf - Pointer to the ring buffer structure
// Input : char data - The character to insert into the buffer
// Output: void
// Adds a character to the ring buffer at the head position if the buffer is not full,
// then advances the head index (modulo buffer size). Ignores the data if the buffer is full.
// -----------------------------------------------------------------------------------
static void debug_buffer_put(debugRingBuffer_t *buf, char data) {
    if (!debug_buffer_is_full(buf)) {
        buf->debugBuffer[buf->debugHead] = data;
        buf->debugHead = (buf->debugHead + 1) % DEBUG_BUFFER_SIZE;
    }
}

// -----------------------------------------------------------------------------------
// Ring buffer data retrieval procedure
// -----------------------------------------------------------------------------------
// Input : debugRingBuffer_t *buf - Pointer to the ring buffer structure
// Input : char *data - Pointer to store the retrieved character
// Output: bool - Returns true if a character was retrieved, false if the buffer is empty
// Retrieves a character from the tail of the ring buffer, stores it in *data,
// and advances the tail index (modulo buffer size). Returns false if the buffer is empty.
// -----------------------------------------------------------------------------------
static bool debug_buffer_get(debugRingBuffer_t *buf, char *data) {
    if (!debug_buffer_is_empty(buf)) {
        *data = buf->debugBuffer[buf->debugTail];
        buf->debugTail = (buf->debugTail + 1) % DEBUG_BUFFER_SIZE;
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------------
// UART1 initialization procedure
// -----------------------------------------------------------------------------------
// Input : int32_t debugBaud - The desired baud rate for UART1 (e.g., 9600, 115200)
// Output: void
// Configures the ATmega328PB's UART1 module for serial transmission with the specified
// baud rate, 8-bit data, no parity, and 1 stop bit. Enables double-speed mode (U2X1),
// transmitter (TXEN1), and data register empty interrupt (UDRIE1). Initializes the ring buffer.
// Uses F_CPU to calculate the baud rate register value (UBRR1).
// -----------------------------------------------------------------------------------
void debugSerialBegin(int32_t debugBaud) {
    uint16_t ubrr = (F_CPU / (8UL * debugBaud)) - 1;
    UBRR1H = (uint8_t)(ubrr >> 8);
    UBRR1L = (uint8_t)ubrr;
    UCSR1A |= (1 << U2X1); // Double speed mode
    UCSR1B = (1 << TXEN1) | (1 << UDRIE1); // Enable TX and data register empty interrupt
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // 8-bit data, no parity, 1 stop bit
    
    debug_buffer_init(&debugTxBuffer);
}

// -----------------------------------------------------------------------------------
// UART1 single character transmission procedure
// -----------------------------------------------------------------------------------
// Input : char data - The character to transmit via UART1
// Output: void
// Adds the character to the ring buffer and enables the UART1 data register empty
// interrupt (UDRIE1) if the buffer was previously empty. Disables interrupts (cli)
// during buffer access to prevent race conditions, then re-enables them (sei).
// -----------------------------------------------------------------------------------
void uart1_print_char(char data) {
    cli();
    bool was_empty = debug_buffer_is_empty(&debugTxBuffer);
    debug_buffer_put(&debugTxBuffer, data);
    if (was_empty) {
        UCSR1B |= (1 << UDRIE1);
    }
    sei();
}

// -----------------------------------------------------------------------------------
// String printing procedure
// -----------------------------------------------------------------------------------
// Input : const char *str - Pointer to a null-terminated string to transmit
// Output: void
// Iterates through the string, sending each character to uart1_print_char for
// transmission via the ring buffer. Stops when the null terminator is reached.
// -----------------------------------------------------------------------------------
void debugPrint(const char *str) {
    while (*str) {
        uart1_print_char(*str++);
    }
}

// -----------------------------------------------------------------------------------
// String printing with newline procedure
// -----------------------------------------------------------------------------------
// Input : const char *str - Pointer to a null-terminated string to transmit
// Output: void
// Prints the string using debugPrint, then appends a carriage return ('\r') and
// newline ('\n') to the ring buffer for transmission, simulating a line break.
// -----------------------------------------------------------------------------------
void debugPrintln(const char *str) {
    debugPrint(str);
    uart1_print_char('\r');
    uart1_print_char('\n');
}

// -----------------------------------------------------------------------------------
// Integer printing procedure
// -----------------------------------------------------------------------------------
// Input : int32_t value - The 32-bit integer to transmit
// Output: void
// Converts the integer to its decimal representation. Handles negative numbers by
// printing a minus sign and converting to positive. For zero, prints '0'. Otherwise,
// builds a digit array in reverse order, then transmits digits in correct order via
// the ring buffer.
// -----------------------------------------------------------------------------------
void debugPrintInt(int32_t value) {
    if (value < 0) {
        uart1_print_char('-');
        value = -value;
    }
    
    if (value == 0) {
        uart1_print_char('0');
        return;
    }

    char digits[10];
    uint8_t count = 0;
    
    while (value > 0) {
        digits[count++] = '0' + (value % 10);
        value /= 10;
    }

    while (count > 0) {
        uart1_print_char(digits[--count]);
    }
}

// -----------------------------------------------------------------------------------
// Integer printing with newline procedure
// -----------------------------------------------------------------------------------
// Input : int32_t value - The 32-bit integer to transmit
// Output: void
// Prints the integer using debugPrintInt, then appends a carriage return ('\r') and
// newline ('\n') to the ring buffer for transmission, simulating a line break.
// -----------------------------------------------------------------------------------
void debugPrintIntln(int32_t value) {
    debugPrintInt(value);
    uart1_print_char('\r');
    uart1_print_char('\n');
}

// -----------------------------------------------------------------------------------
// Floating-point number printing procedure
// -----------------------------------------------------------------------------------
// Input : float value - The floating-point number to transmit
// Input : uint8_t decimalPlaces - Number of decimal places to display (0 or more)
// Output: void
// Prints the float by first handling the sign (negative prints '-'). Converts the
// integer part to an integer and prints it using debugPrintInt. If decimalPlaces > 0,
// prints a decimal point and converts the fractional part to digits by repeatedly
// multiplying by 10, extracting, and transmitting each digit.
// -----------------------------------------------------------------------------------
void debugPrintFloat(float value, uint8_t decimalPlaces) {
    if (value < 0) {
        uart1_print_char('-');
        value = -value;
    }
    
    // Handle integer part
    int32_t intPart = (int32_t)value;
    debugPrintInt(intPart);
    
    // Handle decimal part
    if (decimalPlaces > 0) {
        uart1_print_char('.');
        float decimalPart = value - intPart;
        
        // Convert decimal part to integer by multiplying by 10^decimalPlaces
        for (uint8_t i = 0; i < decimalPlaces; i++) {
            decimalPart *= 10;
            int digit = (int)decimalPart;
            uart1_print_char('0' + digit);
            decimalPart -= digit;
        }
    }
}

// -----------------------------------------------------------------------------------
// Floating-point number printing with newline procedure
// -----------------------------------------------------------------------------------
// Input : float value - The floating-point number to transmit
// Input : uint8_t decimalPlaces - Number of decimal places to display (0 or more)
// Output: void
// Prints the float using debugPrintFloat, then appends a carriage return ('\r') and
// newline ('\n') to the ring buffer for transmission, simulating a line break.
// -----------------------------------------------------------------------------------
void debugPrintFloatln(float value, uint8_t decimalPlaces) {
    debugPrintFloat(value, decimalPlaces);
    uart1_print_char('\r');
    uart1_print_char('\n');
}

// -----------------------------------------------------------------------------------
// UART1 data register empty interrupt service routine
// -----------------------------------------------------------------------------------
// Input : None (ISR triggered by hardware)
// Output: None
// Handles the UART1 data register empty interrupt (USART1_UDRE_vect). Retrieves a
// character from the ring buffer and writes it to the UART1 data register (UDR1).
// If the buffer is empty, disables the data register empty interrupt (UDRIE1).
// -----------------------------------------------------------------------------------
ISR(USART1_UDRE_vect) {
    char data;
    if (debug_buffer_get(&debugTxBuffer, &data)) {
        UDR1 = data;
    } else {
        UCSR1B &= ~(1 << UDRIE1);
    }
}