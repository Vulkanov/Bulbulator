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

#include "Arduino.h"
#include "TM1628ts.h"



// *** Public functions
TM1628ts::TM1628ts(int clockPin, int dataPin, int strobePin)
{
	tm_clk = clockPin;
	tm_dio = dataPin;
	tm_stb = strobePin;
}

void TM1628ts::init(int intensity)
{
	pinMode(tm_dio, OUTPUT);
	pinMode(tm_clk, OUTPUT);
	pinMode(tm_stb, OUTPUT);

	digitalWrite(tm_stb, HIGH);
	digitalWrite(tm_clk, HIGH);

	delay(200);

	tm_sendCommand(0b01000000); // command 2

	digitalWrite(tm_stb, LOW);
	tm_sendByte(0b11000000); // command 3
	for (int i = 0; i < 14; i++)
		tm_sendByte(0x00); // clear RAM
	digitalWrite(tm_stb, HIGH);

	tm_sendCommand(0b00000011); // command 1

	setIntensity(intensity);
}

void TM1628ts::setIntensity(int intensity)
{
	if (intensity < 0)
		{
			tm_sendCommand(0x80); // command 4
			return;
		}

	tm_sendCommand(0x88 | (intensity % 8)); // command 4
}

void TM1628ts::putDigitAt(byte digit, int digit_position)
{
	if ((digit_position < 0) || (digit_position > 12)) // Номер сегмента (один сегмент в 2х битах)
		return;
	
	for (int i = 0; i < 8; i++) {
		for (int n = 0; n < 8; n++) {
			bitWrite(tm_buffer[digit_position], n, bitRead(digit_representation_0[digit], n));
		}
		for (int m = 0; m < 8; m++) {
			bitWrite(tm_buffer[digit_position+1], m, bitRead(digit_representation_1[digit], m));
		}
	}
}

void TM1628ts::putNumberAt(long int num, int startpos, int negative, byte base) // Позиция считается с 12 в обратную сторону
{
	if ((startpos > 14) || (base < 2) || (base > 16))
		return;

	byte digit_position = startpos;

	do
		{
			putDigitAt(num % base, digit_position);
			num = num / base;
			digit_position-=2;
		}
	while ((num > 0) && (digit_position <= 14));

	if ((digit_position < 7) && (negative))
		putDigitAt(16, digit_position);
}

void TM1628ts::clearBuffer(int digit_position)
{
	if (digit_position == -1)
		{
			for (int i = 0; i < 14; i++)
				tm_buffer[i] = 0x00;
			return;
		}

	if (digit_position >= 8)
		return;

	for (int i = 0; i < 7; i++)
		bitWrite(tm_buffer[i * 2], digit_position, 0);
}

void TM1628ts::writeBuffer()
{
	tm_sendCommand(0b01000000); // command 2
	digitalWrite(tm_stb, LOW);
	tm_sendByte(0b11000000); // command 3
	for (int i = 0; i < 14; i++)
		tm_sendByte(tm_buffer[i]); // set RAM
	digitalWrite(tm_stb, HIGH);
}

byte TM1628ts::getKeyboard()
{
	byte keys = 0;

	digitalWrite(tm_stb, LOW);
	tm_sendByte(0b01000010); // command 2
	for (int i = 0; i < 5; i++)
		keys |= tm_receiveByte() << i;
	digitalWrite(tm_stb, HIGH);

	return keys;
}

void TM1628ts::setStatus(byte status_led, int on) {
	bitWrite(tm_buffer[(led_address[status_led])], led_representation[status_led], (on > 0 ? 1 : 0));
}

// *** Basic communication
void TM1628ts::tm_sendByte(byte data)
{
	for (int i = 0; i < 8; i++)
		{
			digitalWrite(tm_clk, LOW);
			digitalWrite(tm_dio, data & 1 ? HIGH : LOW);
			data >>= 1;
			digitalWrite(tm_clk, HIGH);
		}
}

byte TM1628ts::tm_receiveByte()
{
	byte temp = 0;

	pinMode(tm_dio, INPUT);
	digitalWrite(tm_dio, HIGH);

	for (char i = 0; i < 8; i++)
		{
			temp >>= 1;
			digitalWrite(tm_clk, LOW);
			if (digitalRead(tm_dio))
				temp |= 0x80;
			digitalWrite(tm_clk, HIGH);
		}

	pinMode(tm_dio, OUTPUT);
	digitalWrite(tm_dio, LOW);

	return temp;
}

void TM1628ts::tm_sendCommand(byte data)
{
	digitalWrite(tm_stb, LOW);
	tm_sendByte(data);
	digitalWrite(tm_stb, HIGH);
}
