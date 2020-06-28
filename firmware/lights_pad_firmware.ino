#include <Adafruit_NeoPixel.h>
#include <SPI.h>

// Pins
const int BUTTON_PIN_0 = 2;         // Pin for button 0. Has to be an interrupt pin
const int NEOPIXEL_PIN = 6;        // neopixel pin

// buttons defs
const int NUM_BUTTONS = 1;          // Number of buttons
int button_pins[] = {BUTTON_PIN_0};

// Neopixel configuration
const int NEOPIXEL_BRIGHTNESS_LOW = 20;
const int NEOPIXEL_BRIGHTNESS_HIGH = 100;
int neopixel_brightness = NEOPIXEL_BRIGHTNESS_HIGH;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_BUTTONS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// States
const int STATE_INIT = 0;   //
const int STATE_GAME1 = 1;  // Game 1 - pressing the buttons change colors
const int STATE_SLEEP = 9;  // Sleep. LEDs are off, low power usage stuff
int state = STATE_INIT;
unsigned long lastUpdate = 0; // last update for timing

const long STATE_INIT_TIMEOUT_MS = 1000; // how long to stay in init state
const long INACTIVITY_THRESHOLD_MS = 1000 * 10; // how long before going to sleep

void setup() {
  //while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
  Serial.begin(9600);

  // Init colors
  init_button_color_numbers();

  // initialize the pushbutton pin as an input, and turning up the internal
  // pullup pin
  for (int i = 0; i < NUM_BUTTONS ; i++) {
    pinMode(button_pins[i],  INPUT_PULLUP);
  }
  pinMode(LED_BUILTIN,  OUTPUT);

  // Let pins stabalize (needed?)
  //digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  //digitalWrite(LED_BUILTIN, LOW);

  // Attach an interrupt to the ISR vector
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN_0), pin_ISR_0, CHANGE);

  // Init neopixels
  strip.begin();
  strip.clear();
  strip.setBrightness(NEOPIXEL_BRIGHTNESS_LOW);
  strip.show();
  
  //
  lastUpdate = millis();
}

void loop() {
  switch (state) {
    case STATE_INIT:
      state_0_update();
      break;
    case STATE_GAME1:
      state_1_update();
      break;
    case STATE_SLEEP:
      state_9_update();
      break;

      //default:
  }
}



// ----------------------------------------------------------- //
// Colors and color control
const uint16_t NUM_COLORS = 6;                          // How many colors?
const uint16_t WHEEL_POS[] = {0, 85, 170, 42, 127, 212}; // List of colors (as Wheel positions..)
const uint32_t COLOR_BLACK = strip.Color(0, 0, 0);
const uint32_t COLOR_WHITE = strip.Color(255, 255, 255);

/*
   0 - red
   85 - green
   170 - blue
   127 - light blue
   212 - purple
   42 - yello
*/

int8_t button_color_numbers[NUM_BUTTONS]; // current color number of every button

void init_button_color_numbers() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    button_color_numbers[i] = -1;
  }
}

uint16_t get_button_color_number(int button_id) {
  return button_color_numbers[button_id];
}

uint32_t get_button_color(int button_id) {
  int8_t color_number = get_button_color_number(button_id);
  if (color_number == -1) {
    return COLOR_BLACK;
  } else {
    uint16_t wheel_pos = WHEEL_POS[color_number];
    uint32_t  c = Wheel(wheel_pos);
    return (c);
  }
}

/**
   changes the color of a button to the next color
*/
void bump_button_color(int button_id) {
  button_color_numbers[button_id] = (button_color_numbers[button_id] + 1) % NUM_COLORS;
}


// Source - Adafruit's example sketches
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// ----------------------------------------------------------- //
// Activity tracking
unsigned long last_activity = 0;

/**
   Sets last activity to now
*/
void set_last_activity() {
  last_activity = millis();
}

unsigned long get_last_activity() {
  return last_activity;
}

// ----------------------------------------------------------- //
// States
/**
   This state is the initial state of the toy. It shows that the toy
   is on, and allows to change some settings
*/
void state_0_update() {
  // check to see if it's time to change the state of the LED
  unsigned long currentMillis = millis();

  //End of state init?
  if ((currentMillis - lastUpdate) > STATE_INIT_TIMEOUT_MS) {
    Serial.println("Exiting STATE_INIT");
    state = STATE_GAME1;

    // Turn off LEDs, set working brightness
    strip.clear();
    strip.setBrightness(neopixel_brightness);
    strip.show();

    lastUpdate = millis();
    set_last_activity();
    return;
  }

  // Change brightness if button is pressed
  if (is_button_pressed(1)) {
    neopixel_brightness = NEOPIXEL_BRIGHTNESS_LOW;
  }

  // Turn on LEDs. Color depends on mode
  uint32_t c;
  switch (neopixel_brightness) {
    case NEOPIXEL_BRIGHTNESS_HIGH:
      c = strip.Color(255, 0, 0);
      break;
    case NEOPIXEL_BRIGHTNESS_LOW:
      c = strip.Color(0, 255, 0);
      break;
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();

  //
}

/**
   This is game mode
*/
void state_1_update() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (is_new_press(i)) {
      Serial.println("button! (in loop)");
      bump_button_color(i);
      strip.setPixelColor(i, get_button_color(i));
      strip.show();
    }

    //if (is_button_pressed(i)) {
      //strip.setPixelColor(i, COLOR_WHITE);
      //strip.setPixelColor(i, COLOR_BLACK);
      //strip.setPixelColor(i, get_button_color(i));
    //} else {
    //  strip.setPixelColor(i, get_button_color(i));
    //}

  }


  //Go to sleep?
  unsigned long currentMillis = millis();
  if ((currentMillis - get_last_activity()) > INACTIVITY_THRESHOLD_MS) {
    state = STATE_SLEEP;
  }
}

/**
   This is sleep mode
*/
void state_9_update() {
  // Turn off LEDs
  strip.clear();
  strip.show();

  // Wake up?
  unsigned long currentMillis = millis();
  if ((currentMillis - get_last_activity()) <= INACTIVITY_THRESHOLD_MS) {
    state = STATE_GAME1;
  }
}


// ----------------------------------------------------------- //
// button debounce code

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
const boolean BUTTON_PRESSED = LOW;  // Set to LOW if using internal pullup resistors

typedef struct
{
  int button_read = HIGH;     // the current reading from the input pin
  int button_state;           // the current state of the button (after debouncing)
  int last_button_state = HIGH;// the previous statue of the button
  unsigned long last_debounce_time = 0;  // the last time the output pin was toggled
} button_state_t;

button_state_t button_states[NUM_BUTTONS]; // buttons debouncing internal states

/**
   Code for debouncing button
*/
boolean debounce_button(int button_id) {
  // reading
  button_states[button_id].button_read = digitalRead(button_pins[button_id]);

  // If the switch changed, due to noise or pressing:
  if (button_states[button_id].button_read != button_states[button_id].last_button_state) {
    // reset the debouncing timer
    button_states[button_id].last_debounce_time = millis();
  }

  if ((millis() - button_states[button_id].last_debounce_time) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (button_states[button_id].button_read != button_states[button_id].button_state) {
      button_states[button_id].button_state = button_states[button_id].button_read;
    }
  }

  button_states[button_id].last_button_state = button_states[button_id].button_read;
  return button_states[button_id].button_state;
}

// ----------------------------------------------------------- //
// button press tracking. I use this code to track when the button
// was pressed.
boolean buttons_last_status[NUM_BUTTONS] = {false};

/**
   Checks the current status (pressed or not), returns true
   if the button is pressed, and it's a new press event
*/
boolean is_new_press(int button_id) {
  boolean is_pressed_now = is_button_pressed(button_id);
  boolean ret_val = false;
  if (is_pressed_now && !buttons_last_status[button_id]) {
    ret_val = true;
  }

  buttons_last_status[button_id] = is_pressed_now;
  return ret_val;
}

/**
   Is the button in pressed state
*/
boolean is_button_pressed(int button_id) {
  if (debounce_button(button_id) == BUTTON_PRESSED) {
    return true;
  } else {
    return false;
  }
}

// ----------------------------------------------------------- //
// Interrupt functions
void pin_ISR_ACCEL() {
  Serial.println("accel int!");
}


void pin_ISR_0() {
  Serial.println("button0 int!");
  set_last_activity();
}

void pin_ISR_1() {
  Serial.println("button1 int!");
  set_last_activity();
}

void pin_ISR_2() {
  Serial.println("button2 int!");
  set_last_activity();
}
