/*
 * K1600 MIDI Converter - SYSEX Handler
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.0
 *
 */
#include <system.h>
#include "sysex.h"
#include "midi.h"
#include "voice.h"
#include "event.h"

#define SYSEX_TX_MAX_LEN 64
unsigned char sysex_tx_buf[SYSEX_TX_MAX_LEN];
unsigned char sysex_tx_len;

#define SYSEX_RX_MAX_LEN 64
unsigned char sysex_rx_buf[SYSEX_RX_MAX_LEN];
unsigned char sysex_rx_len;

// local functions
void sysex_parse_system_config(void);

// init the sysex code
void sysex_init(void) {
	sysex_tx_len = 0;
	sysex_rx_len = 0;
}

// handle start of SYSEX packet
void sysex_rx_start(void) {
	sysex_rx_len = 0;
}

// handle SYSEX data byte
void sysex_rx_data(unsigned char data_byte) {
	// trying to send too many data bytes
	if(sysex_rx_len >= SYSEX_RX_MAX_LEN) {
		sysex_rx_len = 0;
	}
	sysex_rx_buf[sysex_rx_len] = data_byte;
	sysex_rx_len ++;
}

// handle end of SYSEX packet
void sysex_rx_end(void) {
	unsigned char i;
	unsigned char echo_msg = 1;  // default

	// invalid message
	if(sysex_rx_len == 0) return;

	// can we parse it?
	if(sysex_rx_len >= 5) {
		// is this a Kilpatrick Audio K1600 message?
		if(sysex_rx_buf[0] == 0x00 && 
				sysex_rx_buf[1] == 0x01 &&
				sysex_rx_buf[2] == 0x72 &&
				sysex_rx_buf[3] == 0x40) {
			// set system configuration
			if(sysex_rx_buf[4] == SYSEX_CMD_SYSTEM_CONFIG && sysex_rx_len == 29) {
				sysex_parse_system_config();
			}
		}
	}

	// if we're allowed to echo this message
	if(echo_msg) {
		_midi_tx_sysex_start();
		for(i = 0; i < sysex_rx_len; i ++) {
			_midi_tx_sysex_data(sysex_rx_buf[i]);
		}
		_midi_tx_sysex_end();
	}
}

// send a SYSEX packet with CMD and DATA
void sysex_tx_msg(unsigned char cmd, unsigned char data) {
	_midi_tx_sysex_start();
	_midi_tx_sysex_data(0x00);
	_midi_tx_sysex_data(0x01);
	_midi_tx_sysex_data(0x72);
	_midi_tx_sysex_data(0x40);
	_midi_tx_sysex_data(cmd & 0x7f);
	_midi_tx_sysex_data(data & 0x7f);
	_midi_tx_sysex_end();
}

// send a SYSEX packet with CMD and 2 DATA bytes
void sysex_tx_msg2(unsigned char cmd, unsigned char data0, unsigned char data1) {
	_midi_tx_sysex_start();
	_midi_tx_sysex_data(0x00);
	_midi_tx_sysex_data(0x01);
	_midi_tx_sysex_data(0x72);
	_midi_tx_sysex_data(0x40);
	_midi_tx_sysex_data(cmd & 0x7f);
	_midi_tx_sysex_data(data0 & 0x7f);
	_midi_tx_sysex_data(data1 & 0x7f);
	_midi_tx_sysex_end();
}

//
// PRIVATE FUNCTIONS
//
void sysex_parse_system_config(void) {
	// CV 1
	event_set_cv(0, 
		sysex_rx_buf[5 + 0x00], 
		sysex_rx_buf[5 + 0x02],
		sysex_rx_buf[5 + 0x04]);
	// CV2
	event_set_cv(1,
		sysex_rx_buf[5 + 0x01],
		sysex_rx_buf[5 + 0x03],
		sysex_rx_buf[5 + 0x05]);
	// trig 1
	event_set_trig(0,
		sysex_rx_buf[5 + 0x06],
		sysex_rx_buf[5 + 0x0a],
		sysex_rx_buf[5 + 0x0e]);
	// trig 2
	event_set_trig(1,
		sysex_rx_buf[5 + 0x07],
		sysex_rx_buf[5 + 0x0b],
		sysex_rx_buf[5 + 0x0f]);
	// trig 3
	event_set_trig(2,
		sysex_rx_buf[5 + 0x08],
		sysex_rx_buf[5 + 0x0c],
		sysex_rx_buf[5 + 0x10]);
	// trig 4
	event_set_trig(3,
		sysex_rx_buf[5 + 0x09],
		sysex_rx_buf[5 + 0x0d],
		sysex_rx_buf[5 + 0x11]);
	// clock div
	event_set_clock_div(sysex_rx_buf[5 + 0x12]);
	// voice mode
	voice_set_mode(sysex_rx_buf[5 + 0x13], sysex_rx_buf[5 + 0x14]);
	// voice unit
	voice_set_unit(sysex_rx_buf[5 + 0x15]);
	// pitch bend range
	voice_set_pitch_bend_range(0, sysex_rx_buf[5 + 0x16]);
	voice_set_pitch_bend_range(1, sysex_rx_buf[5 + 0x17]);
}
