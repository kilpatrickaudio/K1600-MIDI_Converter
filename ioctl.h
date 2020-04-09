/*
 * K1600 MIDI Converter - IO Control Routines
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.0
 *
 */
// 1.000V - test mode
#define CV_TEST_VAL 1632
// 0.000V - zero val
#define CV_ZERO_VAL 2040

// init the stuff
void ioctl_init(void);

// runs the task on a timer
void ioctl_timer_task(void);

// set the CV1 output value
void ioctl_set_cv1_out(unsigned int);

// set the CV2 output value
void ioctl_set_cv2_out(unsigned int);

// set the CV1 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_cv1_led(unsigned char, unsigned char);

// set the CV2 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_cv2_led(unsigned char, unsigned char);

// set the GATE1 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_gate1_led(unsigned char, unsigned char);

// set the GATE2 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_gate2_led(unsigned char, unsigned char);

// set the TRIG1 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_trig1_led(unsigned char, unsigned char);

// set the TRIG2 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_trig2_led(unsigned char, unsigned char);

// set the TRIG3 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_trig3_led(unsigned char, unsigned char);

// set the TRIG4 LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_trig4_led(unsigned char, unsigned char);

// set the reset LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_reset_led(unsigned char, unsigned char);

// set the clock LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_clock_led(unsigned char, unsigned char);

// set the MIDI in LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_midi_in_led(unsigned char, unsigned char);

// set the MIDI out LED - on time, off time (for repeat or 0 for one-shot)
void ioctl_set_midi_out_led(unsigned char, unsigned char);

// set the GATE1 out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_gate1_out(unsigned char);

// set the GATE2 out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_gate2_out(unsigned char);

// set the TRIG1 out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_trig1_out(unsigned char);

// set the TRIG2 out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_trig2_out(unsigned char);

// set the TRIG3 out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_trig3_out(unsigned char);

// set the TRIG4 out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_trig4_out(unsigned char);

// set the reset out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_reset_out(unsigned char);

// set the clock out - 0 = off, 1-254 = 1-254 * 1024us, 255 = latch on
void ioctl_set_clock_out(unsigned char);

// gets the state of the setup switch
unsigned char ioctl_get_setup_sw(void);

// gets the state of the select switch
unsigned char ioctl_get_select_sw(void);

// sets the state of the TEST pin - 1 = assert test mode, 0 = high Z
void ioctl_set_test_pin(unsigned char mode);
