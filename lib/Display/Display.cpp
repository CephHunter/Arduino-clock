/*
  Display.cpp - Library for driving a 7*4 display with four shift registers
  Created by Pieter Partous, 11-nov-2017.
*/

#include "Arduino.h"
#include "Display.h"

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitToSegment[] = {
 // :GFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001     // F
};

// Constructor
Display::Display(uint8_t SER, uint8_t SRCLK, uint8_t RCLK1, uint8_t RCLK2, uint8_t RCLK3, uint8_t RCLK4, bool CC) {
  // Copy the pin numbers
  _SER    = SER;
  _SRCLK  = SRCLK;
  _RCLK1  = RCLK1;
  _RCLK2  = RCLK2;
  _RCLK3  = RCLK3;
  _RCLK4  = RCLK4;
  _CC     = CC;

  pinMode(_SER, OUTPUT);
  pinMode(_SRCLK, OUTPUT);
  pinMode(_RCLK1, OUTPUT);
  pinMode(_RCLK2, OUTPUT);
  pinMode(_RCLK3, OUTPUT);
  pinMode(_RCLK4, OUTPUT);

  digitalWrite(_SER, LOW);
  digitalWrite(_SRCLK, LOW);
  digitalWrite(_RCLK1, LOW);
  digitalWrite(_RCLK2, LOW);
  digitalWrite(_RCLK3, LOW);
  digitalWrite(_RCLK4, LOW);
}

void Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos) {
  for (uint8_t i = 1; i <= length; i++) {
    writeByte(segments[i], pos + i - length);
  }
}

void Display::showNumberDec(int num, bool leading_zero, uint8_t length, uint8_t pos) {
  uint8_t digits[4];
	const static int divisors[] = { 1, 10, 100, 1000 };
	bool leading = true;

	for(int8_t k = 0; k < 4; k++) {
	    int divisor = divisors[4 - 1 - k];
		int d = num / divisor;

		if (d == 0) {
		  if (leading_zero || !leading || (k == 3))
		    digits[k] = encodeDigit(d);
	      else
		    digits[k] = 0;
		}
		else {
			digits[k] = encodeDigit(d);
			num -= d * divisor;
			leading = false;
		}
	}

	setSegments(digits + (4 - length), length, pos);
}

uint8_t Display::encodeDigit(uint8_t digit) {
  return digitToSegment[digit & 0x0f];
}

void Display::writeByte(uint8_t data, uint8_t pos) {
  uint8_t _data = data;
  if (!_CC) {
    _data = ~_data;
  }

  // Push data byte into shift register storage
  for (int8_t i = 7; i >= 0; i--) {
    if (bitRead(_data, i) == 1) {
      digitalWrite(_SER, HIGH);
      delayMicroseconds(100);
      //Serial.print("1");
    } else {
      digitalWrite(_SER, LOW);
      delayMicroseconds(100);
      //Serial.print("0");
    }

    digitalWrite(_SRCLK, HIGH);
    delayMicroseconds(100);
    digitalWrite(_SRCLK, LOW);
    delayMicroseconds(100);
  }

  // Push shift register storage to real output
  Serial.println(pos);
  switch (pos) {
    case 0:
      RCLK = _RCLK1;
      Serial.println("pos = 1");
      break;
    case 1:
      RCLK = _RCLK2;
      Serial.println("pos = 2");
      break;
    case 2:
      RCLK = _RCLK3;
      Serial.println("pos = 3");
      break;
    case 3:
      RCLK = _RCLK4;
      Serial.println("pos = 4");
      break;
  }

  digitalWrite(RCLK, HIGH);
  delayMicroseconds(100);
  digitalWrite(RCLK, LOW);
  delayMicroseconds(100);
  //Serial.println(" ");
  //Serial.println("--------");
}
