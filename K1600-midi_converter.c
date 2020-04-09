/*
 * K1600 MIDI Converter
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.3
 *
 * Hardware I/O:
 *
 *  RA0/AN0		- n/c
 *  RA1/AN1		- n/c
 *  RA2/AN2 	- n/c
 *  RA3/AN3		- CV1 LED				- output - active high
 *  RA4			- TRIG3 LED				- output - active high
 *  RA5/AN4		- TRIG4 LED				- output - active high
 *  RA6			- crystal
 *  RA7			- crystal
 *
 *  RB0/INT0	- MIDI OUT INT			- int0 input
 *  RB1			- CV2 LED				- output - active high
 *  RB2			- GATE1 LED				- output - active high
 *  RB3			- GATE2 LED				- output - active high
 *  RB4/AN11	- TRIG1 LED				- output - active high
 *  RB5			- TRIG2 LED				- output - active high
 *  RB6 		- setup sw.				- input - active low
 *	RB7			- select sw.			- input - active low
 *
 *  RC0			- DAC !CS				- output - active low
 *  RC1			- reset LED				- output - active high
 *  RC2			- clock LED				- output - active high
 *  RC3			- SPI CLK				- SPI
 *  RC4			- SPI MISO (unused)		- SPI
 *  RC5			- SPI MOSI				- SPI
 *  RC6			- MIDI TX				- USART
 *  RC7			- MIDI RX				- USART
 *
 *  RD0			- GATE1 output			- output - active high
 *  RD1			- GATE2 output			- output - active high
 *  RD2			- TRIG1 output			- output - active high
 *  RD3			- TRIG2 output			- output - active high
 *  RD4			- TRIG3 output			- output - active high
 *  RD5			- TRIG4 output			- output - active high
 *  RD6			- CLOCK output			- output - active high
 *  RD7			- RESET output			- output - active high
 *
 *  RE0/AN5		- MIDI in LED			- output - active high
 *  RE1/AN6		- MIDI out LED			- output - active high
 *  RE2/AN7		- !TEST MODE			- output - active low
 *  RE3			- VPP
 */
#include <system.h>
#include "ioctl.h"
#include "midi.h"
#include "sysex.h"
#include "event.h"
#include "setup.h"
#include "voice.h"
#include "config_store.h"

// master clock frequency
#pragma CLOCK_FREQ 32000000

// XXX - this does not survive the bootloader - set to 0xff to simulate
// actual production behaviour
#pragma DATA _EEPROM, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff

// PIC18F4520 config fuses
//#pragma DATA 	_CONFIG1H, 0x08  // internal osc, RA6,7 IO
#pragma DATA 	_CONFIG1H, 0x06  // HS + PLL 
#pragma DATA 	_CONFIG2L, 0x00  // BOR disable, PWRTEN enable
#pragma DATA 	_CONFIG2H, 0x0f  // WDT disable
#pragma DATA 	_CONFIG3H, 0x03  // MCLR disable, PORTB digital
#pragma DATA 	_CONFIG4L, 0x81  // DEBUG disable, LVP disable, STVREN enable
#pragma DATA 	_CONFIG5L, 0xff
#pragma DATA 	_CONFIG5H, 0xff
#pragma DATA 	_CONFIG6L, 0xff
#pragma DATA 	_CONFIG6H, 0xff
#pragma DATA 	_CONFIG7L, 0xff
#pragma DATA 	_CONFIG7H, 0xff

#define MIDI_OUT_LED_LEN 2

unsigned char task_div = 0;

// main!
void main(void) {
	// analog inputs
	adcon0 = 0x00;  // disable
	adcon1 = 0x0f;  // all digital I/O

	// I/O pins
	trisa = 0x00; 
	porta = 0x00;
	trisb = 0xc1;
	portb = 0x00;
	intcon2.NOT_RBPU = 0;  // weak pullups on port B enabled
						   // needed for NT input
	trisc = 0xc0;
	portc = 0x00;
	trisd = 0x00;
	portd = 0x00;
	trise = 0x00;
	porte = 0xf8;

	// set up USART
	trisc.6 = 0;
	trisc.7 = 1;
	spbrg = 63;  // 31250bps with 32MHz oscillator
	txsta.BRGH = 1; // high speed
	txsta.SYNC = 0;  // async mode
	rcsta.SPEN = 1;  // serial port enable
	pie1.RCIE = 0;  // no interrupts
	rcsta.RX9 = 0;  // 8 bit reception
	txsta.TX9 = 0;  // 8 bit transmit
	rcsta.CREN = 1;  // enable receiver
	txsta.TXEN = 1; // enable transmit

	// timer 1 - task timer - 1us per count
	t1con = 0x31;
	tmr1h = 0xff;
	tmr1l = 0x00;

	// set up modules
	ioctl_init();
	midi_init();
	sysex_init();
	config_store_init();  // this must be before event / voice and after MIDI
	setup_init();  // this must be after config_store_init and after ioctl_init
	event_init();  // this must be after setup and config init
	voice_init();  // this must be after setup and config init

	// set up interrupts
	intcon2.INTEDG0 = 0;  // needed for transistor INT input

	intcon = 0x00;
	intcon.PEIE = 1;
//	pie1.TMR1IE = 1;  // timer 1 - task timer
	pie1.RCIE = 1;
	intcon.INT0IE = 1;  // int0 - MIDI receive sense
	intcon.GIE = 1;

	while(1) {
		clear_wdt();
		midi_tx_task();
		midi_rx_task();
	 	// timer 1 task timer - 256us interval
		if(pir1.TMR1IF) {
			pir1.TMR1IF = 0;
			tmr1h = 0xff;
			tmr1l = 0x00;
			ioctl_timer_task();
			task_div ++;
			// do stuff every 4ms
			if(task_div == 0x01) {
				setup_timer_task();
			}
			else if(task_div == 0x08) {
				config_store_timer_task();
			}
			else if(task_div == 0x0f) {
				voice_timer_task();
				task_div = 0;
			}
		}
	}
}

// interrupt
void interrupt(void) {
	// MIDI input for blinking the out/thru LED
	if(intcon.INT0IF) {
		intcon.INT0IF = 0;
		ioctl_set_midi_out_led(MIDI_OUT_LED_LEN, 0);
	}

	// MIDI receive
	if(pir1.RCIF) {
		midi_rx_byte(rcreg);
		// clear errors
		if(rcsta.FERR || rcsta.OERR) {
			rcsta.CREN = 0;
			rcsta.CREN = 1;
		}
	}
}

