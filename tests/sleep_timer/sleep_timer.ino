// https://github.com/rocketscream/Low-Power
#include "LowPower.h"

volatile int inactivity_sec= 0;
#define BUTTON 2
#define INACTIVITY_MAX_SEC 10

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  cli();//stop interrupts
  
  // put your setup code here, to run once:
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
 
  // send an intro:
  Serial.println("Starting");

  pinMode(2,INPUT_PULLUP);  // internal pull-up resistor
  attachInterrupt (digitalPinToInterrupt (BUTTON), changeEffect, CHANGE); // pressed

}

// Interrupt is called once a second
SIGNAL(TIMER1_COMPA_vect) 
{
  inactivity_sec = min(INACTIVITY_MAX_SEC, inactivity_sec + 1);
  // Sleep
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  if (inactivity_sec >= INACTIVITY_MAX_SEC) {
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  }
} 

void changeEffect() {
  inactivity_sec = 0;
}


void loop() {
  // put your main code here, to run repeatedly:
 Serial.println(inactivity_sec); 
 delay(500);    
}
