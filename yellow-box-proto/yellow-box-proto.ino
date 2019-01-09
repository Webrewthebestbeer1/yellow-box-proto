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
  lc.setRow(0 , 2, B00000101); // r
  lc.setRow(0 , 1, B00000101); // r
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

void loop() {
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
    displayTemperature(temp);
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
