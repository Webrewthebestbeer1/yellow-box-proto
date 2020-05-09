#include <TM1637Display.h>
#include <Adafruit_MAX31865.h>

// Global vars
boolean MainSwitch = false;
boolean HeatSwitch = false;
boolean PumpSwitch = false;
boolean ModeSwitch = false;

int RotaryEncoderState;
int RotaryEncoderLastState;
float RotaryEncoderReading = 0.0;
float RotaryEncoderSpeed = 0.1;
unsigned long RotaryEncoderLastChangeTime = 0;

float TemperatureReading = 0.0;
uint8_t TemperatureError;
unsigned long TimeNow = 0;

// LCD Pins
#define CLK_TEMP 13
#define DIO_TEMP 12
#define CLK_TARGET 11
#define DIO_TARGET 10
#define TERMOMETER_CS 6
#define TERMOMETER_SDI 7
#define TERMOMETER_SDO 8
#define TERMOMETER_CLK 9

// Constants
const int MAIN_SWITCH_PIN = 4;
const int MODE_SWITCH_PIN = 5;
const int HEAT_SWITCH_PIN = A0;
const int PUMP_SWITCH_PIN = A1;
 
const int HEAT_TRANSISTOR_PIN = A2;
const int PUMP_TRANSISTOR_PIN = A3;
 
const int ROTARY_ENCODER_PIN_A = 2;
const int ROTARY_ENCODER_PIN_B = 3;

const int DUTY_CYCLE = 1000;

const uint8_t SEG_BOIL[] = {
  SEG_F | SEG_E | SEG_D | SEG_C | SEG_G,            // b
  SEG_C | SEG_D | SEG_E | SEG_G,                    // o
  SEG_E,                                            // i
  SEG_D | SEG_E                                     // l
};
const uint8_t SEG_ERROR[] = {
  SEG_A | SEG_D | SEG_F | SEG_E | SEG_G,            // E
  SEG_E | SEG_G,                                    // r
  SEG_E | SEG_G,                                    // r
  SEG_C | SEG_D | SEG_E | SEG_G                     // o
};
const uint8_t SEG_CELCIUS[] = {
  SEG_E | SEG_F | SEG_A | SEG_D                     // C
};
const uint8_t SEG_OFF[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,    // O
  SEG_A | SEG_E | SEG_F | SEG_G,                    // F
  SEG_A | SEG_E | SEG_F | SEG_G                     // F
};

// Imported functionality
//
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

TM1637Display DisplayTemp(CLK_TEMP, DIO_TEMP);
TM1637Display DisplayTarget(CLK_TARGET, DIO_TARGET);

Adafruit_MAX31865 max = 
  Adafruit_MAX31865(
    TERMOMETER_CS, TERMOMETER_SDI, TERMOMETER_SDO, TERMOMETER_CLK);

