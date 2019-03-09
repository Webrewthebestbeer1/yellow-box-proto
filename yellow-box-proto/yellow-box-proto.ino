#include "TM1637.h"
#include <Adafruit_MAX31865.h>
#include <math.h>

// Adafruit_MAX31865 max = Adafruit_MAX31865(45);
Adafruit_MAX31865 max = Adafruit_MAX31865(7, 8, 9, 10);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

TM1637 lc(12, 11);

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
int encoderPos = 200; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
int oldEncPos = 200; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
unsigned long time_now = 0;
unsigned long encoderLastChangeTime = 0;

void setup() {
  max.begin(MAX31865_2WIRE);  // set to 2WIRE or 4WIRE as necessary
  lc.init();
  lc.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(9600); // start the serial monitor link
}

void displayBoil() {
  lc.display(3, 'b');
  lc.display(2, 'o');
  lc.display(1, B00010000); // i
  lc.display(0, B00001100); // l
}
/*
void displayError(uint8_t error) {
  for (int i = 3; i > 3 - 4; i--) {
    lc.setRow(0, i, B00000000);
  }
  lc.setChar(0, 3, 'E', false);
  lc.setChar(0, 2, 1, false);
  lc.setChar(0, 1, 0, false);
  if (error & MAX31865_FAULT_HIGHTHRESH) {
    lc.setChar(0, 0, 1, false);
  }
  if (error & MAX31865_FAULT_LOWTHRESH) {
    lc.setChar(0, 0, 2, false);
  }
  if (error & MAX31865_FAULT_REFINLOW) {
    lc.setChar(0, 0, 3, false);
  }
  if (error & MAX31865_FAULT_REFINHIGH) {
    lc.setChar(0, 0, 4, false);
  }
  if (error & MAX31865_FAULT_RTDINLOW) {
    lc.setChar(0, 0, 5, false);
  }
  if (error & MAX31865_FAULT_OVUV) {
    lc.setChar(0, 0, 6, false);
  }
  
}
*/

void displayTemperature(float temp) {
  double fractpart, intpart;
  fractpart = modf(temp, &intpart);
  int integer = (int) intpart;
  int decimals = (int) ((fractpart * 100) + 0.5);
  if (integer > 9) {
    int tens = integer / 10 % 10;
    int units = integer % 10;
    lc.display(0, tens);
    lc.display(1, units);
  } else {
    lc.display(1, integer); // TODO: dot
    lc.showNumberDecEx(0, (0x80 >> 2), true);
  }
  int decimalTens = decimals / 10 % 10;
  // int decimalUnits = decimals % 10;
  lc.display(2, decimalTens);
  // lc.setChar(0, 0, decimalUnits, false);
  lc.display(3, 12);
}

void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (millis() - encoderLastChangeTime < 20) {
      encoderPos = encoderPos - 10;
    } else {
      encoderPos --; //decrement the encoder's position count  
    }
    encoderLastChangeTime = millis();
    
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (millis() - encoderLastChangeTime < 20) {
      encoderPos = encoderPos + 10;
    } else {
      encoderPos ++; //decrement the encoder's position count  
    }
    encoderLastChangeTime = millis();
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void readAndDisplayTemperature() {
  uint16_t rtd = max.readRTD();
  float ratio = rtd;
  ratio /= 32768;
  float temp = max.temperature(RNOMINAL, RREF);
  uint8_t error = max.readFault();
  if (error) {
    // displayError(error);
  } else if (temp >= 100.0) {
    displayBoil();
  } else {
    displayTemperature(temp);  
  }
  Serial.print("Temp: "); Serial.println(temp);
}

void loop() {
  if (oldEncPos != encoderPos) {
    if (encoderPos < 0) {
      encoderPos = 0;
    }
    if (encoderPos > 1000.0) {
      encoderPos = 1000.0;
    }
    Serial.println(encoderPos / 10.0);
    oldEncPos = encoderPos;
  }
  /*
  if (encoderPos >= 1000.0) {
    displayBoil();
  } else {
    displayTemperature(encoderPos / 10.0);
  }
  */
  
  if (millis() > time_now + 1000) {
    time_now = millis();
    readAndDisplayTemperature();
  }
}
