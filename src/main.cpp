#include <Arduino.h>      // Arduino library, needed if saved as a .cpp file

// Declaring functions, needed if saved as a .cpp file
void buzzer();
void lerEncoder();

/*                       Digital Clock by LAGSILVA
     Digital Clock with PIR & LED Display - TM1637 - 4 Digits x 7 Segments
               Display Time, Temperature, Humidity & Dew Point
        Special Features: PIR sensor & Alarm with data recorded at RTC
                            V1.0 - 22.Apr.2016
                          by lagsilva@gmail.com
*/

#include <TM1637Display.h>            // Library of Display TM1637 (I2C)
#include <Bounce2.h>                  // Library to read Encoder key
#include <TimerOne.h>                 // Library of Timer1
#include <Time.h>                     // Time Library
#include <DS1307RTC.h>                // Library of Real Time Clock (RTC)
#include <Wire.h>                     // Library of Wire to support DS1307RTC (Real-Time Clock) - Pins to Arduino UNO: A4 (SDA), A5 (SCL)
// #include <DHT11.h>                    // Library of Temperature and Humidity sensor
// DHT11 library geeft problemene dus de temperatuur sensor heb ik nu uitgezet

#define DS1307_I2C_ADDRESS 0x68       // This is the I2C address (RTC)

int hora, minuto, hh, mm, temp, umid, tpo, dezHora, uniHora, horaAlarme, minutoAlarme;
byte statusPIR, statusAlarme, statusTempo, statusGatilho;

#define DIO 2                         // Arduino Conection on Pin #2 = DIO of Display Module TM1637
#define CLK 3                         // Arduino Conection on Pin #3 = CLK of Display Module TM1637

//Encoder variables
byte encoderPinSW = 4;                // Encoder variable - SW
byte encoderPinB = 5;                 // Encoder variable - DT
byte encoderPinA = 6;                 // Encoder variable - CLK

int PIR = 7;                          // Pin #7 connected to PIR (Presence Infra Red sensor)
int BUZ = 8;                          // Pin #8 connected to buzzer (Alarm)

// int chk;                              // Variable to read the sensor DHT11
// DHT11 DHT                             // Define the name DHT for the sensor of Temperature and Humidity
// #define DHT11_PIN 10                  // Sensor DHT11 conected to the Pin #11 on Arduino

byte encoderPos = 0;
byte encoderPinALast = LOW;
byte n = LOW;
byte linSW = 0;
Bounce SW = Bounce();                 // Definition of Bounce object for the switch (SW) of Encoder

// Begin of display
TM1637Display display(CLK, DIO);

// Segments map of display
//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
//


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

  setSyncProvider(RTC.get);                                 // Update the time with data of RTC (Real Time Clock)
  setSyncInterval(3600);                                  // Set the number of seconds between re-sync

  display.setBrightness (0x0a);                             //(0x0f) is the max brightness

  pinMode(PIR, INPUT);                                      // Define pin of PIR as input
  pinMode(BUZ, OUTPUT);                                     // Define pin of Buzzer as output

  pinMode (encoderPinA, INPUT);
  pinMode (encoderPinB, INPUT);
  pinMode (encoderPinSW, INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  SW.attach(encoderPinSW);
  SW.interval(100);

  //TimerOne initialization
  Timer1.initialize(1000);                                  // Set Timer1 for 1000 microseconds
  Timer1.attachInterrupt(lerEncoder);                       // Timer1 reads the Encoder

  setTime(16, 20, 00, 26, 03, 2016);                      // Set the time and calendar manually
  RTC.set(now());                                         // Update the RTC with current time


  // Read data of Alarm recorded in RTC
  Wire.beginTransmission(DS1307_I2C_ADDRESS);               // Open I2C line in write mode
  Wire.write((byte)0x08);                                   // Set the register pointer to (0x08)
  Wire.endTransmission();                                   // End Write Transmission
  Wire.requestFrom(DS1307_I2C_ADDRESS, 3);                  // In this case read only 3 bytes
  horaAlarme = Wire.read();                                 // Read the hour of Alarm stored at RTC
  minutoAlarme = Wire.read();                               // Read the minute of Alarm stored at RTC
  statusAlarme = Wire.read();                               // Read the status of Alarm stored at RTC

  // Initial values of variables
  hh = 23;
  mm = 59;
  statusGatilho = LOW;
  statusTempo = LOW;

}


void buzzer() {                                             // Routine for Alarm/Buzzer

  display.showNumberDec(dezHora, true, 1, 0);
  display.setSegments(dataWithColon + uniHora, 1, 1);
  display.showNumberDec(minuto, true, 2, 2);

  tone(BUZ, 880, 300);
  delay(300);
  tone(BUZ, 523, 200);
  delay(200);

  display.setSegments(dataWithColon + 10, 1, 0);            // Clear digit #1
  display.setSegments(dataWithColon + 10, 1, 1);            // Clear digit #2
  display.setSegments(dataWithColon + 10, 1, 2);            // Clear digit #3
  display.setSegments(dataWithColon + 10, 1, 3);            // Clear digit #4
  delay(100);

}


void lerEncoder() {  //Encoder reading

  // Update the Bounce instance :
  SW.update();
  //Check the Encoder key status
  if (SW.fell()) {
    linSW++;
    if (linSW <= 6) {
      statusPIR = LOW;
      display.setSegments(letraS, 1, 0);                    // Show the symbol of word S
      display.setSegments(letraE, 1, 1);                    // Show the symbol of word E
      display.setSegments(letraT, 1, 2);                    // Show the symbol of word T
      display.setSegments(dataWithColon + 10, 1, 3);        // Clear digit #4
      delay(500);
    }
    else {
      linSW = 0;
      statusPIR = HIGH;
    }
  }

  switch (linSW) {


    case 1: //Set the Hours of Alarm

      n = digitalRead(encoderPinA);

      if ((encoderPinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoderPinB) == LOW) {
          encoderPos++;
          horaAlarme++;
        } else {
          encoderPos--;
          horaAlarme--;
        }
        encoderPinALast = n;
        horaAlarme = constrain(horaAlarme, 0, 23);
        display.showNumberDec(horaAlarme, true, 2, 0);
        display.setSegments(letraA, 1, 2);                    // Show the symbol of word A
        display.setSegments(letraL, 1, 3);                    // Show the symbol of word L
      }


    case 2: //Set the Minutes of Alarm

      n = digitalRead(encoderPinA);

      if ((encoderPinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoderPinB) == LOW) {
          encoderPos++;
          minutoAlarme++;
        } else {
          encoderPos--;
          minutoAlarme--;
        }
        encoderPinALast = n;
        minutoAlarme = constrain(minutoAlarme, 0, 59);
        display.setSegments(letraA, 1, 0);                    // Show the symbol of word A
        display.setSegments(letraL, 1, 1);                    // Show the symbol of word L
        display.showNumberDec(minutoAlarme, true, 2, 2);
      }


    case 3: //Set status of Alarm (On-Off)

      n = digitalRead(encoderPinA);

      if ((encoderPinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoderPinB) == LOW) {
          encoderPos++;
          statusAlarme = HIGH;
        } else {
          encoderPos--;
          statusAlarme = LOW;
        }
        encoderPinALast = n;
        if (statusAlarme == LOW) {
          display.setSegments(letraO, 1, 0);                // Show the symbol of word O
          display.setSegments(letraF, 1, 1);                // Show the symbol of word F
          display.setSegments(letraF, 1, 2);                // Show the symbol of word F
          display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
        }
        if (statusAlarme == HIGH) {
          //display.showNumberDec(1111, true, 4, 0);
          display.setSegments(letraO, 1, 0);                // Show the symbol of word O
          display.setSegments(letraN, 1, 1);                // Show the symbol of word N
          display.setSegments(dataWithColon + 10, 1, 2);    // Clear digit #3
          display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
        }
        statusGatilho = HIGH;
      }


    case 4: //Set the Hour of Time

      n = digitalRead(encoderPinA);

      if ((encoderPinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoderPinB) == LOW) {
          encoderPos++;
          hh++;
        } else {
          encoderPos--;
          hh--;
        }
        encoderPinALast = n;
        hh = constrain(hh, 0, 23);
        display.showNumberDec(hh, true, 2, 0);
        display.setSegments(letraT, 1, 2);                    // Show the symbol of word T
        display.setSegments(letraI, 1, 3);                    // Show the symbol of word I
      }


    case 5: //Set the Minutes of Time

      n = digitalRead(encoderPinA);

      if ((encoderPinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoderPinB) == LOW) {
          encoderPos++;
          mm++;
        } else {
          encoderPos--;
          mm--;
        }
        encoderPinALast = n;
        mm = constrain(mm, 0, 59);
        display.setSegments(letraT, 1, 0);
        display.setSegments(letraI, 1, 1);                    // Show the symbol of word T
        display.showNumberDec(mm, true, 2, 2);                // Show the symbol of word I
      }


    case 6: //Set Time Adjustment

      n = digitalRead(encoderPinA);

      if ((encoderPinALast == LOW) && (n == HIGH)) {
        if (digitalRead(encoderPinB) == LOW) {
          encoderPos++;
          statusTempo = HIGH;
        } else {
          encoderPos--;
          statusTempo = LOW;
        }
        encoderPinALast = n;
        if (statusTempo == LOW) {
          display.setSegments(letraO, 1, 0);                // Show the symbol of word O
          display.setSegments(letraF, 1, 1);                // Show the symbol of word F
          display.setSegments(letraF, 1, 2);                // Show the symbol of word F
          display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
        }
        if (statusTempo == HIGH) {
          //display.showNumberDec(1111, true, 4, 0);
          display.setSegments(letraO, 1, 0);                // Show the symbol of word O
          display.setSegments(letraN, 1, 1);                // Show the symbol of word N
          display.setSegments(dataWithColon + 10, 1, 2);    // Clear digit #3
          display.setSegments(dataWithColon + 10, 1, 3);    // Clear digit #4
        }
      }

      encoderPinALast = n;

  }
}


void loop() {

  // chk = DHT.read(DHT11_PIN);                                // Read data of sensor DHT11

  //Clear all digits of display
  display.setSegments(dataWithColon + 10, 1, 0);            // Clear digit #1
  display.setSegments(dataWithColon + 10, 1, 1);            // Clear digit #2
  display.setSegments(dataWithColon + 10, 1, 2);            // Clear digit #3
  display.setSegments(dataWithColon + 10, 1, 3);            // Clear digit #4

  do {                                                      // Keep the display off and check the sensor PIR and the time of Alarm
    if (linSW == 0) {
      statusPIR = digitalRead(PIR);
    }
    hora = hour();
    minuto = minute();
    if (hora == horaAlarme && minuto == minutoAlarme && statusAlarme == HIGH) {
      buzzer();
    }
  } while (statusPIR == LOW);

  hora = hour();
  minuto = minute();
  if (hora == horaAlarme && minuto == minutoAlarme && statusAlarme == HIGH) {
    buzzer();
  }

  //Set Time (adjustment of Hours and Minutes)
  if (linSW == 0 && statusTempo == HIGH) {
    setTime(hh, mm, 00, day(), month(), year());
    RTC.set(now());
    setSyncProvider(RTC.get);
    statusTempo = LOW;
  }

  // Write data of Alarm into RTC
  if (linSW == 0 && statusGatilho == HIGH) {
    Wire.beginTransmission(DS1307_I2C_ADDRESS);               // Open I2C line in write mode
    Wire.write((byte)0x08);                                   // Set the register pointer to (0x08)
    Wire.write(horaAlarme);                                   // Record at RTC memory the Hour of Alarm
    Wire.write(minutoAlarme);                                 // Record at RTC memory the Minute of Alarm
    Wire.write(statusAlarme);                                 // Record at RTC memory the Status of Alarm
    Wire.endTransmission();                                   // End Write Transmission
    statusGatilho = LOW;
  }

  if (linSW == 0) {
    //Display Hour and Minutes
    dezHora = hora / 10;
    uniHora = hora % 10;
    display.showNumberDec(dezHora, true, 1, 0);
    display.setSegments(dataWithColon + uniHora, 1, 1);
    display.showNumberDec(minuto, true, 2, 2);
    delay(2000);
  }

  // if (linSW == 0) {
  //
  //   temp = DHT.temperature;                     // Read temperature in Celsius degrees (C)
  //
  //   //Optional calculation of Temperature in Fahrenheit degrees (F). Remove the comments ("//") of following statement before use it.
  //   //temp = (temp * 18) / 10 + 32;
  //
  //   //Display Temperature (C or F)
  //   display.showNumberDec(temp, true, 2, 0);    // Show Temperature in 2 digits begenning at Display #0 (first from left to right)
  //   display.setSegments(Grau, 1, 2);            // Show the symbol of degree ()
  //   display.setSegments(Celsius, 1, 3);         // Celsius symbol
  //   //display.setSegments(Fahrenheit, 1, 3);    // Optional Fahrenheit symbol. Remove the comments ("//") of following statement before use it.
  //   delay(2000);
  // }

  // if (linSW == 0) {
  //
  //   umid = DHT.humidity + 10;                   // Read Humidity (%)
  //
  //   //Optional calculation of Temperature in Fahrenheit degrees (F). Remove the comments ("//") of following statement to use it.
  //   //temp = (temp * 18) / 10 + 32;
  //
  //   //Display Relative Humidity (%)
  //   display.showNumberDec(umid, true, 2, 0);
  //   display.setSegments(Grau, 1, 2);
  //   display.setSegments(UR, 1, 3);
  //   delay(2000);
  // }

  // if (linSW == 0) {
  //
  //   tpo = temp - (100 - umid) / 5;                // Dew Point calculation (aproximated)
  //
  //   //Optional calculation of Temperature in Fahrenheit degrees (F). Remove the comments ("//") of following statement before use it.
  //   //tpo = (tpo * 18) / 10 + 32;
  //
  //   //Display Dew Point (C or F)
  //   display.showNumberDec(tpo, true, 2, 0);
  //   display.setSegments(UR, 1, 2);
  //   display.setSegments(UR, 1, 3);
  //   delay(2000);
  // }

}
