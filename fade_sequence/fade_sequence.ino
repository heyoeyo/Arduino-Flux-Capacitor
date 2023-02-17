
// Arduino PWM pin numbers. Order should match connection order!
const uint8_t PIN_NUMBERS[] = {3,5,6,10,11};

// Number of milliseconds per full cycle of all LEDs
const int CYCLE_PERIOD_MS = 250;

// Hard cut-off for LEDs lighting up. Should be a value between 0.0 and 1.0
// -> Normally the 'led_activation_pattern' function control LED brightness
// -> This threshold can be used to fully turn off LEDs, instead of relying on activation function
const float signal_distance_threshold = 1.0;

// Can swap true/false to flip on/off behavior
// -> Useful in case circuit is wired so that LEDs are 'active low'
const bool LED_ON = true;
const bool LED_OFF = false;

// Enable/disable use of PWM (smoother on/off), need to disable if not using PWM pins!
// -> Doesn't make a big difference for faster cycles/low smoothing
// -> Turning PWM off can be helpful for debugging timing/sequencing
const bool USE_PWM = true;


// ---------------------------------------------------------------------------------------------------

// Used internally, don't change
int num_pins;
float one_over_num_pins;
const unsigned long UPDATE_PERIOD_MS = 5;
unsigned long last_update_ms = 0;


// ...................................................................................................

float led_activation_pattern(float distance) {

  /*
  Function used to control LED brightness, based on how 'close' a signal is to the LED
  -> When the signal is 'at' the LED, the distance value will be 0
  -> When the signal is 'far' from the LED, the distance will be 1
  The output of this function is expected to be a number between 0 and 1,
  where 0 means the LED will be fully off and 1 means fully on.
  */
  
  // Larger 'width' correspond to the LED staying on longer
  // -> Large values give a more blurred light-up pattern across LEDs
  // -> Small values give a quicker blinky pattern
  float width = 0.5;

  // Get value that is 1 when signal is 'closest' and 0 when far away
  float proximity = 1.0 - abs(distance);
  if (proximity > width) {
    return 0.0;
  }

  // Calculate a parabolic signal strength
  // -> scaled so that the output is 1 when signal is close to LED and 0 at cutoff distance
  float shifted = (proximity - width);
  float signal_strength = (shifted * shifted) / (width * width);
  return signal_strength;
}


// ...................................................................................................

void setup() {

  // Figure out how many pins we have
  num_pins = sizeof(PIN_NUMBERS);
  one_over_num_pins = 1.0 / float(num_pins);

  // Make sure each pin is configured as an output & off to begin with
  for (int p = 0; p < num_pins; p++) {
    pinMode(PIN_NUMBERS[p], OUTPUT);
    digitalWrite(PIN_NUMBERS[p], LED_OFF);
  }
  
  return;
}

// ...................................................................................................

void loop() {

  // Get the amount of milliseconds past since device was powered on
  unsigned long time_ms = millis();

  // Don't update pins every loop iteration, since it can cause weird flickering
  bool skip_loop = (time_ms - last_update_ms) < UPDATE_PERIOD_MS;
  if (skip_loop) return;
  last_update_ms = time_ms;

  // Get a normalized (0 to 1) signal, which acts like an angle
  // -> Imagine each LED is evenly spaced around a circle.
  //    We have a point rotating around the circle (at some frequency),
  //    when the point gets 'close' to one of the LEDs, the LED turns on.
  //    We can have the brightness of the LED vary based on how close the rotating
  //    point is, in order to fade the LEDs in and out (if using PWM pins)
  float signal_angle = float(time_ms % CYCLE_PERIOD_MS) / float(CYCLE_PERIOD_MS);

  // Loop over each pin to check if it should turn on/off
  for (int p = 0; p < num_pins; p++) {

    // Grab each pin number & 'angle' of the LED (on our imaginary circle)
    int curr_pin = PIN_NUMBERS[p];
    float pin_angle =  float(p) * one_over_num_pins;
    
    // Figure out how 'close' our signal is to each pin
    // -> want a value that is always between -1.0 and +1.0
    // -> +/- 1.0 implies the signal is on the opposite side of our imaginary circle (i.e. as far away as possible)
    // -> value is positive if signal is passing pin and negative as signal approaches pin
    float raw_distance = signal_angle - pin_angle;
    float shift_down = (raw_distance > 0.5) ? -1 : 0;
    float shift_up = (raw_distance < -0.5) ? 1 : 0;
    float distance = (raw_distance + shift_up + shift_down) * 2.0;
    
    // Turn pin fully off if the signal is 'too far' away
    if (abs(distance) > signal_distance_threshold) {
      digitalWrite(curr_pin, LED_OFF);
      continue;
    }

    // Determine pwm light-up value based on signal-to-pin distance
    float led_strength = led_activation_pattern(distance);
    uint8_t pwm_value = round(255.0 * constrain(led_strength, 0.0, 1.0));
    if (USE_PWM) {
      analogWrite(curr_pin, pwm_value);
    } else {
      // -> If we aren't using PWM, only turn LED on if PWM signal is past 50% (50% of max value 255 is 127.5)
      digitalWrite(curr_pin, (pwm_value > 127) ? LED_ON : LED_OFF);
    }
    
  } // End of for loop

  return;
}
