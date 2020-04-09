/*
 * MIDI Receiver/Parser/Transmitter Code
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
//#include <plib.h>  // MCC18
#include <system.h>  // BoostC
#include "midi.h"
#include "midi_callbacks.h"

// status bytes
#define MIDI_CHAN_PRESSURE 0xd0
#define MIDI_CONTROL_CHANGE 0xb0
#define MIDI_KEY_PRESSURE 0xa0
#define MIDI_NOTE_OFF 0x80
#define MIDI_NOTE_ON 0x90
#define MIDI_PITCH_BEND 0xe0
#define MIDI_PROG_CHANGE 0xc0

// system common messages
#define MIDI_SONG_POSITION 0xf2
#define MIDI_SONG_SELECT 0xf3

// sysex exclusive messages
#define MIDI_SYSEX_START 0xf0
#define MIDI_SYSEX_END 0xf7

// system realtime messages
#define MIDI_TIMING_TICK 0xf8
#define MIDI_START_SONG 0xfa
#define MIDI_CONTINUE_SONG 0xfb
#define MIDI_STOP_SONG 0xfc
#define MIDI_ACTIVE_SENSING 0xfe
#define MIDI_SYSTEM_RESET 0xff

// state
#define RX_STATE_IDLE 0
#define RX_STATE_DATA0 1
#define RX_STATE_DATA1 2
#define RX_STATE_SYSEX_DATA 3
unsigned char rx_state;  // receiver state
unsigned char midi_learn_mode;

// RX message
unsigned char rx_status_chan;  // current message channel
unsigned char rx_status;  // current status byte
unsigned char rx_data0;  // data0 byte
unsigned char rx_data1;  // data1 byte

// RX buffer
unsigned rx_msg[64];  // receive msg buffer
unsigned rx_in_pos;
unsigned rx_out_pos;

// TX message
unsigned char tx_msg[256];  // transmit msg buffer
unsigned char tx_in_pos;
unsigned char tx_out_pos;
#define TX_IN_POS tx_in_pos++

// function prototypes
void process_msg(void);

// init the MIDI receiver module
void midi_init(void) {
	rx_state = RX_STATE_IDLE;
	rx_status = 0;  // no running status yet
 	rx_status_chan = 0;
	tx_in_pos = 0;
	tx_out_pos = 0;
	rx_in_pos = 0;
	rx_out_pos = 0;
	midi_learn_mode = 0;
}

// handle a new byte received from the stream
void midi_rx_byte(unsigned char rx_byte) {
	rx_in_pos = (rx_in_pos + 1) & 0x3f;
	rx_msg[rx_in_pos] = rx_byte;
}

// transmit task
void midi_tx_task(void) {
	if(!txsta.TRMT) return;  // BoostC
	if(tx_in_pos == tx_out_pos) return;
//	UARTSendDataByte(UART1, tx_msg[tx_out_pos++]);  // MCC18
	txreg = tx_msg[tx_out_pos++];  // BoostC
}

// receive task
void midi_rx_task(void) {
	unsigned char stat, chan;
	unsigned char rx_byte;
	// turn off interrupts
	intcon.GIE = 0;
	// get data from RX buffer
	if(rx_in_pos == rx_out_pos) {
		intcon.GIE = 1;
		return;
	}
	else {
		rx_out_pos = (rx_out_pos + 1) & 0x3f;
		rx_byte = rx_msg[rx_out_pos];
	}
	// turn on interrupts
	intcon.GIE = 1;
	// status byte
	if(rx_byte & 0x80) {
		stat = (rx_byte & 0xf0);
	   	chan = (rx_byte & 0x0f);

		// system messages
   		if(stat == 0xf0) {
			// realtime messages
	     	if(rx_byte == MIDI_TIMING_TICK) {
				_midi_rx_timing_tick();
				return;
    	 	}
	     	if(rx_byte == MIDI_START_SONG) {
				_midi_rx_start_song();
				return;
    		}	
     		if(rx_byte == MIDI_CONTINUE_SONG) {
				_midi_rx_continue_song();
				return;
     		}
	     	if(rx_byte == MIDI_STOP_SONG) {
				_midi_rx_stop_song();
				return;
	     	}
			if(rx_byte == MIDI_ACTIVE_SENSING) {
				_midi_rx_active_sensing();
				return;
			}
			if(rx_byte == MIDI_SYSTEM_RESET) {
				_midi_rx_system_reset();
				return;
			}
			// system common messages
    	 	if(rx_byte == MIDI_SONG_POSITION) {
				rx_status_chan = 255;  // reset running status channel
				rx_status = rx_byte;
				rx_state = RX_STATE_DATA0;
				return;
    	 	}
	     	if(rx_byte == MIDI_SONG_SELECT) {
				rx_status_chan = 255;  // reset running status channel
				rx_status = rx_byte;
				rx_state = RX_STATE_DATA0;
				return;
    	 	}
			// sysex messages
    		if(rx_byte == MIDI_SYSEX_START) {
				_midi_rx_sysex_start();
				rx_status_chan = 255;  // reset running status channel
				rx_status = rx_byte;
				rx_state = RX_STATE_SYSEX_DATA;
				return;
	     	}
	     	if(rx_byte == MIDI_SYSEX_END) {
				_midi_rx_sysex_end();
				rx_status_chan = 255;  // reset running status channel
				rx_status = 0;
				rx_state = RX_STATE_IDLE;
				return;
     		}
   		}
	   	// channel messages
   		else {
     		if(stat == MIDI_NOTE_OFF) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
	     	}
    	 	if(stat == MIDI_NOTE_ON) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
     		}
	     	if(stat == MIDI_KEY_PRESSURE) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
    	 	}
	    	if(stat == MIDI_CONTROL_CHANGE) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
	     	}
    	 	if(stat == MIDI_PROG_CHANGE) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
	     	}
    	 	if(stat == MIDI_CHAN_PRESSURE) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
	     	}
    	 	if(stat == MIDI_PITCH_BEND) {
				rx_status_chan = chan;
				rx_status = stat;
				rx_state = RX_STATE_DATA0;
				return;
	    	}
   		}
   		return;  // just in case
	}
 	// data byte 0
 	if(rx_state == RX_STATE_DATA0) {
   		rx_data0 = rx_byte;

		// data length = 1 - process these messages right away
   		if(rx_status == MIDI_SONG_SELECT ||
				rx_status == MIDI_PROG_CHANGE ||
      			rx_status == MIDI_CHAN_PRESSURE) {
     		process_msg();
			// if this message supports running status
     		if(rx_status_chan != 255) {
				rx_state = RX_STATE_DATA0;  // loop back for running status
			}
			else {
				rx_state = RX_STATE_IDLE;
			}
   		}
   		// data length = 2
   		else {
     		rx_state = RX_STATE_DATA1;
   		}
	   return;
 	}

 	// data byte 1
 	if(rx_state == RX_STATE_DATA1) {
   		rx_data1 = rx_byte;
   		process_msg();
		// if this message supports running status
   		if(rx_status_chan != 255) {   		
   			rx_state = RX_STATE_DATA0;  // loop back for running status
		}
		else {
			rx_state = RX_STATE_IDLE;
		}
   		return;
 	}

	// sysex data
	if(rx_state == RX_STATE_SYSEX_DATA) {
		_midi_rx_sysex_data(rx_byte);
		return;
	}
}

// process a received message
void process_msg(void) {
	if(rx_status == MIDI_SONG_POSITION) {
   		_midi_rx_song_position((rx_data1 >> 7) | rx_data0);
   		return;
 	}
 	if(rx_status == MIDI_SONG_SELECT) {
   		_midi_rx_song_select(rx_data0);
   		return;
 	}
	// this is a channel message by this point
	if(midi_learn_mode) {
		_midi_learn_channel(rx_status_chan);
		midi_set_learn_mode(0);  // turn this off
	}
 	if(rx_status == MIDI_NOTE_OFF) {
   		_midi_rx_note_off(rx_status_chan, rx_data0);
   		return;
 	}
 	if(rx_status == MIDI_NOTE_ON) {
   		if(rx_data1 == 0) _midi_rx_note_off(rx_status_chan, rx_data0);
   		else _midi_rx_note_on(rx_status_chan, rx_data0, rx_data1);
   		return;
 	}
 	if(rx_status == MIDI_KEY_PRESSURE) {
   		_midi_rx_key_pressure(rx_status_chan, rx_data0, rx_data1);
   		return;
 	}
 	if(rx_status == MIDI_CONTROL_CHANGE) {
   		_midi_rx_control_change(rx_status_chan, rx_data0, rx_data1);
   		return;
 	}
 	if(rx_status == MIDI_PROG_CHANGE) {
   		_midi_rx_program_change(rx_status_chan, rx_data0);
   		return;
 	}
 	if(rx_status == MIDI_CHAN_PRESSURE) {
   		_midi_rx_channel_pressure(rx_status_chan, rx_data0);
   		return;
 	}
 	if(rx_status == MIDI_PITCH_BEND) {
   		_midi_rx_pitch_bend(rx_status_chan, 
			(unsigned int) (((unsigned int) rx_data1 << 7) | 
			(unsigned int) rx_data0));
   		return;
 	}
}

// sets the learn mode - 1 = on, 0 = off
void midi_set_learn_mode(unsigned char mode) {
	midi_learn_mode = (mode & 0x01);
}

//
// SENDERS
//
// send note off - sends note on with velocity 0
void _midi_tx_note_off(unsigned char channel,
		unsigned char note) {  
 	tx_msg[TX_IN_POS] = MIDI_NOTE_ON | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (note & 0x7f);
 	tx_msg[TX_IN_POS] = 0x00;
}

// send note on
void _midi_tx_note_on(unsigned char channel,
		unsigned char note,
		unsigned char velocity) {
	tx_msg[TX_IN_POS] = MIDI_NOTE_ON | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (note & 0x7f);
 	tx_msg[TX_IN_POS] = (velocity & 0x7f);	
}

// key pressure
void _midi_tx_key_pressure(unsigned char channel,
			   unsigned char note,
			   unsigned char pressure) {
	tx_msg[TX_IN_POS] = MIDI_KEY_PRESSURE | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (note & 0x7f);
 	tx_msg[TX_IN_POS] = (pressure & 0x7f);
}

// control change
void _midi_tx_control_change(unsigned char channel,
		unsigned char controller,
		unsigned char value) {
 	tx_msg[TX_IN_POS] = MIDI_CONTROL_CHANGE | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (controller & 0x7f);
 	tx_msg[TX_IN_POS] = (value & 0x7f);
}

// program change
void _midi_tx_program_change(unsigned char channel,
		unsigned char program) {
 	tx_msg[TX_IN_POS] = MIDI_PROG_CHANGE | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (program & 0x7f);
}

// channel pressure
void _midi_tx_channel_pressure(unsigned char channel,
			       unsigned char pressure) {
 	tx_msg[TX_IN_POS] = MIDI_CHAN_PRESSURE | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (pressure & 0x7f);
}

// pitch bend
void _midi_tx_pitch_bend(unsigned char channel,
		unsigned int bend) {
 	tx_msg[TX_IN_POS] = MIDI_PITCH_BEND | (channel & 0x0f);
 	tx_msg[TX_IN_POS] = (bend & 0x7f);
 	tx_msg[TX_IN_POS] = (bend & 0x3f80) >> 7;
}

// sysex message start
void _midi_tx_sysex_start(void) {
	tx_msg[TX_IN_POS] = MIDI_SYSEX_START;
}

// sysex message data byte
void _midi_tx_sysex_data(unsigned char data_byte) {
 	tx_msg[TX_IN_POS] = data_byte;
}

// sysex message end
void _midi_tx_sysex_end(void) {
	tx_msg[TX_IN_POS] = MIDI_SYSEX_END;
}

// song position
void _midi_tx_song_position(unsigned int pos) {
	tx_msg[TX_IN_POS] = MIDI_SONG_POSITION;
 	tx_msg[TX_IN_POS] = (pos & 0x7f);
 	tx_msg[TX_IN_POS] = (pos & 0x3f8) >> 7;
}

// song select
void _midi_tx_song_select(unsigned char song) {
	tx_msg[TX_IN_POS] = MIDI_SONG_SELECT;
	tx_msg[TX_IN_POS] = (song & 0x7f);
}

// timing tick
void _midi_tx_timing_tick(void) {
	tx_msg[TX_IN_POS] = MIDI_TIMING_TICK;
}

// start song
void _midi_tx_start_song(void) {
 	tx_msg[TX_IN_POS] = MIDI_START_SONG;
}

// continue song
void _midi_tx_continue_song(void) {
 	tx_msg[TX_IN_POS] = MIDI_CONTINUE_SONG;
}

// stop song
void _midi_tx_stop_song(void) {
 	tx_msg[TX_IN_POS] = MIDI_STOP_SONG;
}

// system reset
void _midi_tx_system_reset(void) {
 	tx_msg[TX_IN_POS] = MIDI_SYSTEM_RESET;
}
