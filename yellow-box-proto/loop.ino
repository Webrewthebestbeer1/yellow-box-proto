void loop() {
  if (dutyCycle) {
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
        digitalWrite(HEAT_TRANSISTOR_PIN, 
          (TemperatureReading < RotaryEncoderReading) ? HIGH : LOW);
        digitalWrite(PUMP_TRANSISTOR_PIN, HIGH);

      } else {
        digitalWrite(HEAT_TRANSISTOR_PIN, HeatSwitch ? HIGH : LOW);
        digitalWrite(PUMP_TRANSISTOR_PIN, PumpSwitch ? HIGH : LOW);
      }

    } else {
      displayTemp.setSegments(SEG_OFF);
      displayTarget.setSegments(SEG_OFF);
    }
  }
}

boolean dutyCycle() {
  boolean result = false;
  if (millis() > TimeNow + DUTY_CYCLE) {
      result = true;
  }
  return result;
}

