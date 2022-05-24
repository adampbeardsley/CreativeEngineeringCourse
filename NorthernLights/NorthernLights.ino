#include <avr/io.h>         /* Defines pins, ports, etc */
// See pinout at https://docs.arduino.cc/hardware/micro

#define LED0 4
#define LED1 5
#define LED2 6
#define BUTTON0 7
#define BUTTON1 2
#define BUTTON2 3
#define DEBOUNCE_TIME 5

uint8_t buttonPressed0 = 0;
uint8_t buttonPressed1 = 0;
uint8_t buttonPressed2 = 0;

uint8_t debouncePress(int button){
  if (!digitalRead(button)) {
    _delay_ms(DEBOUNCE_TIME);
    if (!digitalRead(button)) {
      return (1);
    }
  }
  return 0;
}

void onoff_interrupt(void){
  // This will turn system on/off
  // For now, toggle light
  if (debouncePress(BUTTON0)){
    digitalWrite(LED0, !digitalRead(LED0));
  }
}

void color_interrupt(void){
  // This will change color mode
  // For now, toggle light
  if (debouncePress(BUTTON1)){
    digitalWrite(LED1, !digitalRead(LED1));
  }
}

void mode_interrupt(void){
  // This will change mode
  // for now, toggle light
  if (debouncePress(BUTTON2)){
    digitalWrite(LED2, !digitalRead(LED2));
  }
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
  attachInterrupt(digitalPinToInterrupt(BUTTON0), onoff_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON1), color_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), mode_interrupt, FALLING);

  // Initialize the lights. Otherwise they start in random state
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
}
