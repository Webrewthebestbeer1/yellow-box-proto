void setup() {
  max.begin(MAX31865_2WIRE);
  displayTemp.setBrightness(0x0f);
  displayTarget.setBrightness(0x0f);
  pinMode(MAIN_SWITCH_PIN, INPUT);
  pinMode(HEAT_SWITCH_PIN, INPUT);
  pinMode(PUMP_SWITCH_PIN, INPUT);
  pinMode(MAIN_SWITCH_PIN, INPUT);
  pinMode(HEAT_TRANSISTOR_PIN, OUTPUT);
  pinMode(PUMP_TRANSISTOR_PIN, OUTPUT);
  pinMode(ROTARY_ENCODER_PIN_A, INPUT);
  pinMode(ROTARY_ENCODER_PIN_B, INPUT);
  ROTARY_ENCODER_LAST_STATE = digitalRead(ROTARY_ENCODER_PIN_A);
  Serial.begin(9600);
}

