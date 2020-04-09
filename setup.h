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
// setup modes
#define MAX_SETUP_MODES 12
#define SETUP_MODE_NONE 0
#define SETUP_MODE_CV1 1
#define SETUP_MODE_CV2 2
#define SETUP_MODE_CV_SPLIT 3
#define SETUP_MODE_CV_POLY 4
#define SETUP_MODE_CV_ARP 5
#define SETUP_MODE_CV_VELO 6
#define SETUP_MODE_TRIG1 7
#define SETUP_MODE_TRIG2 8
#define SETUP_MODE_TRIG3 9
#define SETUP_MODE_TRIG4 10
#define SETUP_MODE_INTERNAL 11

// init the setup control
void setup_init(void);

// run the setup timer task
void setup_timer_task(void);

// gets the current setup mode
unsigned char setup_get_mode(void);

// cancel setup mode
void setup_mode_cancel(void);