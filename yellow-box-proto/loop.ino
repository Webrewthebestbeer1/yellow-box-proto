void displayTemperature(TM1637Display display, float temp, boolean error=false);

void loop() {
  if (isNextIteration()) {
    readSwitches();
    readRotaryEncoder();
    readTemperature();

    if (MainSwitch) {
      printSwitches();
      printRotaryEncoder();
      printTemperature();
      displayTemperature(displayTarget, RotaryEncoderReading);
      displayTemperature(displayTemp, TemperatureReading, TemperatureError);

      if (ModeSwitch) {
        // TODO: Add smart temperature adjustment here
        controlHeater();
        digitalWrite(PUMP_TRANSISTOR_PIN, HIGH);

      } else {
        digitalWrite(HEAT_TRANSISTOR_PIN, HeatSwitch ? HIGH : LOW);
        digitalWrite(PUMP_TRANSISTOR_PIN, PumpSwitch ? HIGH : LOW);
      }

    } else {
      displayTarget.setSegments(SEG_OFF);
      displayTemp.setSegments(SEG_OFF);
    }
  }
}

boolean isNextIteration() {
  boolean result = false;
  if (millis() >= ClockStart + DUTY_CLOCK) {
    result = true;
    ClockStart = millis();
  }
  return result;
}

