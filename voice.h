/*
 * K1600 MIDI Converter - Voice Manager Code
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.1
 *
 * Ver. 1.1 changes:
 *  - added arp mode
 *
 */
// CV modes
#define VOICE_MODE_SINGLE 0
#define VOICE_MODE_SPLIT 1
#define VOICE_MODE_POLY 2
#define VOICE_MODE_ARP 3
#define VOICE_MODE_VELO 4

// init the voice manager
void voice_init(void);

// runs the timer task
void voice_timer_task(void);

// set up the voice mode
void voice_set_mode(unsigned char mode, unsigned char split);

// set up the voice unit
void voice_set_unit(unsigned char unit);

// set the pitch bend range for a voice
void voice_set_pitch_bend_range(unsigned char voice, unsigned char bend);

// set the legato retrig mode on/off
void voice_set_legato_retrig(unsigned char voice, unsigned char state);

// note on
void voice_note_on(unsigned char voice, unsigned char note, unsigned char velocity);

// note off
void voice_note_off(unsigned char voice, unsigned char note);

// damper pedal
void voice_damper(unsigned char voice, unsigned char state);

// pitch bend
void voice_pitch_bend(unsigned char voice, unsigned int bend);

// reset voice outputs and state
void voice_state_reset(void);
