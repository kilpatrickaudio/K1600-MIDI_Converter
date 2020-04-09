/*
* MIDI RX callbacks to be implemented by RX handlers
*
* Copyright 2009: Kilpatrick Audio
* Written by: Andrew Kilpatrick
*
*/
//
// SETUP MESSAGES
//
// learn the MIDI channel
void _midi_learn_channel(unsigned char channel);

//
// CHANNEL MESSAGES
//
// note off - note on with velocity = 0 calls this
void _midi_rx_note_off(unsigned char channel, 
	unsigned char note);

// note on - note on with velocity > 0 calls this
void _midi_rx_note_on(unsigned char channel, 
	unsigned char note, 
	unsigned char velocity);

// key pressure
void _midi_rx_key_pressure(unsigned char channel, 
	unsigned char note,
	unsigned char pressure);

// control change
void _midi_rx_control_change(unsigned char channel,
	unsigned char controller,
	unsigned char value);

// program change
void _midi_rx_program_change(unsigned char channel,
	unsigned char program);

// channel pressure
void _midi_rx_channel_pressure(unsigned char channel,
	unsigned char pressure);

// pitch bend
void _midi_rx_pitch_bend(unsigned char channel,
	unsigned int bend);

//
// SYSTEM COMMON MESSAGES
//
// song position
void _midi_rx_song_position(unsigned int pos);

// song select
void _midi_rx_song_select(unsigned char song);

//
// SYSEX MESSAGES
//
// sysex message start
void _midi_rx_sysex_start(void);

// sysex message data byte
void _midi_rx_sysex_data(unsigned char data_byte);

// sysex message end
void _midi_rx_sysex_end(void);

//
// SYSTEM REALTIME MESSAGES
//
// timing tick
void _midi_rx_timing_tick(void);

// start song
void _midi_rx_start_song(void);

// continue song
void _midi_rx_continue_song(void);

// stop song
void _midi_rx_stop_song(void);

// active sensing
void _midi_rx_active_sensing(void);

// system reset
void _midi_rx_system_reset(void);

