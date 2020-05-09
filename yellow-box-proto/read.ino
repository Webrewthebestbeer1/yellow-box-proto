void readSwitches() {
  boolean newMainSwitch = !digitalRead(MAIN_SWITCH_PIN);
  if (newMainSwitch != MainSwitch) {
    MainSwitch = newMainSwitch;
  }
  boolean newModeSwitch = !digitalRead(MODE_SWITCH_PIN);
  if (newModeSwitch != ModeSwitch) {
    ModeSwitch = newModeSwitch;
  }
  boolean newHeatSwitch = !digitalRead(HEAT_SWITCH_PIN);
  if (newHeatSwitch != HeatSwitch) {
    HeatSwitch = newHeatSwitch;
  }
  boolean newPumpSwitch = !digitalRead(PUMP_SWITCH_PIN);
  if (newPumpSwitch != PumpSwitch) {
    PumpSwitch = newPumpSwitch;
  }
}

void readRotaryEncoder() {
  // Reads the "current" state of the outputA
  RotaryEncoderState = digitalRead(ROTARY_ENCODER_PIN_A); 

  // If the previous and the current state of the outputA are different, 
  // that means a Pulse has occured
  if (RotaryEncoderState != RotaryEncoderLastState) {
    
    // If the outputB state is different to the outputA state, 
    // that means the encoder is rotating clockwise
    if ((millis() - RotaryEncoderLastChangeTime) < 200) {
      RotaryEncoderSpeed += 0.1;
    } else {
      RotaryEncoderSpeed = 0.1;
    }
    if (digitalRead(ROTARY_ENCODER_PIN_B) != RotaryEncoderState) { 
      RotaryEncoderReading += RotaryEncoderSpeed;
    } else {
      RotaryEncoderReading -= RotaryEncoderSpeed;
    }
    RotaryEncoderLastChangeTime = millis();
  } 

  // Updates the previous state of the outputA with the current state
  RotaryEncoderLastState = RotaryEncoderState; 
}

void readTemperature() {
  uint16_t rtd = max.readRTD();
  float ratio = rtd;
  ratio /= 32768;
  TemperatureReading = max.temperature(RNOMINAL, RREF);
  TemperatureError = max.readFault();
}

