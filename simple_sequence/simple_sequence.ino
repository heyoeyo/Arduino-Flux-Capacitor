
// Arduino pin numbers. Order should match desired sequence order!
const uint8_t PIN_NUMBERS[] = {8,9,10,11,12};

// On-time duration of each LED (in milliseconds)
const int ON_TIME_MS = 50;

// ...................................................................................................

// Used internally, don't change
int num_pins;

// ...................................................................................................

void setup() {

  // Figure out how many pins we have
  num_pins = sizeof(PIN_NUMBERS);

  // Make sure each pin is configured as an output
  for (int p = 0; p < num_pins; p++) {
    pinMode(PIN_NUMBERS[p], OUTPUT);
    digitalWrite(PIN_NUMBERS[p], LOW);
  }

}

// ...................................................................................................

void loop() {
  
  // Step through each pin number, turn it on for a while, then off. Repeat for every pin
  int curr_pin;
  for (int p = 0; p < num_pins; p++) {
    curr_pin = PIN_NUMBERS[p];
    digitalWrite(curr_pin, HIGH);
    delay(ON_TIME_MS);
    digitalWrite(curr_pin, LOW);
  }

}
