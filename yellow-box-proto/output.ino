void printSwitches() {
  Serial.println();
  Serial.print("MainSwitch ");
  Serial.print(MainSwitch ? "on" : "off");
  Serial.print(" | HeatSwitch ");
  Serial.print(HeatSwitch ? "on" : "off");
  Serial.print(" | PumpSwitch ");
  Serial.print(PumpSwitch ? "on" : "off");
  Serial.print(" | ModeSwitch ");
  Serial.print(ModeSwitch ? "on" : "off");
  Serial.println();
}

void printRotaryEncoder() {
  // TODO: Fiks rotary encoder (dritt)
  // Serial.print("RotaryEncoderState ");
  // Serial.print(RotaryEncoderState);
  // Serial.print(" | RotaryEncoderLastState ");
  // Serial.print(RotaryEncoderLastState);
  // Serial.print(" | RotaryEncoderReading ");
  // Serial.print(RotaryEncoderReading);
  // Serial.print(" | RotaryEncoderSpeed ");
  // Serial.print(RotaryEncoderSpeed);
  // Serial.print(" | RotaryEncoderLastChangeTime ");
  // Serial.print(RotaryEncoderLastChangeTime);
  // Serial.println();
}

void printTemperature() {
  Serial.print("TemperatureReading ");
  Serial.print(TemperatureReading);
  Serial.print(" | TemperatureError ");
  Serial.print(TemperatureError);
  Serial.println();
}

void printHeatDebug() {
  Serial.print("ClockStart ");
  Serial.print(ClockStart);
  Serial.print(" | CycleStart ");
  Serial.print(CycleStart);
  Serial.print(" | HeatOn ");
  Serial.print(digitalRead(HEAT_TRANSISTOR_PIN));
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
