#include <Arduino.h>
#include <Display.h>

// Display variables
uint8_t SER   = 2;                    // Pin 2 connected to the serial pins of all four shift registers
uint8_t SRCLK = 3;                    // Pin 3 connected to the Shift register clock pins of all four shift registers
uint8_t RCLK1 = 4;                    // Pin 4 connected to the storage clock of the shift register controlling the left most display
uint8_t RCLK2 = 5;                    // Pin 5 connected to the storage clock of the shift register controlling the second from the left display
uint8_t RCLK3 = 6;                    // Pin 6 connected to the storage clock of the shift register controlling the third from the left display
uint8_t RCLK4 = 7;                    // Pin 7 connected to the storage clock of the shift register controlling the fourth from the left display
bool CC       = true;

// Begin of display
Display display(SER, SRCLK, RCLK1, RCLK2, RCLK3, RCLK4, CC);

// Definition of special characters to be used with the display

const uint8_t Grau[] = {
  SEG_A | SEG_B | SEG_F | SEG_G ,                   //
};
const uint8_t UR[] = {
  SEG_C | SEG_D | SEG_E | SEG_G ,                   // %
};
const uint8_t Celsius[] = {
  SEG_A | SEG_D | SEG_E | SEG_F ,                   // C
};
const uint8_t Fahrenheit[] = {
  SEG_A | SEG_E | SEG_F | SEG_G ,                   // F
};
const uint8_t letraA[] = {
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,    // A
};
const uint8_t letraL[] = {
  SEG_D | SEG_E | SEG_F ,                           // L
};
const uint8_t letraT[] = {
  SEG_D | SEG_E | SEG_F | SEG_G,                    // t
};
const uint8_t letraI[] = {
  SEG_E,                                            // i
};
const uint8_t letraS[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,            // S
};
const uint8_t letraO[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // O
};
const uint8_t letraE[] = {
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,            // E
};
const uint8_t letraN[] = {
  SEG_C | SEG_E | SEG_G,                            // n
};
const uint8_t letraF[] = {
  SEG_A | SEG_E | SEG_F | SEG_G ,                   // F
};

// Definition of numbers to be shown with colon in the display
uint8_t dataWithColon[] = {
  0b10111111, // 0
  0b10000110, // 1
  0b11011011, // 2
  0b11001111, // 3
  0b11100110, // 4
  0b11101101, // 5
  0b11111101, // 6
  0b10000111, // 7
  0b11111111, // 8
  0b11101111, // 9
  0b00000000, // 10
};

void setup() {
    Serial.begin(9600);
}

void loop() {
    display.showNumberDec( 1111, true, 4, 3);
    Serial.println("-");
    delay(1000);
}
