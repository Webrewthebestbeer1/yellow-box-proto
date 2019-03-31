#include <TM1637Display.h>

// LCD Pins
#define CLK_TEMP 13
#define DIO_TEMP 12
#define CLK_TARGET 11
#define DIO_TARGET 10
#define TERMOMETER_CS 6
#define TERMOMETER_SDI 7
#define TERMOMETER_SDO 8
#define TERMOMETER_CLK 9
static int mainSwitchPin = 4;
static int modeSwitchPin = 5;
static int heatSwitchPin = A0;
static int pumpSwitchPin = A1;
static int heatTransistorPin = A2;
static int pumpTransistorPin = A3;
static int rotaryEncoderPinA = 2;
static int rotaryEncoderPinB = 3;
volatile byte rotaryEncoderFlagA = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte rotaryEncoderFlagB = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
int encoderPos = 200; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
int oldEncPos = 200; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
//volatile byte reading = 0;
unsigned long encoderLastChangeTime = 0;

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
TM1637Display displayTemp(CLK_TEMP, DIO_TEMP);
TM1637Display displayTarget(CLK_TARGET, DIO_TARGET);


#include <Adafruit_MAX31865.h>
Adafruit_MAX31865 max = Adafruit_MAX31865(TERMOMETER_CS, TERMOMETER_SDI, TERMOMETER_SDO, TERMOMETER_CLK);

// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  100.0

boolean mainSwitch = false;
boolean heatSwitch = false;
boolean pumpSwitch = false;
boolean modeSwitch = false;
float temperatureReading = 0.0;
float rotaryEncoderReading = 0.0;

unsigned long time_now = 0;

void setup() {
  max.begin(MAX31865_2WIRE);
  displayTemp.setBrightness(0x0f);
  displayTarget.setBrightness(0x0f);
  pinMode(mainSwitchPin, INPUT);
  pinMode(heatSwitchPin, INPUT);
  pinMode(pumpSwitchPin, INPUT);
  pinMode(mainSwitchPin, INPUT);
  pinMode(heatTransistorPin, OUTPUT);
  pinMode(pumpTransistorPin, OUTPUT);
  pinMode(rotaryEncoderPinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(rotaryEncoderPinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0, rotaryEncoderInterruptA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, rotaryEncoderInterruptB, RISING);
  Serial.begin(9600);
}


void displayTemperature(TM1637Display display, float temp, boolean error=false) {
  if (error || temp < 0) {
    display.setSegments(SEG_ERROR);
  }
  else if (temp >= 100) {
    display.setSegments(SEG_BOIL); 
  } else {
    display.showNumberDecEx(round(temp*10), 0b01000000, false, 3, 0);
    display.setSegments(SEG_CELCIUS, 1, 3); 
  }
}

void readAndDisplayTemperature() {
  uint16_t rtd = max.readRTD();
  float ratio = rtd;
  ratio /= 32768;
  float temp = max.temperature(RNOMINAL, RREF);
  uint8_t error = max.readFault();
  temperatureReading = temp;
  displayTemperature(displayTemp, temp, error); 
  Serial.print("Temp: "); Serial.println(temp);
}

void readRotaryEncoder() {
  if (oldEncPos != encoderPos) {
    if (encoderPos < 0) {
      encoderPos = 0;
    }
    if (encoderPos > 1000.0) {
      encoderPos = 1000.0;
    }
    Serial.println(encoderPos / 10.0);
    displayTemperature(displayTarget, encoderPos / 10.0);
    rotaryEncoderReading = encoderPos / 10.0;
    oldEncPos = encoderPos;
  }
}

void rotaryEncoderInterruptA() {
  //Serial.print("interrupt A");
  cli(); //stop interrupts happening before we read pin values
  volatile byte reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && rotaryEncoderFlagA) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (millis() - encoderLastChangeTime < 20) {
      encoderPos = encoderPos - 10;
    } else {
      encoderPos -= 10; //decrement the encoder's position count  
    }
    encoderLastChangeTime = millis();
    
    rotaryEncoderFlagA = 0; //reset flags for the next turn
    rotaryEncoderFlagB = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) rotaryEncoderFlagB = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void rotaryEncoderInterruptB() {
  //Serial.print("interrupt B");
  cli(); //stop interrupts happening before we read pin values
  volatile byte reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && rotaryEncoderFlagB) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (millis() - encoderLastChangeTime < 20) {
      encoderPos = encoderPos + 10;
    } else {
      encoderPos += 10; //decrement the encoder's position count  
    }
    encoderLastChangeTime = millis();
    rotaryEncoderFlagA = 0; //reset flags for the next turn
    rotaryEncoderFlagB = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) rotaryEncoderFlagA = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void readSwitches() {
  boolean newMainSwitch = !digitalRead(mainSwitchPin);
  if (newMainSwitch != mainSwitch) {
    mainSwitch = newMainSwitch;
    Serial.print("mainSwitch ");
    Serial.println(mainSwitch ? "on" : "off");
  }
  boolean newModeSwitch = !digitalRead(modeSwitchPin);
  if (newModeSwitch != modeSwitch) {
    modeSwitch = newModeSwitch;
    Serial.print("modeSwitch ");
    Serial.println(modeSwitch ? "on" : "off");
  }
  boolean newHeatSwitch = !digitalRead(heatSwitchPin);
  if (newHeatSwitch != heatSwitch) {
    heatSwitch = newHeatSwitch;
    Serial.print("heatSwitch ");
    Serial.println(heatSwitch ? "on" : "off");
  }
  boolean newPumpSwitch = !digitalRead(pumpSwitchPin);
  if (newPumpSwitch != pumpSwitch) {
    pumpSwitch = newPumpSwitch;
    Serial.print("pumpSwitch ");
    Serial.println(pumpSwitch ? "on" : "off");
  }
}

void loop() {
  readSwitches();
  if (mainSwitch) {
    readRotaryEncoder();
    if (millis() > time_now + 1000) {
      time_now = millis();
      readAndDisplayTemperature();
    }
    if (modeSwitch) {
      digitalWrite(heatTransistorPin, (temperatureReading < rotaryEncoderReading) ? HIGH : LOW);
      digitalWrite(pumpTransistorPin, HIGH);
    } else{
      digitalWrite(heatTransistorPin, heatSwitch ? HIGH : LOW);
      digitalWrite(pumpTransistorPin, pumpSwitch ? HIGH : LOW);
    }
  } else {
    displayTemp.setSegments(SEG_OFF); 
    displayTarget.setSegments(SEG_OFF); 
  }
}
