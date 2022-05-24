#include <avr/io.h>         /* Defines pins, ports, etc */
// See pinout at https://docs.arduino.cc/hardware/micro

#define LED0 4
#define LED1 5
#define LED2 6
#define BUTTON0 7
#define BUTTON1 2
#define BUTTON2 3 

void onoff_interrupt(void){
  // This will turn system on/off
  // For now, toggle light
  digitalWrite(LED0, digitalRead(BUTTON0));
}

void color_interrupt(void){
  // This will change color mode
  // For now, toggle light
  digitalWrite(LED1, digitalRead(BUTTON1));
}

void mode_interrupt(void){
  // This will change mode
  // for now, toggle light
  digitalWrite(LED2, digitalRead(BUTTON2));
}

void setup() {
  // Pin directions
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON0, INPUT_PULLUP); // Use pullup - less wiring
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  // Set up interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON0), onoff_interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON1), color_interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), mode_interrupt, CHANGE);

  // Initialize the lights. Otherwise they start in random state
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
}
