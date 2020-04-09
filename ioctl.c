/*
 * K1600 MIDI Converter - IO Control Routines
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.0
 *
 *  RA3/AN3		- CV1 LED				- output - active high
 *  RA4			- TRIG3 LED				- output - active high
 *  RA5/AN4		- TRIG4 LED				- output - active high
 *
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
 *
 *  RD0			- GATE1 output			- output - active high
 *  RD1			- GATE2 output			- output - active high
 *  RD2			- TRIG1 output			- output - active high
 *  RD3			- TRIG2 output			- output - active high
 *  RD4			- TRIG3 output			- output - active high
 *  RD5			- TRIG4 output			- output - active high
 *  RD6			- clock output			- output - active high
 *  RD7			- reset output			- output - active high
 *
 *  RE0/AN5		- MIDI in LED			- output - active high
 *  RE1/AN6		- MIDI out LED			- output - active high
 *  RE2/AN7		- !TEST MODE			- output - active low
 */
#include <system.h>
#include "ioctl.h"

// hardware defines
#define DAC_CS portc.0
#define CV1_LED porta.3
#define CV2_LED portb.1
#define GATE1_LED portb.2
#define GATE2_LED portb.3
#define TRIG1_LED portb.4
#define TRIG2_LED portb.5
#define TRIG3_LED porta.4
#define TRIG4_LED porta.5
#define RESET_LED portc.1  // new mapping
#define CLOCK_LED portc.2  // new mapping
//#define RESET_LED portc.2  // prototype mapping
//#define CLOCK_LED portc.1  // prototype mapping
#define MIDI_IN_LED porte.0
#define MIDI_OUT_LED porte.1
#define GATE1_OUT portd.0
#define GATE2_OUT portd.1
#define TRIG1_OUT portd.2
#define TRIG2_OUT portd.3
#define TRIG3_OUT portd.4
#define TRIG4_OUT portd.5
#define CLOCK_OUT portd.6
#define RESET_OUT portd.7
#define TEST_PIN porte.2
#define SETUP_SW portb.6
#define SELECT_SW portb.7

#define LED_BLANK 6

// local variables
unsigned int dac0_val;				// current DAC0 value
unsigned int dac1_val;				// current DAC1 value
unsigned int dac0_val_new;			// desired DAC0 value
unsigned int dac1_val_new;			// desired DAC1 value
unsigned char task_phase;			// task phase counter
unsigned char test_active;			// 1 = test mode active, 0 = test mode inactive
unsigned char led_on_count[12];		// LED on counters
unsigned char led_off_count[12];	// LED off counter
unsigned char led_on_time[12];		// LED on time
unsigned char led_off_time[12];		// LED off time
unsigned char led_phase;			// which LED to control this time
unsigned char led_blank;			// blanking counter
unsigned char gate1_out_count;		// GATE1 out counter
unsigned char gate2_out_count;		// GATE1 out counter
unsigned char trig1_out_count;		// TRIG1 out counter
unsigned char trig2_out_count;		// TRIG2 out counter
unsigned char trig3_out_count;		// TRIG3 out counter
unsigned char trig4_out_count;		// TRIG4 out counter
unsigned char reset_out_count;		// reset out counter
unsigned char clock_out_count;		// clock out counter

// local functions
void ioctl_spi_send(unsigned char);
void ioctl_led_blink(void);
void ioctl_pulse_out(void);

// init the stuff
void ioctl_init(void) {
	unsigned char i;

	// set up the SPI port for talking to the DAC and LED register
	trisc.3 = 0;
	trisc.5 = 0;
	DAC_CS = 1;
//	sspstat = 0x00;
	sspstat = 0x80;
	sspcon1 = 0x32;
	dac0_val = 0;
	dac1_val = 0;
	dac0_val_new = 2048;
	dac1_val_new = 2048;
	pir1.SSPIF = 0;
	sspbuf = 0x00; 
	test_active = 0;

	// reset outputs
	CV1_LED = 0;
	CV2_LED = 0;
	GATE2_LED = 0;
	TRIG1_LED = 0;
	TRIG2_LED = 0;
	TRIG3_LED = 0;
	TRIG4_LED = 0;
	RESET_LED = 0;
	CLOCK_LED = 0;
	MIDI_IN_LED = 0;
	MIDI_OUT_LED = 0;
	GATE1_OUT = 0;
	GATE2_OUT = 0;
	TRIG1_OUT = 0;
	TRIG2_OUT = 0;
	TRIG3_OUT = 0;
	TRIG4_OUT = 0;
	RESET_OUT = 0;
	CLOCK_OUT = 0;
	TEST_PIN = 1;

	for(i = 0; i < 12; i ++) {
		led_on_count[i] = 0;
		led_off_count[i] = 0;
		led_on_time[i] = 0;
		led_off_time[i] = 0;
	}
	led_phase = 0;
	led_blank = 0;

	gate1_out_count = 0;
	gate2_out_count = 0;
	trig1_out_count = 0;
	trig2_out_count = 0;
	trig3_out_count = 0;
	trig4_out_count = 0;
	reset_out_count = 0;
	clock_out_count = 0;
}

// runs the task on a timer - every 256uS
void ioctl_timer_task(void) {

	// do DACs
	if(task_phase & 0x01) {
		if(test_active) {
			dac1_val_new = 0;
			dac1_val = 1;  // force an update
		}
		if(dac1_val != dac1_val_new) {
			dac1_val = dac1_val_new;
			DAC_CS = 0;
	//		delay_us(30);
			ioctl_spi_send(0xb0 | ((dac1_val >> 8) & 0x0f));
			delay_us(30);
			ioctl_spi_send(dac1_val & 0xff);
			delay_us(30);
			DAC_CS = 1;
		}
	}
	else {
		if(test_active) {
			dac0_val_new = 0;
			dac0_val = 1;  // force an update
		}
		if(dac0_val != dac0_val_new) {
			dac0_val = dac0_val_new;
			DAC_CS = 0;
	//		delay_us(30);
			ioctl_spi_send(0x30 | ((dac0_val >> 8) & 0x0f));
			delay_us(30);
			ioctl_spi_send(dac0_val & 0xff);
			delay_us(30);
			DAC_CS = 1;
		}
	}
	ioctl_led_blink();
 	// every 1024us
	if((task_phase & 0x03) == 0) {
		ioctl_pulse_out();  // do digital outputs
	}
	task_phase ++;
}

// set the CV1 output value
void ioctl_set_cv1_out(unsigned int val) {
	dac0_val_new = val;
}

// set the CV2 output value
void ioctl_set_cv2_out(unsigned int val) {
	dac1_val_new = val;
}

// set the CV1 LED
void ioctl_set_cv1_led(unsigned char on, unsigned char off) {
	led_on_time[0] = on;
	led_off_time[0] = off;
	led_on_count[0] = on;
	led_off_count[0] = off;
}

// set the CV2 LED
void ioctl_set_cv2_led(unsigned char on, unsigned char off) {
	led_on_time[1] = on;
	led_off_time[1] = off;
	led_on_count[1] = on;
	led_off_count[1] = off;
}

// set the GATE1 LED
void ioctl_set_gate1_led(unsigned char on, unsigned char off) {
	led_on_time[2] = on;
	led_off_time[2] = off;
	led_on_count[2] = on;
	led_off_count[2] = off;
}

// set the GATE2 LED
void ioctl_set_gate2_led(unsigned char on, unsigned char off) {
	led_on_time[3] = on;
	led_off_time[3] = off;
	led_on_count[3] = on;
	led_off_count[3] = off;
}

// set the TRIG1 LED
void ioctl_set_trig1_led(unsigned char on, unsigned char off) {
	led_on_time[4] = on;
	led_off_time[4] = off;
	led_on_count[4] = on;
	led_off_count[4] = off;
}

// set the TRIG2 LED
void ioctl_set_trig2_led(unsigned char on, unsigned char off) {
	led_on_time[5] = on;
	led_off_time[5] = off;
	led_on_count[5] = on;
	led_off_count[5] = off;
}

// set the TRIG3 LED
void ioctl_set_trig3_led(unsigned char on, unsigned char off) {
	led_on_time[6] = on;
	led_off_time[6] = off;
	led_on_count[6] = on;
	led_off_count[6] = off;
}

// set the TRIG4 LED
void ioctl_set_trig4_led(unsigned char on, unsigned char off) {
	led_on_time[7] = on;
	led_off_time[7] = off;
	led_on_count[7] = on;
	led_off_count[7] = off;
}

// set the reset LED
void ioctl_set_reset_led(unsigned char on, unsigned char off) {
	led_on_time[8] = on;
	led_off_time[8] = off;
	led_on_count[8] = on;
	led_off_count[8] = off;
}

// set the clock LED
void ioctl_set_clock_led(unsigned char on, unsigned char off) {
	led_on_time[9] = on;
	led_off_time[9] = off;
	led_on_count[9] = on;
	led_off_count[9] = off;
}

// set the MIDI in LED
void ioctl_set_midi_in_led(unsigned char on, unsigned char off) {
	led_on_time[10] = on;
	led_off_time[10] = off;
	led_on_count[10] = on;
	led_off_count[10] = off;
}

// set the MIDI out LED
void ioctl_set_midi_out_led(unsigned char on, unsigned char off) {
	led_on_time[11] = on;
	led_off_time[11] = off;
	led_on_count[11] = on;
	led_off_count[11] = off;
}


// send a byte on the SPI bus and wait it to be sent
void ioctl_spi_send(unsigned char data) {
	pir1.SSPIF = 0;
	sspbuf = data;
	while(!pir1.SSPIF) clear_wdt();
}

// handle LED blinking and timeouts
void ioctl_led_blink(void) {
	if(led_blank == 0) {
		if(led_on_count[led_phase]) {
			if(led_phase == 0) CV1_LED = 1;
			else if(led_phase == 1) CV2_LED = 1;
			else if(led_phase == 2) GATE1_LED = 1;
			else if(led_phase == 3) GATE2_LED = 1;
			else if(led_phase == 4) TRIG1_LED = 1;
			else if(led_phase == 5) TRIG2_LED = 1;
			else if(led_phase == 6) TRIG3_LED = 1;
			else if(led_phase == 7) TRIG4_LED = 1;
			else if(led_phase == 8) RESET_LED = 1;
			else if(led_phase == 9) CLOCK_LED = 1;
			else if(led_phase == 10) MIDI_IN_LED = 1;
			else if(led_phase == 11) MIDI_OUT_LED = 1;
			if(led_on_count[led_phase] != 255) {
				led_on_count[led_phase] --;
			}
		}
		else {
			if(led_phase == 0) CV1_LED = 0;
			else if(led_phase == 1) CV2_LED = 0;
			else if(led_phase == 2) GATE1_LED = 0;
			else if(led_phase == 3) GATE2_LED = 0;
			else if(led_phase == 4) TRIG1_LED = 0;
			else if(led_phase == 5) TRIG2_LED = 0;
			else if(led_phase == 6) TRIG3_LED = 0;
			else if(led_phase == 7) TRIG4_LED = 0;
			else if(led_phase == 8) RESET_LED = 0;
			else if(led_phase == 9) CLOCK_LED = 0;
			else if(led_phase == 10) MIDI_IN_LED = 0;
			else if(led_phase == 11) MIDI_OUT_LED = 0;
			// if we're blinking - otherwise set off time to 0
			if(led_off_count[led_phase]) {
				led_off_count[led_phase] --;
				if(led_off_count[led_phase] == 0) {
					led_on_count[led_phase] = led_on_time[led_phase];
					led_off_count[led_phase] = led_off_time[led_phase];
				}
			}
		}
	}
	else {
		if(led_phase == 0) CV1_LED = 0;
		else if(led_phase == 1) CV2_LED = 0;
		else if(led_phase == 2) GATE1_LED = 0;
		else if(led_phase == 3) GATE2_LED = 0;
		else if(led_phase == 4) TRIG1_LED = 0;
		else if(led_phase == 5) TRIG2_LED = 0;
		else if(led_phase == 6) TRIG3_LED = 0;
		else if(led_phase == 7) TRIG4_LED = 0;
		else if(led_phase == 8) RESET_LED = 0;
		else if(led_phase == 9) CLOCK_LED = 0;
		else if(led_phase == 10) MIDI_IN_LED = 0;
		else if(led_phase == 11) MIDI_OUT_LED = 0;
	}
	led_phase ++;
	if(led_phase == 12) {
		led_phase = 0;
		led_blank ++;
		if(led_blank == LED_BLANK) {
			led_blank = 0;
		}
	}
}

// control the pulse outputs
void ioctl_pulse_out(void) {
	if(gate1_out_count) {
		GATE1_OUT = 1;
		if(gate1_out_count != 255) gate1_out_count --;
	}
	else {
		GATE1_OUT = 0;
	}
	if(gate2_out_count) {
		GATE2_OUT = 1;
		if(gate2_out_count != 255) gate2_out_count --;
	}
	else {
		GATE2_OUT = 0;
	}
	if(trig1_out_count) {
		TRIG1_OUT = 1;
		if(trig1_out_count != 255) trig1_out_count --;
	}
	else {
		TRIG1_OUT = 0;
	}
	if(trig2_out_count) {
		TRIG2_OUT = 1;
		if(trig2_out_count != 255) trig2_out_count --;
	}
	else {
		TRIG2_OUT = 0;
	}
	if(trig3_out_count) {
		TRIG3_OUT = 1;
		if(trig3_out_count != 255) trig3_out_count --;
	}
	else {
		TRIG3_OUT = 0;
	}
	if(trig4_out_count) {
		TRIG4_OUT = 1;
		if(trig4_out_count != 255) trig4_out_count --;
	}
	else {
		TRIG4_OUT = 0;
	}
	if(reset_out_count) {
		RESET_OUT = 1;
		if(reset_out_count != 255) reset_out_count --;
	}
	else {
		RESET_OUT = 0;
	}
	if(clock_out_count) {
		CLOCK_OUT = 1;
		if(clock_out_count != 255) clock_out_count --;
	}
	else {
		CLOCK_OUT = 0;
	}
}

// set the GATE1 out
void ioctl_set_gate1_out(unsigned char val) {
	gate1_out_count = val;
}

// set the GATE2 out
void ioctl_set_gate2_out(unsigned char val) {
	gate2_out_count = val;
}

// set the TRIG1 out
void ioctl_set_trig1_out(unsigned char val) {
	if(val) TRIG1_OUT = 1;
	trig1_out_count = val;
}

// set the TRIG2 out
void ioctl_set_trig2_out(unsigned char val) {
	if(val) TRIG2_OUT = 1;
	trig2_out_count = val;
}

// set the TRIG3 out
void ioctl_set_trig3_out(unsigned char val) {
	if(val) TRIG3_OUT = 1;
	trig3_out_count = val;
}

// set the TRIG4 out
void ioctl_set_trig4_out(unsigned char val) {
	if(val) TRIG4_OUT = 1;
	trig4_out_count = val;
}

// set the reset out
void ioctl_set_reset_out(unsigned char val) {
	if(val) RESET_OUT = 1;
	reset_out_count = val;
}

// set the clock out
void ioctl_set_clock_out(unsigned char val) {
	if(val) CLOCK_OUT = 1;
	clock_out_count = val;
}

// gets the state of the setup switch
unsigned char ioctl_get_setup_sw(void) {
	if(!SETUP_SW) return 1;
	return 0;
}

// gets the state of the select switch
unsigned char ioctl_get_select_sw(void) {
	if(!SELECT_SW) return 1;
	return 0;
}

// sets the state of the TEST pin - 1 = assert test mode, 0 = high Z
void ioctl_set_test_pin(unsigned char mode) {
	if(mode == 1) {
		TEST_PIN = 0;
	}
	else {
		TEST_PIN = 1;
	}
}
