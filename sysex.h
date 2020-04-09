/*
 * K1600 MIDI Converter - SYSEX Handler
 *
 * Copyright 2010: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 * Version: 1.0
 *
 */
#define SYSEX_CMD_SYSTEM_CONFIG 0x02
#define SYSEX_CMD_EEPROM_READ 0x70
#define SYSEX_CMD_EEPROM_WRITE 0x71

// init the sysex code
void sysex_init(void) ;

// handle start of SYSEX packet
void sysex_rx_start(void);

// handle SYSEX data byte
void sysex_rx_data(unsigned char data_byte);

// handle end of SYSEX packet
void sysex_rx_end(void);

// send a SYSEX packet with CMD and DATA
void sysex_tx_msg(unsigned char cmd, unsigned char data);

// send a SYSEX packet with CMD and 2 DATA bytes
void sysex_tx_msg2(unsigned char cmd, unsigned char data0, unsigned char data1);
