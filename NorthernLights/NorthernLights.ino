#include <avr/io.h>         /* Defines pins, ports, etc */
// See pinout at https://docs.arduino.cc/hardware/micro

#define LED1 4
#define LED2 5
#define LED3 6
#define BUTTON1 8
#define BUTTON2 9

ISR(PCINT0_vect){
  digitalWrite(LED1, digitalRead(BUTTON1));
  digitalWrite(LED2, digitalRead(BUTTON2));
}

void setup() {
  // Pin directions
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP); // Use pullup - less wiring
  pinMode(BUTTON2, INPUT_PULLUP);

  PCICR |= (1 << PCIE0);
  PCMSK0 |= ((1 << PCINT4) | (1 << PCINT5));
  sei();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED3, HIGH);
  delay(500);
  digitalWrite(LED3, LOW);
  delay(500);
}
