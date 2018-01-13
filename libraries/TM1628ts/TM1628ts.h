/*
	TM1628 7-Segment Display Control Library
	designed for TeleSystem TS5.9RX DVD Recorder front panel
	which uses different LED mappings from other DVD players

	Some parts of code are heavily inspired from the TM1628 library
	developed by Vasyl Yudin [https://github.com/BlockThor/TM1628]

	Copyright (C) 2017 One Transistor [https://onetransistor.github.io]

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TM1628ts_H
#define TM1628ts_H

#include "Arduino.h"

#define   POWER_LED   		0 

#define   HEATER_LED    	1 
#define   NO_WATER_LED   	2 
#define   ONE_DIGIT_LED   	3 

#define   SETUP_LED    		4 
#define   GRADUS_LED	    5 
#define   RH_LED		    6 
#define   ION_LED		    7 

#define   LEVEL_LED1      	8 
#define   LEVEL_LED2		9 

#define   LEVEL_LED3  		10 
#define   LEVEL_LED4    	11 

#define   LEVEL_LED5		12 
#define   LEVEL_LED6		13
#define   LEVEL_LED7		14

#define   LEVEL_LED8		15
#define   LEVEL_LED9		16

#define	  DVOETOCHIE_LED	17



class TM1628ts
{

// Segments assignment
// RAM location is at Segment number * Position
// Position is 1 to 7 from the right
//
// Segment numbers:
//
//        __2_(bit 2)_
//       /     \
//    8 /      / 0 (bit 2)
//      \_____/
//      /  6 \
//   10 /      / 14
//     \_____/
//       12
//
//  0   2   4   6   8  10  12  14
// ___ ___ ___ ___ ___ ___ ___ ___
//  1   1   1   1   0   0   0   0	=  in tm_input 
// 										and
//  0   0   0   0   0   0   1   1	=  in tm_input1 displays 0 ... and so on

	const unsigned char digit_representation_0[18] = {0b11110000, 0b10000000, 0b01101000, // 0, 1, 2,
										0b11001000, 0b10011000, //  3, 4
										0b11011000, 0b11111000, 0b10000000, 0b11111000, 0b11011000, // 5, 6, 7, 8, 9
	                                    0b10111000, 0b11111000, 0b01110000, 0b11111000, 0b01111000, // A, b, C, d, E,
	                                    0b00111000, 0b00001000, //  F, minus sign
	                                    0b00011000 // degree sign
	                                   };
	const unsigned char digit_representation_1[18]= {0b00000011, 0b00000001, 0b00000011, // 0, 1, 2,
										0b00000011, 0b00000001, //  3, 4
										0b00000010, 0b00000010, 0b00000011, 0b00000011, 0b00000011, // 5, 6, 7, 8, 9
	                                    0b00000011, 0b00000000, 0b00000010, 0b00000000, 0b00000010, // A, b, C, d, E, 
	                                    0b00000010, 0b00000000, // F, minus sign
	                                    0b00000011 // degree sign
	                                   };
	const unsigned char led_representation[18] = {
										1, 			1, 			0, 			  2, 			 0, 
									 // POWER_LED,  HEATER_LED, NO_WATER_LED, ONE_DIGIT_LED, SETUP_LED
										2, 2, 1, 1, 0, 
									 // GRADUS_LED, RH_LED,     ION_LED,    1_LEVEL_LED,2_LEVEL_LED
	                                    1, 0, 2, 1, 0, 
									 // 3_LEVEL_LED,4_LEVEL_LED,5_LEVEL_LED,6_LEVEL_LED,7_LEVEL_LED,
	                                    1, 0, 
									 // 8_LEVEL_LED, 9_LEVEL_LED
	                                    2 
									 // DVOETOCHIE_LED
	                                   };
	const unsigned char led_address[18] = {
										10, 		0, 			0, 		  		0, 		 	 12, 
									 // POWER_LED,  HEATER_LED, NO_WATER_LED, ONE_DIGIT_LED, SETUP_LED
										12, 		8, 		12, 		2, 		2, 
									 // GRADUS_LED, RH_LED,     ION_LED,    1_LEVEL_LED,2_LEVEL_LED
	                                    4, 		4, 		6, 		6, 		6, 
									 // 3_LEVEL_LED,4_LEVEL_LED,5_LEVEL_LED,6_LEVEL_LED,7_LEVEL_LED,
	                                    8, 		 8, 
									 // 8_LEVEL_LED, 9_LEVEL_LED
	                                    2 
									 // DVOETOCHIE_LED
	                                   };
	byte tm_buffer[14] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

public:
	// Run once:
	// 1. Constructor: set the corresponding pins
	TM1628ts(int clockPin, int dataPin, int strobePin);

	// 2. Initialize display with empty RAM and keep it off (-1) or power it on with intensity 0..7
	//    Call it before before any other function !
	void init(int intensity = -1);

	// Turn on/off and set intensity
	// 1. Change intensity 0..7; -1 = display off
	void setIntensity(int intensity = -1);

	// Buffer operations
	// 1. Set digit at position in RAM buffer (overwrites previous digit)
	void putDigitAt(byte digit, int digit_position);

	// 2. Set a number at position in RAM (overwrites positions occupied by the number)
	void putNumberAt(long int num, int startpos = 1, int negative = 0, byte base = 10);

	// 3. Empty entire RAM buffer (-1) or a single position (1..7)
	void clearBuffer(int digit_position = -1);

	// 4. Set a status LED on (1) or off (0) in RAM buffer
	void setStatus(byte status, int on = 1); // use only one status code at a time !!!

	// 5. Write buffer to display controller (needed after above functions to actually send display data to controller)
	void writeBuffer();

	// Read controller
	// 1. Get keyboard mask
	byte getKeyboard();


private:
	int tm_clk, tm_dio, tm_stb;

	void tm_sendByte(byte data);
	byte tm_receiveByte();

	void tm_sendCommand(byte data);

};

#endif