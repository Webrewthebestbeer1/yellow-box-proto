#include "LedControl.h"
#include <Adafruit_MAX31865.h>
#include <math.h>

Adafruit_MAX31865 max = Adafruit_MAX31865(45);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(12,11,10,1);

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
int encoderPos = 200; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
int oldEncPos = 200; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent


void setup() {
  max.begin(MAX31865_2WIRE);  // set to 2WIRE or 4WIRE as necessary
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
  
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(9600); // start the serial monitor link
  Serial.begin(115200);
}

void displayBoil() {
  lc.clearDisplay(0);
  lc.setChar(0, 3, 'b', false);
  lc.setChar(0, 2, 'o', false);
  lc.setRow(0 , 1, B00010000); // i
  lc.setRow(0 , 0, B00001100); // l
}

void displayError(uint8_t error) {
  lc.clearDisplay(0);
  lc.setChar(0, 3, 'e', false);
  lc.setChar(0, 2, 'R', false);
  lc.setChar(0, 1, 'R', false);
  if (error & MAX31865_FAULT_HIGHTHRESH) {
    lc.setChar(0, 0, 0, false);
  }
  if (error & MAX31865_FAULT_LOWTHRESH) {
    lc.setChar(0, 0, 1, false);
  }
  if (error & MAX31865_FAULT_REFINLOW) {
    lc.setChar(0, 0, 2, false);
  }
  if (error & MAX31865_FAULT_REFINHIGH) {
    lc.setChar(0, 0, 3, false);
  }
  if (error & MAX31865_FAULT_RTDINLOW) {
    lc.setChar(0, 0, 4, false);
  }
  if (error & MAX31865_FAULT_OVUV) {
    lc.setChar(0, 0, 5, false);
  }
  
}

void displayTemperature(float temp) {
  lc.clearDisplay(0);
  double fractpart, intpart;
  fractpart = modf(temp, &intpart);
  int integer = (int) intpart;
  int decimals = (int) ((fractpart * 100) + 0.5);
  if (integer > 9) {
    int tens = integer / 10 % 10;
    int units = integer % 10;
    lc.setChar(0, 3, tens, false);
    lc.setChar(0, 2, units, true);
  } else {
    lc.setChar(0, 2, integer, true);
  }
  int decimalTens = decimals / 10 % 10;
  // int decimalUnits = decimals % 10;
  lc.setChar(0, 1, decimalTens, false);
  // lc.setChar(0, 0, decimalUnits, false);
  lc.setChar(0, 0, 'c', false);
}

void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
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
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void loop() {
  if (oldEncPos != encoderPos) {
    if (encoderPos < 0) {
      encoderPos = 0;
    }
    Serial.println(encoderPos / 10.0);
    oldEncPos = encoderPos;
  }
  uint16_t rtd = max.readRTD();

  float ratio = rtd;
  ratio /= 32768;
  float temp = max.temperature(RNOMINAL, RREF);
  uint8_t error = max.readFault();
  if (error) {
    displayError(error);
  } else if (temp >= 100.0) {
    displayBoil();
  } else {
    displayTemperature(encoderPos / 10.0);  
  }
  
  Serial.print("Temperature = "); Serial.println(temp);

  
  // Check and print any faults
  uint8_t fault = max.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    max.clearFault();
  }
  
  delay(1000);
}
