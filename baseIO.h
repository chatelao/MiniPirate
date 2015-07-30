/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project.
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef baseIO_h
#define baseIO_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
/*
//manage user terminal input
unsigned int bpUserNumberPrompt(unsigned int maxBytes, unsigned int maxValue, unsigned int defValue);

//manage user terminal input
unsigned int bpGetUserInput(unsigned int *currentByte, unsigned int maxBytes, unsigned char *terminalInput );

//reads forward to grab ASCII number string, returns byte value
unsigned int bpGetASCIInumber(unsigned int totalBytes, unsigned int *currentByte, unsigned char *commandArr);

//convert ASCII number string (HEX, BIN, DEC) to INT value
unsigned int ASCII2INT(unsigned char totalDigits, unsigned char *numberArr);

//echo ASCII 0 or 1, given unsigned char c
//#define bpEchoState(c) UART1TX(c+0x30) 
void bpEchoState(unsigned int c);
*/

//
//
//functions for terminal output of various types and lengths
//
//

//Write a string to the user terminal
void bpWstring(char const *s);

//write a string and line break to the user terminal
void bpWline(char const *s); 

//output an 8bit/byte hex value to the user terminal
void bpWhex(unsigned int c);

//output an 16bit/2byte hex value to the user terminal
void bpWinthex(unsigned int c); 

//output an 32bit/4byte hex value to the user terminal
void bpWlonghex(unsigned long int c);

//output an 8bit/byte binary value to the user terminal
void bpWbin(unsigned char c);

//output an 8bit/byte decimal value to the user terminal
void bpWdec(unsigned char c);

//output an 16bit/integer decimal value to the user terminal
void bpWintdec(unsigned int i);

//output an 32bit/long decimal value to the user terminal
void bpWlongdec(unsigned long l);
//friendly version
void bpWlongdecf(unsigned long l);

//print an ADC measurement in decimal form
void bpWvolts(unsigned int adc);

//pseudofuncion break sequences
//#define bpBR UART1TX('\x0D');UART1TX('\x0A') //macro for line break

#define bpBR bpWline("");		// saves 258 bytes..  \\o \o/ o//

#define bpWBR bpBR
#define bpSP UART1TX(' ') //macro for space
#define bpWchar(x) UART1TX(x)

/*
//
//
// Base user terminal UART functions
//
//
#define UART_NORMAL_SPEED 34
#define UART_FAST_SPEED    3

// interrupt transfer related stuff
extern unsigned char *UART1RXBuf;
extern unsigned int UART1RXToRecv;
extern unsigned int UART1RXRecvd;
extern unsigned char *UART1TXBuf;
extern unsigned int UART1TXSent;
extern unsigned int UART1TXAvailable;

// starts interrupt TX
void UART1TXInt(void);

//is byte available in RX buffer?
unsigned char UART1RXRdy(void); 
unsigned char UART1TXRdy(void);
//get a byte from UART
unsigned char UART1RX(void);
void WAITTXEmpty(void);
*/

//add byte to buffer, pause if full
//uses PIC 4 byte UART FIFO buffer
void UART1TX(char c); 

/*
void ClearCommsError(void);
unsigned char CheckCommsError(void);
// sets the uart baudrate generator
void UART1Speed(unsigned char brg);
//Initialize the terminal UART for the speed currently set in bpConfig.termSpeed
void InitializeUART1(void);
unsigned char UART1TXRdy(void);
//
//
// Ring buffer for UART
//
//
void UARTbufService(void);
void UARTbufFlush(void);
void UARTbufSetup(void);
void UARTbuf(char c);
void bpWhexBuf(unsigned int c); //write a hex value to ring buffer


unsigned char USBUSARTIsTxTrfReady(void);
//void putUSBUSART(char *data, unsigned char length);

//
// Hardware functions and definitions
//
// Read the lower 16 bits from programming flash memory
unsigned int bpReadFlash(unsigned int page, unsigned int addr);
// Read the full 24 bits from programming flash memory
unsigned long int bpReadFlashFull(unsigned int page, unsigned int addr);
*/

void printStrDec(char const *str, int dec_nbre,int pad_digits=0);
void printStrHex(char const *str, int hex_nbre);
void printStrBin(char const *str, int bin_nbre);


char pollPeek();

boolean isNumberPeek();
boolean isBlankPeek();
boolean isNumberOrBlankPeek();

char pollSerial();
char pollLowSerial();
void pollBlanks();
//
// Parse BIN (0bXXX), HEX (0xXXX) and DEC (XXX)
//
int pollInt();
int peekSerial();
int pollPin();
void printHighLow(int value);
void printPin(int pin);
void printPorts();
void printPortsQuick();

inline int getPinMode( int i )
	{
	return *portModeRegister(digitalPinToPort(i)) & digitalPinToBitMask(i);
	}

#endif

