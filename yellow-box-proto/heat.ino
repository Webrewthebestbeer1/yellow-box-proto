void controlHeater() {
  if (temperatureTooLow() && activePartOfCycle()) {
    digitalWrite(HEAT_TRANSISTOR_PIN, HIGH);
  } else {
    digitalWrite(HEAT_TRANSISTOR_PIN, LOW);
  }
}

boolean temperatureTooLow() {
  boolean result = false;

  if (TemperatureRising) {
    if (TemperatureReading + TEMP_RISING_OFFSET < RotaryEncoderReading) {
      result = true;
    } else {
      TemperatureRising = false;
    }

  } else {
    if (TemperatureReading + TEMP_SINKING_OFFSET < RotaryEncoderReading) {
      result = true;
      TemperatureRising = true;
    } 
  }

  return result;
}

boolean activePartOfCycle() {
  boolean result = false;

  if (millis() >= CycleStart + DUTY_CYCLE) {
    CycleStart = millis();
    result = true;

  } else if (millis() < CycleStart + (DUTY_CYCLE * HEATER_ACTIVE)) {
    result = true;
  }

  return result;
}

