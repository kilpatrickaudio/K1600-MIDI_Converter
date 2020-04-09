/*
 * MIDI Receiver/Parser/Transmitter Code
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
void midi_init(void);
void midi_rx_byte(unsigned char rx_byte);
void midi_tx_task(void);
void midi_rx_task(void);
void midi_set_learn_mode(unsigned char mode);

// senders
void _midi_tx_note_on(unsigned char channel,
		      unsigned char note,
		      unsigned char velocity);
			  

void _midi_tx_note_off(unsigned char channel,
		       unsigned char note);
			   
void _midi_tx_pitch_bend(unsigned char channel,
			 unsigned int bend);
			 
void _midi_tx_key_pressure(unsigned char channel,
			   unsigned char note,
			   unsigned char pressure);

void _midi_tx_control_change(unsigned char channel,
		unsigned char controller,
		unsigned char value);

void _midi_tx_program_change(unsigned char channel,
		unsigned char program);

void _midi_tx_channel_pressure(unsigned char channel,
			       unsigned char pressure);

void _midi_tx_pitch_bend(unsigned char channel,
		unsigned int bend);

void _midi_tx_sysex_start(void);

void _midi_tx_sysex_data(unsigned char data_byte);

void _midi_tx_sysex_end(void);

void _midi_tx_song_position(unsigned int pos);

void _midi_tx_song_select(unsigned char song);

void _midi_tx_timing_tick(void);

void _midi_tx_start_song(void);

void _midi_tx_continue_song(void);

void _midi_tx_stop_song(void);

void _midi_tx_system_reset(void);
