/*
 * K1600 MIDI Converter - Setup Control
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.1
 *
 * Ver. 1.1 changes:
 *  - added arp mode
 *
 */
#include <system.h>
#include "setup.h"
#include "ioctl.h"
#include "config_store.h"

// config
#define SETUP_MODE_BLINK_PHASE_A 0
#define SETUP_MODE_BLINK_PHASE_B 32	
#define SETUP_MODE_BLINK_LEN 5			// the pulse LED for blinking the LEDs
#define SETUP_MODE_BLINK_LEN_ARP 10		// the pulse LED for blinking the LEDs
#define SELECT_SW_TIME 3				// select switch trigger time
#define SETUP_MODE_TIMEOUT 900			// timeout * 4ms

unsigned char setup_mode;
unsigned char setup_mode_old;
unsigned char setup_mode_blink;
unsigned char select_sw_count;
unsigned char select_sw_state;
unsigned int setup_mode_timeout;
unsigned char setup_enabled;

// function prototypes
void setup_blinking(void);
void setup_mode_next(void);
void setup_reset(void);

// init the setup control
void setup_init(void) {
	unsigned char i, clear_count;
	setup_mode = SETUP_MODE_NONE;
	setup_mode_old = setup_mode;
	setup_mode_blink = 0;
	select_sw_count = 0;
	select_sw_state = 0;
	setup_mode_timeout = 0;
	setup_enabled = 0;
	clear_count = 0;

	// check if the setup switch is held down at startup
	if(ioctl_get_select_sw()) {
		for(i = 0; i < 10; i ++) {
			if(ioctl_get_select_sw()) clear_count ++;
			delay_ms(20);
		}
		if(clear_count == 10) {
			setup_reset();
		}
		while(ioctl_get_select_sw()) {
			ioctl_timer_task();
			delay_us(200);
		}
		for(i = 0; i < 10; i ++) {
			clear_wdt();
			ioctl_timer_task();
			delay_ms(10);
		}
	}
	// check if the config memory has been reset
	else if(config_store_get_val(CONFIG_SETUP_TOKEN) != 0x00) {
		setup_reset();
	}
}

// run the setup timer task - every 4ms
void setup_timer_task(void) {
	// the select switch is pushed down
	if(ioctl_get_select_sw()) {
		select_sw_count = 16;
		if(select_sw_state == 0) {
			select_sw_state = 1;
			setup_mode_next();
		}
	}
	else {
		if(select_sw_count) {
			select_sw_count --;
		}
		if(select_sw_count == 0) {
			select_sw_state = 0;
		}
	}

	// the setup switch is up
	if(ioctl_get_setup_sw()) {
		// reset the setup mode timeout
		setup_mode_timeout = SETUP_MODE_TIMEOUT;
		if(!setup_enabled) {
			setup_enabled = 1;
			if(setup_mode == SETUP_MODE_NONE) {
				setup_mode_next();
			}
		}
	}
	// get out of the setup mode
	else if(setup_enabled) {
		setup_enabled = 0;
		setup_mode_timeout = 0;
		setup_mode_cancel();
	}

	// handle timeout of the select mode
	if(setup_mode_timeout) {
		setup_mode_timeout --;
		if(setup_mode_timeout == 0) {
			setup_mode_cancel();
		}
	}

	// handle the blinking of LEDs
	setup_blinking();
}

// setup mode blinking
void setup_blinking(void) {
    static int blink_timer = 0;
	if(setup_mode == SETUP_MODE_NONE) return;

	// normal blinking
	if(setup_mode_blink == SETUP_MODE_BLINK_PHASE_A) {
		// CV1
		if(setup_mode == SETUP_MODE_CV1) {
			ioctl_set_cv1_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_gate1_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_cv1_out(CV_TEST_VAL);
				ioctl_set_gate1_out(50);
			}
		}
		// CV2
		else if(setup_mode == SETUP_MODE_CV2) {
			ioctl_set_cv2_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_gate2_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_cv2_out(CV_TEST_VAL);
				ioctl_set_gate2_out(50);
			}
		}
		// CV split - phase A
		else if(setup_mode == SETUP_MODE_CV_SPLIT) {
			ioctl_set_cv1_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_gate1_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_cv1_out(CV_TEST_VAL);
				ioctl_set_gate1_out(50);
			}
		}
		// CV poly
		else if(setup_mode == SETUP_MODE_CV_POLY) {
			ioctl_set_cv1_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_cv2_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_gate1_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_gate2_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_cv1_out(CV_TEST_VAL);
				ioctl_set_cv2_out(CV_TEST_VAL);
				ioctl_set_gate1_out(50);
				ioctl_set_gate2_out(50);
			}
		}
		// CV ARP - phase A
		else if(setup_mode == SETUP_MODE_CV_ARP) {
			ioctl_set_cv1_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			ioctl_set_cv2_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			ioctl_set_gate1_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			ioctl_set_gate2_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			if(setup_enabled) {
				ioctl_set_cv1_out(CV_TEST_VAL);
				ioctl_set_cv2_out(CV_TEST_VAL);
				ioctl_set_gate1_out(50);
				ioctl_set_gate2_out(50);
			}
		}
        // CV VELO
		else if(setup_mode == SETUP_MODE_CV_VELO) {
			ioctl_set_cv1_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			ioctl_set_gate1_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			ioctl_set_gate2_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			if(setup_enabled) {
				ioctl_set_cv1_out(CV_TEST_VAL);
				ioctl_set_cv2_out(CV_TEST_VAL);
				ioctl_set_gate1_out(50);
				ioctl_set_gate2_out(50);
			}
		}
		// TRIG1
		else if(setup_mode == SETUP_MODE_TRIG1) {
			ioctl_set_trig1_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_trig1_out(50);
			}
		}
		// TRIG2
		else if(setup_mode == SETUP_MODE_TRIG2) {
			ioctl_set_trig2_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_trig2_out(50);
			}
		}
		// TRIG3
		else if(setup_mode == SETUP_MODE_TRIG3) {
			ioctl_set_trig3_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_trig3_out(50);
			}
		}
		// TRIG4
		else if(setup_mode == SETUP_MODE_TRIG4) {
			ioctl_set_trig4_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_trig4_out(50);
			}
		}
		// MIDI internal
		else if(setup_mode == SETUP_MODE_INTERNAL) {
			ioctl_set_midi_in_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_midi_out_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_test_pin(1);
			}
			else {
				ioctl_set_test_pin(0);
			}
		}
	}
	// offset blinking
	else if(setup_mode_blink == SETUP_MODE_BLINK_PHASE_B) {
		// CV split - phase B
		if(setup_mode == SETUP_MODE_CV_SPLIT) {
			ioctl_set_cv2_led(SETUP_MODE_BLINK_LEN, 0);
			ioctl_set_gate2_led(SETUP_MODE_BLINK_LEN, 0);
			if(setup_enabled) {
				ioctl_set_cv2_out(CV_TEST_VAL);
				ioctl_set_gate2_out(50);
			}
		}
		// CV ARP - phase B
		if(setup_mode == SETUP_MODE_CV_ARP) {
			ioctl_set_cv1_led(SETUP_MODE_BLINK_LEN_ARP, 0);
			ioctl_set_gate1_led(SETUP_MODE_BLINK_LEN_ARP, 0);
		}
	}

    // CV VELO fading in of CV2
	else if(setup_mode == SETUP_MODE_CV_VELO) {
		ioctl_set_cv2_led(blink_timer & 0x180, 0);
    }
    blink_timer = (blink_timer + 1) & 0x1ff;

	setup_mode_blink = (setup_mode_blink + 1) & 0x3f;	
}

// gets the current setup mode
unsigned char setup_get_mode(void) {
	return setup_mode;
}

// change to the next setup mode
void setup_mode_next(void) {
	// use the last valid mode
	if(setup_mode == 0 && setup_mode_old != 0) setup_mode = setup_mode_old;
	// move to the next setup mode
	else setup_mode ++;
	// cancel setup
	if(setup_mode >= MAX_SETUP_MODES) {
		setup_mode = 0;
		setup_mode_old = 0;
	}
	else {
		setup_mode_timeout = SETUP_MODE_TIMEOUT;
	}
	// reset the mode blinking
	setup_mode_blink = 0;
}

// cancel setup mode
void setup_mode_cancel(void) {
	// make sure test mode is off
	if(setup_mode == SETUP_MODE_INTERNAL) {
		ioctl_set_test_pin(0);
	}
	setup_mode_old = setup_mode;
	setup_mode = 0;
	setup_mode_timeout = 0;
}

// reset the setup - for manual clear or new firmware load
void setup_reset(void) {
	unsigned char i;
	for(i = 0; i < CONFIG_MAX; i ++) {
		config_store_set_val(i, 0);
	}
	ioctl_set_cv1_led(10, 0);
	ioctl_set_cv2_led(10, 0);
	ioctl_set_gate1_led(10, 0);
	ioctl_set_gate2_led(10, 0);
	ioctl_set_trig1_led(10, 0);
	ioctl_set_trig2_led(10, 0);
	ioctl_set_trig3_led(10, 0);
	ioctl_set_trig4_led(10, 0);
	ioctl_set_clock_led(10, 0);
	ioctl_set_reset_led(10, 0);
	ioctl_set_midi_in_led(10, 0);
	ioctl_set_midi_out_led(10, 0);
}
