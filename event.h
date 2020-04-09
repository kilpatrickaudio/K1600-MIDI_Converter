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
// init the event mapper
void event_init(void);

// set a CV config
void event_set_cv(unsigned char num, unsigned char map, unsigned char chan, unsigned char val);

// set a trigger config
void event_set_trig(unsigned char num, unsigned char map, unsigned char chan, unsigned char val);

// set the clock div
void event_set_clock_div(unsigned char div);
