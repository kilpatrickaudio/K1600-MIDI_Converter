/*
 * K1600 MIDI Converter - Config Storage
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.0
 *
 */
#define CONFIG_MAX 32
#define CONFIG_CV1_MAP 0x00
#define CONFIG_CV2_MAP 0x01
#define CONFIG_CV1_CHAN 0x02
#define CONFIG_CV2_CHAN 0x03
#define CONFIG_CV1_VAL 0x04
#define CONFIG_CV2_VAL 0x05
#define CONFIG_TRIG1_MAP 0x06
#define CONFIG_TRIG2_MAP 0x07
#define CONFIG_TRIG3_MAP 0x08
#define CONFIG_TRIG4_MAP 0x09
#define CONFIG_TRIG1_CHAN 0x0a
#define CONFIG_TRIG2_CHAN 0x0b
#define CONFIG_TRIG3_CHAN 0x0c
#define CONFIG_TRIG4_CHAN 0x0d
#define CONFIG_TRIG1_VAL 0x0e
#define CONFIG_TRIG2_VAL 0x0f
#define CONFIG_TRIG3_VAL 0x10
#define CONFIG_TRIG4_VAL 0x11
#define CONFIG_CLOCK_DIV 0x12
#define CONFIG_VOICE_MODE 0x13
#define CONFIG_VOICE_SPLIT 0x14
#define CONFIG_VOICE_UNIT 0x15
#define CONFIG_VOICE_BEND1 0x16
#define CONFIG_VOICE_BEND2 0x17
#define CONFIG_VOICE_LEGATO_RETRIG1 0x18
#define CONFIG_VOICE_LEGATO_RETRIG2 0x19
#define CONFIG_SETUP_TOKEN 0x1f

// init the config store
void config_store_init(void);

// run the config storage task
void config_store_timer_task(void);

// gets a config byte
unsigned char config_store_get_val(unsigned char addr);

// sets a config byte
void config_store_set_val(unsigned char addr, unsigned char val);
