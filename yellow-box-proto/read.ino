void readSwitches() {
  MainSwitch = !digitalRead(MAIN_SWITCH_PIN);
  ModeSwitch = !digitalRead(MODE_SWITCH_PIN);
  HeatSwitch = !digitalRead(HEAT_SWITCH_PIN);
  PumpSwitch = !digitalRead(PUMP_SWITCH_PIN);
}

void readRotaryEncoder() {
  // TODO: Fiks rotary encoder
  // Reads the "current" state of the outputA
  // RotaryEncoderState = digitalRead(ROTARY_ENCODER_PIN_A); 

  // If the previous and the current state of the outputA are different, 
  // that means a Pulse has occured
  // if (RotaryEncoderState != RotaryEncoderLastState) {
    
    // If the outputB state is different to the outputA state, 
    // that means the encoder is rotating clockwise
    // if ((millis() - RotaryEncoderLastChangeTime) < 200) {
    //   RotaryEncoderSpeed += 0.1;
    // } else {
    //   RotaryEncoderSpeed = 0.1;
    // }
    // if (digitalRead(ROTARY_ENCODER_PIN_B) != RotaryEncoderState) { 
    //   RotaryEncoderReading += RotaryEncoderSpeed;
    // } else {
    //   RotaryEncoderReading -= RotaryEncoderSpeed;
    // }
    // RotaryEncoderLastChangeTime = millis();
  // } 

  // Updates the previous state of the outputA with the current state
  // RotaryEncoderLastState = RotaryEncoderState; 
}

void readTemperature() {
  uint16_t rtd = max.readRTD();
  float ratio = rtd;
  ratio /= 32768;
  TemperatureReading = max.temperature(RNOMINAL, RREF) + TEMP_ERROR;
  TemperatureError = max.readFault();
}
