void printSwitches() {
  Serial.print("MainSwitch ");
  Serial.println(MainSwitch ? "on" : "off");
  Serial.print("HeatSwitch ");
  Serial.println(HeatSwitch ? "on" : "off");
  Serial.print("PumpSwitch ");
  Serial.println(PumpSwitch ? "on" : "off");
  Serial.print("ModeSwitch ");
  Serial.println(ModeSwitch ? "on" : "off");
  Serial.println();
}

void printRotaryEncoder() {
  Serial.print("RotaryEncoderState ");
  Serial.println(RotaryEncoderState);
  Serial.print("RotaryEncoderLastState ");
  Serial.println(RotaryEncoderLastState);
  Serial.print("RotaryEncoderReading ");
  Serial.println(RotaryEncoderReading);
  Serial.print("RotaryEncoderSpeed ");
  Serial.println(RotaryEncoderSpeed);
  Serial.print("RotaryEncoderLastChangeTime ");
  Serial.println(RotaryEncoderLastChangeTime);
  Serial.println();
}

void printTemperature() {
  Serial.print("TemperatureReading ");
  Serial.println(RotaryEncoderState);
  Serial.print("TemperatureError ");
  Serial.println(TemperatureError);
  Serial.println();
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

