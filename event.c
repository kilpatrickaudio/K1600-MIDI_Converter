/*
 * K1600 MIDI Converter - Event Mapper
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
#include "event.h"
#include "midi.h"
#include "midi_callbacks.h"
#include "ioctl.h"
#include "setup.h"
#include "voice.h"
#include "config_store.h"
#include "sysex.h"

// configs
#define CV_LED_LEN 3
#define TRIG_OUT_LEN 10
#define TRIG_LED_LEN 3
#define CLOCK_OUT_LEN 5
#define CLOCK_LED_LEN 2
#define RESET_OUT_LEN 5
#define RESET_LED_LEN 2
#define MIDI_IN_LED_LEN 2
#define PITCH_BEND_TRIG_UP 0x27ff
#define PITCH_BEND_TRIG_DOWN 0x17ff

// event mappings
#define EVENT_MAP_UNASSIGNED 0
#define EVENT_MAP_NOTE 1
#define EVENT_MAP_CC 2
#define EVENT_MAP_PITCH_BEND 3

// CV/gate
unsigned char cv1_map;  // CV1 event mapping
unsigned char cv2_map;  // CV2 event mapping
unsigned char cv1_chan;  // CV1 receive channel
unsigned char cv2_chan;  // CV2 receive channel
unsigned char cv1_val;  // CV1 CC assignment / bend dir
unsigned char cv2_val;  // CV2 CC assignment / bend dir

// triggers
unsigned char trig1_map;  // TRIG1 event mapping
unsigned char trig2_map;  // TRIG2 event mapping
unsigned char trig3_map;  // TRIG3 event mapping
unsigned char trig4_map;  // TRIG4 event mapping
unsigned char trig1_chan;  // TRIG1 receive channel
unsigned char trig2_chan;  // TRIG2 receive channel
unsigned char trig3_chan;  // TRIG3 receive channel
unsigned char trig4_chan;  // TRIG4 receive channel
unsigned char trig1_val;  // TRIG1 note / CC number / bend dir
unsigned char trig2_val;  // TRIG2 note / CC number / bend dir
unsigned char trig3_val;  // TRIG3 note / CC number / bend dir
unsigned char trig4_val;  // TRIG4 note / CC number / bend dir

// debug DAC values
unsigned char cv1_testl;	// CV1 test - lower 5 bits (left just)
unsigned char cv1_testh;    // CV1 test - upper 7 bits (right just)
unsigned char cv2_testl;	// CV2 test - lower 5 bits (left just)
unsigned char cv2_testh;    // CV2 test - upper 7 bits (right just)

// clock
unsigned char clock_enabled;  // enable control to follow MIDI start, continue, stop
unsigned char clock_div;  // the clock divide ratio
unsigned char clock_count;  // the clock counter
unsigned char start_arm;  // cause the clock to pulse on the next tick - for SPP and START messages

// local functions
void event_blink_in(void);

// init the event mapper
void event_init(void) {
	// CV init
	cv1_map = config_store_get_val(CONFIG_CV1_MAP);
	cv2_map = config_store_get_val(CONFIG_CV2_MAP);
	cv1_chan = config_store_get_val(CONFIG_CV1_CHAN);
	cv2_chan = config_store_get_val(CONFIG_CV2_CHAN);
	cv1_val = config_store_get_val(CONFIG_CV1_VAL);
	cv2_val = config_store_get_val(CONFIG_CV2_VAL);

	// trigger init
	trig1_map = config_store_get_val(CONFIG_TRIG1_MAP);
	trig2_map = config_store_get_val(CONFIG_TRIG2_MAP);
	trig3_map = config_store_get_val(CONFIG_TRIG3_MAP);
	trig4_map = config_store_get_val(CONFIG_TRIG4_MAP);
	trig1_chan = config_store_get_val(CONFIG_TRIG1_CHAN);
	trig2_chan = config_store_get_val(CONFIG_TRIG2_CHAN);
	trig3_chan = config_store_get_val(CONFIG_TRIG3_CHAN);
	trig4_chan = config_store_get_val(CONFIG_TRIG4_CHAN);
	trig1_val = config_store_get_val(CONFIG_TRIG1_VAL);
	trig2_val = config_store_get_val(CONFIG_TRIG2_VAL);
	trig3_val = config_store_get_val(CONFIG_TRIG3_VAL);
	trig4_val = config_store_get_val(CONFIG_TRIG4_VAL);

	// clock init
	clock_enabled = 0;
	clock_div = config_store_get_val(CONFIG_CLOCK_DIV);
	clock_count = 0;
	start_arm = 0;

	// initialize outputs
	cv1_testl = CV_ZERO_VAL & 0xff;
	cv1_testh = CV_ZERO_VAL >> 8;
	cv2_testl = CV_ZERO_VAL & 0xff;
	cv2_testh = CV_ZERO_VAL >> 8;
	ioctl_set_cv1_out(CV_ZERO_VAL);
	ioctl_set_cv2_out(CV_ZERO_VAL);
}

// blink the MIDI input LED
void event_blink_in(void) {
	ioctl_set_midi_in_led(MIDI_IN_LED_LEN, 0);
}

//
// MIDI CALLBACKS
//
//
// CHANNEL MESSAGES
//
// note off - note on with velocity = 0 calls this
void _midi_rx_note_off(unsigned char channel, 
		unsigned char note) {
	// CV/gate - note off
	if(cv1_map == EVENT_MAP_NOTE && cv1_chan == channel) {
		voice_note_off(0, note);
	}
	if(cv2_map == EVENT_MAP_NOTE && cv2_chan == channel) {
		voice_note_off(1, note);
	}

	// echo and blink
	_midi_tx_note_off(channel, note);
	event_blink_in();
}

// note on - note on with velocity > 0 calls this
void _midi_rx_note_on(unsigned char channel, 
		unsigned char note, 
		unsigned char velocity) {
	if(channel == 15) return;  // channel 16 is not used for notes

	//
	// SETUP
	//
	// note setup mode
	unsigned char temp = setup_get_mode();
	if(temp != SETUP_MODE_NONE) {
		if(temp == SETUP_MODE_CV1) {
			event_set_cv(0, EVENT_MAP_NOTE, channel, 0);
			voice_set_mode(VOICE_MODE_SINGLE, 0);
			voice_set_unit(0);  // always unit 0
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_CV2) {
			event_set_cv(1, EVENT_MAP_NOTE, channel, 0);
			voice_set_mode(VOICE_MODE_SINGLE, 0);
			voice_set_unit(0);  // always unit 0
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_CV_SPLIT) {
			event_set_cv(0, EVENT_MAP_NOTE, channel, 0);
			event_set_cv(1, EVENT_MAP_NOTE, channel, 0); 
			voice_set_mode(VOICE_MODE_SPLIT, note);
			voice_set_unit(0);  // always unit 0
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_CV_POLY) {
			event_set_cv(0, EVENT_MAP_NOTE, channel, 0);
			event_set_cv(1, EVENT_MAP_NOTE, channel, 0);
			voice_set_mode(VOICE_MODE_POLY, 0);
			if(note == 62) voice_set_unit(1);  // D4
			else if(note == 64) voice_set_unit(2);  // E4
			else if(note == 65) voice_set_unit(3);  // F4
			else if(note == 67) voice_set_unit(4);  // G4
			else if(note == 69) voice_set_unit(5);  // A4
			else if(note == 71) voice_set_unit(6);  // B4
			else if(note == 72) voice_set_unit(7);  // C5
			else voice_set_unit(0);  // C4 (or default)
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_CV_ARP) {
			event_set_cv(0, EVENT_MAP_NOTE, channel, 0);
			event_set_cv(1, EVENT_MAP_NOTE, channel, 0); 
			voice_set_mode(VOICE_MODE_ARP, 0);
			voice_set_unit(0);  // always unit 0
			setup_mode_cancel();
		}
		// set up velocity mode on CV/gate 1 and CV/gate 2
		else if(temp == SETUP_MODE_CV_VELO) {
			event_set_cv(0, EVENT_MAP_NOTE, channel, 0);
			event_set_cv(1, EVENT_MAP_NOTE, channel, 0); 
			voice_set_mode(VOICE_MODE_VELO, 0);
			voice_set_unit(0);  // always unit 0
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG1) {
			event_set_trig(0, EVENT_MAP_NOTE, channel, note);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG2) {
			event_set_trig(1, EVENT_MAP_NOTE, channel, note);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG3) {
			event_set_trig(2, EVENT_MAP_NOTE, channel, note);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG4) {
			event_set_trig(3, EVENT_MAP_NOTE, channel, note);
			setup_mode_cancel();
		}
	}

	//
	// PLAYING
	//
	// note CV/gate
	if(cv1_map == EVENT_MAP_NOTE && cv1_chan == channel) {
		voice_note_on(0, note, velocity);
	}
	if(cv2_map == EVENT_MAP_NOTE && cv2_chan == channel) {
		voice_note_on(1, note, velocity);
	}
	// note triggers
	if(trig1_map == EVENT_MAP_NOTE && trig1_chan == channel && trig1_val == note) {
		ioctl_set_trig1_out(TRIG_OUT_LEN);
		ioctl_set_trig1_led(TRIG_LED_LEN, 0);
	}
	if(trig2_map == EVENT_MAP_NOTE && trig2_chan == channel && trig2_val == note) {
		ioctl_set_trig2_out(TRIG_OUT_LEN); 
		ioctl_set_trig2_led(TRIG_LED_LEN, 0);
	}
	if(trig3_map == EVENT_MAP_NOTE && trig3_chan == channel && trig3_val == note) {
		ioctl_set_trig3_out(TRIG_OUT_LEN); 
		ioctl_set_trig3_led(TRIG_LED_LEN, 0);
	}
	if(trig4_map == EVENT_MAP_NOTE && trig4_chan == channel && trig4_val == note) {
		ioctl_set_trig4_out(TRIG_OUT_LEN); 
		ioctl_set_trig4_led(TRIG_LED_LEN, 0);
	}

	// echo and blink
	_midi_tx_note_on(channel, note, velocity);
	event_blink_in();
}

// control change
void _midi_rx_control_change(unsigned char channel,
		unsigned char controller,
		unsigned char value) {
	//
	// SETUP
	//
	// CC setup mode
	unsigned char temp = setup_get_mode();
	if(temp != SETUP_MODE_NONE) {
		if(channel == 15) return;  // channel 16 is reserved
		if(controller > 121) return;  // CC #122-127 cannot be assigned
		if(temp == SETUP_MODE_CV1) {
			event_set_cv(0, EVENT_MAP_CC, channel, controller);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_CV2) {
			event_set_cv(1, EVENT_MAP_CC, channel, controller);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG1) {
			event_set_trig(0, EVENT_MAP_CC, channel, controller);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG2) {
			event_set_trig(1, EVENT_MAP_CC, channel, controller);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG3) {
			event_set_trig(2, EVENT_MAP_CC, channel, controller);
			setup_mode_cancel();
		}
		else if(temp == SETUP_MODE_TRIG4) {
			event_set_trig(3, EVENT_MAP_CC, channel, controller);
			setup_mode_cancel();
		}
	}

	//
	// PLAYING
	//
	// CC in note mode - CV1
	if(cv1_map == EVENT_MAP_NOTE && cv1_chan == channel) {
		// legato retrig mode control
		if(controller == 20) {
			voice_set_legato_retrig(0, value >> 6);
		}
		// damper pedal
		else if(controller == 64) {
			voice_damper(0, value);
		}
	}
	// CC in note mode - CV2
	if(cv2_map == EVENT_MAP_NOTE && cv2_chan == channel) {
		// legato retrig mode control
		if(controller == 20) {
			voice_set_legato_retrig(1, value >> 6);
		}
		// damper pedal
		else if(controller == 64) {
			voice_damper(1, value);
		}
	}
	// CC CV/gate
	if(cv1_map == EVENT_MAP_CC && cv1_chan == channel && cv1_val == controller) {
		ioctl_set_cv1_out(4095 - (value << 5));
		ioctl_set_cv1_led(CV_LED_LEN, 0);
		if(value & 0x40) {
			ioctl_set_gate1_out(255);
			ioctl_set_gate1_led(255, 0);
		}
		else {
			ioctl_set_gate1_out(0);
			ioctl_set_gate1_led(0, 0);
		}
	}
	if(cv2_map == EVENT_MAP_CC && cv2_chan == channel && cv2_val == controller) {
		ioctl_set_cv2_out(4095 - (value << 5));
		ioctl_set_cv2_led(CV_LED_LEN, 0);
		if(value & 0x40) {
			ioctl_set_gate2_out(255);
			ioctl_set_gate2_led(255, 0);
		}
		else {
			ioctl_set_gate2_out(0);
			ioctl_set_gate2_led(0, 0);
		}
	}
	// CC trigger
	if(trig1_map == EVENT_MAP_CC && trig1_chan == channel && trig1_val == controller) {
		if(value & 0x40) {
			ioctl_set_trig1_out(255);
			ioctl_set_trig1_led(255, 0);
		}
		else {
			ioctl_set_trig1_out(0);
			ioctl_set_trig1_led(0, 0);
		}
	}	
	if(trig2_map == EVENT_MAP_CC && trig2_chan == channel && trig2_val == controller) {
		if(value & 0x40) {
			ioctl_set_trig2_out(255);
			ioctl_set_trig2_led(255, 0);
		}
		else {
			ioctl_set_trig2_out(0);
			ioctl_set_trig2_led(0, 0);
		}
	}	
	if(trig3_map == EVENT_MAP_CC && trig3_chan == channel && trig3_val == controller) {
		if(value & 0x40) {
			ioctl_set_trig3_out(255);
			ioctl_set_trig3_led(255, 0);
		}
		else {
			ioctl_set_trig3_out(0);
			ioctl_set_trig3_led(0, 0);
		}
	}	
	if(trig4_map == EVENT_MAP_CC && trig4_chan == channel && trig4_val == controller) {
		if(value & 0x40) {
			ioctl_set_trig4_out(255);
			ioctl_set_trig4_led(255, 0);
		}
		else {
			ioctl_set_trig4_out(0);
			ioctl_set_trig4_led(0, 0);
		}
	}	
	// CC channel 16 direct control mode
	if(channel == 15) {
		// make the value 8 bit
		temp = (value << 1);
		if(value) temp |= 0x01;

		// CV1 value - MSB
		if(controller == 16) {
			cv1_testh = value;
			ioctl_set_cv1_out((cv1_testh << 5) | (cv1_testl >> 2));
			ioctl_set_cv1_led(CV_LED_LEN, 0);
		}
		// CV1 value - LSB
		else if(controller == 48) {
			cv1_testl = value;
			ioctl_set_cv1_out((cv1_testh << 5) | (cv1_testl >> 2));
			ioctl_set_cv1_led(CV_LED_LEN, 0);
		}
		// CV2 value - MSB
		else if(controller == 17) {
			cv2_testh = value;
			ioctl_set_cv2_out((cv2_testh << 5) | (cv2_testl >> 2));
			ioctl_set_cv2_led(CV_LED_LEN, 0);
		}
		// CV2 value - LSB
		else if(controller == 49) {
			cv2_testl = value;
			ioctl_set_cv2_out((cv2_testh << 5) | (cv2_testl >> 2));
			ioctl_set_cv2_led(CV_LED_LEN, 0);
		}
		// gate 1
		else if(controller == 18) {
			ioctl_set_gate1_out(temp);
			ioctl_set_gate1_led(temp, 0);
		}
		// gate 2
		else if(controller == 19) {
			ioctl_set_gate2_out(temp);
			ioctl_set_gate2_led(temp, 0);
		}
		// trig 1
		else if(controller == 70) {
			ioctl_set_trig1_out(temp);
			ioctl_set_trig1_led(temp, 0);
		}
		// trig 2
		else if(controller == 71) {
			ioctl_set_trig2_out(temp);
			ioctl_set_trig2_led(temp, 0);
		}
		// trig 3
		else if(controller == 72) {
			ioctl_set_trig3_out(temp);
			ioctl_set_trig3_led(temp, 0);
		}
		// trig 4
		else if(controller == 73) {
			ioctl_set_trig4_out(temp);
			ioctl_set_trig4_led(temp, 0);
		}
		// clock
		else if(controller == 74) {
			ioctl_set_clock_out(temp);
			ioctl_set_clock_led(temp, 0);
		}
		// reset
		else if(controller == 75) {
			ioctl_set_reset_out(temp);
			ioctl_set_reset_led(temp, 0);			
		}
	}

	// echo and blink
	_midi_tx_control_change(channel, controller, value);
	event_blink_in();
}

// pitch bend
void _midi_rx_pitch_bend(unsigned char channel,
		unsigned int bend) {
	//
	// SETUP
	//
	// pitch bend setup mode
	unsigned char temp = setup_get_mode();
	if(temp != SETUP_MODE_NONE) {
		if(channel == 15) return;  // channel 16 is reserved
		if(temp == SETUP_MODE_CV1) {
			if(bend > PITCH_BEND_TRIG_UP) {
				event_set_cv(0, EVENT_MAP_PITCH_BEND, channel, 1);
				setup_mode_cancel();
			}
			else if(bend < PITCH_BEND_TRIG_DOWN) {
				event_set_cv(0, EVENT_MAP_PITCH_BEND, channel, 0);
				setup_mode_cancel();
			}
		}
		else if(temp == SETUP_MODE_CV2) {
			if(bend > PITCH_BEND_TRIG_UP) {
				event_set_cv(1, EVENT_MAP_PITCH_BEND, channel, 1);
				setup_mode_cancel();
			}
			else if(bend < PITCH_BEND_TRIG_DOWN) {
				event_set_cv(1, EVENT_MAP_PITCH_BEND, channel, 0);
				setup_mode_cancel();
			}
		}
		else if(temp == SETUP_MODE_TRIG1) {
			if(bend > PITCH_BEND_TRIG_UP) {
				event_set_trig(0, EVENT_MAP_PITCH_BEND, channel, 1);
				setup_mode_cancel();
			}
			else if(bend < PITCH_BEND_TRIG_DOWN) {
				event_set_trig(0, EVENT_MAP_PITCH_BEND, channel, 0);
				setup_mode_cancel();
			}
		}
		else if(temp == SETUP_MODE_TRIG2) {
			if(bend > PITCH_BEND_TRIG_UP) {
				event_set_trig(1, EVENT_MAP_PITCH_BEND, channel, 1);
				setup_mode_cancel();
			}
			else if(bend < PITCH_BEND_TRIG_DOWN) {
				event_set_trig(1, EVENT_MAP_PITCH_BEND, channel, 0);
				setup_mode_cancel();
			}
		}
		else if(temp == SETUP_MODE_TRIG3) {
			if(bend > PITCH_BEND_TRIG_UP) {
				event_set_trig(2, EVENT_MAP_PITCH_BEND, channel, 1);
				setup_mode_cancel();
			}
			else if(bend < PITCH_BEND_TRIG_DOWN) {
				event_set_trig(2, EVENT_MAP_PITCH_BEND, channel, 0);
				setup_mode_cancel();
			}
		}
		else if(temp == SETUP_MODE_TRIG4) {
			if(bend > PITCH_BEND_TRIG_UP) {
				event_set_trig(3, EVENT_MAP_PITCH_BEND, channel, 1);
				setup_mode_cancel();
			}
			else if(bend < PITCH_BEND_TRIG_DOWN) {
				event_set_trig(3, EVENT_MAP_PITCH_BEND, channel, 0);
				setup_mode_cancel();
			}
		}
	}

	//
	// PLAYING
	//
	// pitch bend - note bend mode
	if(cv1_map == EVENT_MAP_NOTE && cv1_chan == channel) {
		voice_pitch_bend(0, bend);
	}
	if(cv2_map == EVENT_MAP_NOTE && cv2_chan == channel) {
		voice_pitch_bend(1, bend);
	}
	// pitch bend CV/gate
	if(cv1_map == EVENT_MAP_PITCH_BEND && cv1_chan == channel) {
		// normal bend
		if(cv1_val == 1) {
			ioctl_set_cv1_out(4095 - (bend >> 2));
			ioctl_set_cv1_led(CV_LED_LEN, 0);
			if(bend > PITCH_BEND_TRIG_UP) {
				ioctl_set_gate1_out(255);
				ioctl_set_gate1_led(255, 255);
			}
			else {
				ioctl_set_gate1_out(0);
				ioctl_set_gate1_led(0, 0);
			}
		}
		// reverse bend
		else {
			ioctl_set_cv1_out(bend >> 2);
			ioctl_set_cv1_led(CV_LED_LEN, 0);
			if(bend > PITCH_BEND_TRIG_UP) {
				ioctl_set_gate2_out(255);
				ioctl_set_gate2_led(255, 255);
			}
			else {
				ioctl_set_gate2_out(0);
				ioctl_set_gate2_led(0, 0);
			}
		}
	}
	if(cv2_map == EVENT_MAP_PITCH_BEND && cv2_chan == channel) {
		// normal bend
		if(cv2_val == 1) {
			ioctl_set_cv2_out(4095 - (bend >> 2));
			ioctl_set_cv2_led(CV_LED_LEN, 0);
			if(bend > PITCH_BEND_TRIG_UP) {
				ioctl_set_gate2_out(255);
				ioctl_set_gate2_led(255, 255);
			}
			else {
				ioctl_set_gate2_out(0);
				ioctl_set_gate2_led(0, 0);
			}
		}
		// reverse bend
		else {
			ioctl_set_cv2_out(bend >> 2);
			ioctl_set_cv2_led(CV_LED_LEN, 0);
			if(bend < PITCH_BEND_TRIG_DOWN) {
				ioctl_set_gate2_out(255);
				ioctl_set_gate2_led(255, 255);
			}
			else {
				ioctl_set_gate2_out(0);
				ioctl_set_gate2_led(0, 0);
			}
		}
	}
	// pitch bend triggers
	if(trig1_map == EVENT_MAP_PITCH_BEND && trig1_chan == channel) {
		if(trig1_val == 1 && bend > PITCH_BEND_TRIG_UP ||
				trig1_val == 0 && bend < PITCH_BEND_TRIG_DOWN) {
			ioctl_set_trig1_out(255);
			ioctl_set_trig1_led(255, 255);
		}
		else {
			ioctl_set_trig1_out(0);
			ioctl_set_trig1_led(0, 0);
		}			
	}
	if(trig2_map == EVENT_MAP_PITCH_BEND && trig2_chan == channel) {
		if(trig2_val == 1 && bend > PITCH_BEND_TRIG_UP ||
				trig2_val == 0 && bend < PITCH_BEND_TRIG_DOWN) {
			ioctl_set_trig2_out(255);
			ioctl_set_trig2_led(255, 255);
		}
		else {
			ioctl_set_trig2_out(0);
			ioctl_set_trig2_led(0, 0);
		}			
	}
	if(trig3_map == EVENT_MAP_PITCH_BEND && trig3_chan == channel) {
		if(trig3_val == 1 && bend > PITCH_BEND_TRIG_UP ||
				trig3_val == 0 && bend < PITCH_BEND_TRIG_DOWN) {
			ioctl_set_trig3_out(255);
			ioctl_set_trig3_led(255, 255);
		}
		else {
			ioctl_set_trig3_out(0);
			ioctl_set_trig3_led(0, 0);
		}			
	}
	if(trig4_map == EVENT_MAP_PITCH_BEND && trig4_chan == channel) {
		if(trig4_val == 1 && bend > PITCH_BEND_TRIG_UP ||
				trig4_val == 0 && bend < PITCH_BEND_TRIG_DOWN) {
			ioctl_set_trig4_out(255);
			ioctl_set_trig4_led(255, 255);
		}
		else {
			ioctl_set_trig4_out(0);
			ioctl_set_trig4_led(0, 0);
		}			
	}

	// echo and blink
	_midi_tx_pitch_bend(channel, bend);
	event_blink_in();
}

// program change
void _midi_rx_program_change(unsigned char channel,
		unsigned char program) {
	// clock divide setting - program change 1-48 - only on channel 15
	if(program < 48 && channel == 15) {
		event_set_clock_div(program + 1);
	}
	// pitch bend CV1 - 0-11 = 1-12
	else if(program < 12) {
		voice_set_pitch_bend_range(0, program + 1);
	}
	// pitch bend CV2 - 12-23 = 1-12
	else if(program > 11 && program < 24) {
		voice_set_pitch_bend_range(1, program - 11);
	}

	// echo and blink
	_midi_tx_program_change(channel, program);
	event_blink_in();
}

//
// SYSTEM COMMON MESSAGES
//
// song position
void _midi_rx_song_position(unsigned int pos) {
	unsigned long ticks = (pos * 6);
	clock_count = ticks % clock_div;
	if(clock_count == 0) {
		start_arm = 1;
	}	
	// echo and blink
	_midi_tx_song_position(pos);
	event_blink_in();
}

//
// SYSEX MESSAGES
//
// sysex message start
void _midi_rx_sysex_start(void) {
	sysex_rx_start();
	// blink
	event_blink_in();
}

// sysex message data byte
void _midi_rx_sysex_data(unsigned char data_byte) {
	sysex_rx_data(data_byte);
	// blink
	event_blink_in();
}

// sysex message end
void _midi_rx_sysex_end(void) {
	sysex_rx_end();
	// blink
	event_blink_in();
}

//
// SYSTEM REALTIME MESSAGES
//
// timing tick
void _midi_rx_timing_tick(void) {
	if(clock_enabled) {
		clock_count ++;
		// divide down the clock or pulse if we are on a starting pulse
		if(clock_count >= clock_div || start_arm) {
			// pulse the output
			ioctl_set_clock_out(CLOCK_OUT_LEN);
			ioctl_set_clock_led(CLOCK_LED_LEN, 0);
			clock_count = 0;
			start_arm = 0;
		}
	}
	// echo and blink
	_midi_tx_timing_tick();
	event_blink_in();
}

// start song
void _midi_rx_start_song(void) {
	// reset pulses
	ioctl_set_reset_out(RESET_OUT_LEN);
	ioctl_set_reset_led(RESET_LED_LEN, 0);
	clock_enabled = 1;
	clock_count = 0;
	start_arm = 1;  // cause the next tick to make a pulse
	// echo and blink
	_midi_tx_start_song();
	event_blink_in();
}

// continue song
void _midi_rx_continue_song(void) {
	clock_enabled = 1;
	// echo and blink
	_midi_tx_continue_song();
	event_blink_in();
}

// stop song
void _midi_rx_stop_song(void) {
	clock_enabled = 0;
	// echo and blink
	_midi_tx_stop_song();
	event_blink_in();
}

// system reset
void _midi_rx_system_reset(void) {
	// this resets CV/gate outputs
	voice_state_reset();
	// reset all trigger and clock outputs
	ioctl_set_trig1_out(0);
	ioctl_set_trig2_out(0);
	ioctl_set_trig3_out(0);
	ioctl_set_trig4_out(0);
	ioctl_set_clock_out(0);
	ioctl_set_reset_out(0);
	// echo system reset
	_midi_tx_system_reset();
	event_blink_in();
}

//
// UNSUPPORTED CALLBACKS
//
// key pressure
void _midi_rx_key_pressure(unsigned char channel, 
		unsigned char note,
		unsigned char pressure) {
	// key pressure is not supported
	// echo and blink
	_midi_tx_key_pressure(channel, note, pressure);
	event_blink_in();
}

// channel pressure
void _midi_rx_channel_pressure(unsigned char channel,
		unsigned char pressure) {
	// channel pressure is not supported
	// echo and blink
	_midi_tx_channel_pressure(channel, pressure);
	event_blink_in();
}

// song select
void _midi_rx_song_select(unsigned char song) {
	// song select not supported
	// echo and blink
	_midi_tx_song_select(song);
	event_blink_in();
}

// active sensing
void _midi_rx_active_sensing(void) {
	// active sensing not supported
}

// learn the MIDI channel
void _midi_learn_channel(unsigned char channel) {
	// we don't use this way of doing it
}

//
// CONFIG
//
// set a CV config
void event_set_cv(unsigned char num, unsigned char map, 
		unsigned char chan, unsigned char val) {
	if(num > 1) return;
	if(num == 1) {
		cv2_map = map & 0x03;
		cv2_chan = chan & 0x0f;
		if(cv2_chan == 0x0f) cv2_chan = 0x00;
		cv2_val = val & 0x7f;
		config_store_set_val(CONFIG_CV2_MAP, cv2_map);
		config_store_set_val(CONFIG_CV2_CHAN, cv2_chan);
		config_store_set_val(CONFIG_CV2_VAL, cv2_val);
	}
	else {
		cv1_map = map & 0x03;
		cv1_chan = chan & 0x0f;
		if(cv1_chan == 0x0f) cv1_chan = 0x00;
		cv1_val = val & 0x7f;
		config_store_set_val(CONFIG_CV1_MAP, cv1_map);
		config_store_set_val(CONFIG_CV1_CHAN, cv1_chan);
		config_store_set_val(CONFIG_CV1_VAL, cv1_val);
	}
}

// set a TRIGGER config
void event_set_trig(unsigned char num, unsigned char map,
		unsigned char chan, unsigned char val) {
	if(num > 3) return;
	if(num == 3) {
		trig4_map = map & 0x03;
		trig4_chan = chan & 0x0f;
		if(trig4_chan == 0x0f) trig4_chan = 0x00;
		trig4_val = val & 0x7f;
		config_store_set_val(CONFIG_TRIG4_MAP, trig4_map);
		config_store_set_val(CONFIG_TRIG4_CHAN, trig4_chan);
		config_store_set_val(CONFIG_TRIG4_VAL, trig4_val);
	}
	else if(num == 2) {
		trig3_map = map & 0x03;
		trig3_chan = chan & 0x0f;
		if(trig3_chan == 0x0f) trig3_chan = 0x00;
		trig3_val = val & 0x7f;
		config_store_set_val(CONFIG_TRIG3_MAP, trig3_map);
		config_store_set_val(CONFIG_TRIG3_CHAN, trig3_chan);
		config_store_set_val(CONFIG_TRIG3_VAL, trig3_val);
	}
	else if(num == 1) {
		trig2_map = map & 0x03;
		trig2_chan = chan & 0x0f;
		if(trig2_chan == 0x0f) trig2_chan = 0x00;
		trig2_val = val & 0x7f;
		config_store_set_val(CONFIG_TRIG2_MAP, trig2_map);
		config_store_set_val(CONFIG_TRIG2_CHAN, trig2_chan);
		config_store_set_val(CONFIG_TRIG2_VAL, trig2_val);
	}
	else {
		trig1_map = map & 0x03;
		trig1_chan = chan & 0x0f;
		if(trig1_chan == 0x0f) trig1_chan = 0x00;
		trig1_val = val & 0x7f;
		config_store_set_val(CONFIG_TRIG1_MAP, trig1_map);
		config_store_set_val(CONFIG_TRIG1_CHAN, trig1_chan);
		config_store_set_val(CONFIG_TRIG1_VAL, trig1_val);
	}
}

// set the clock div
void event_set_clock_div(unsigned char div) {
	clock_div = div;
	if(clock_div > 48) clock_div = 48;
	config_store_set_val(CONFIG_CLOCK_DIV, clock_div);
}
