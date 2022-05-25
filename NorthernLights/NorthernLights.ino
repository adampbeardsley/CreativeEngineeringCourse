#include <avr/io.h>         /* Defines pins, ports, etc */
// See pinout at https://docs.arduino.cc/hardware/micro
#include <Adafruit_NeoPixel.h>

#define LED0 5
#define LED1 6
#define LED2 7
#define LEDS 9  // LEDs on pin 9
#define NUMPIXELS 50
#define BUTTON1 2  // color button
#define BUTTON2 3  // pattern button
#define DEBOUNCE_TIME 5
#define NCOLOR_MODES 3

Adafruit_NeoPixel pixels(NUMPIXELS, LEDS, NEO_GRB + NEO_KHZ800);

uint8_t color_mode = 0;  // White, RG, N. Lights
uint8_t pattern = 0;  // Solid, Fading, (Music)
int colors[NCOLOR_MODES][2][3] = {
  {{255, 255, 255}, {255, 255, 255}}, // White
  {{255, 0, 0}, {0, 255, 0}}, // Green/Red
  {{255, 0, 0}, {255, 0, 255}} // Green/Yellow
};

uint8_t debouncePress(int button){
  if (!digitalRead(button)) {
    _delay_ms(DEBOUNCE_TIME);
    if (!digitalRead(button)) {
      return (1);
    }
  }
  return 0;
}

void color_interrupt(void){
  // This will change color mode
  // For now, toggle light
  if (debouncePress(BUTTON1)){
    // Update color
    color_mode = (color_mode + 1) % NCOLOR_MODES;
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
  pinMode(BUTTON1, INPUT_PULLUP); // Use pullup - less wiring
  pinMode(BUTTON2, INPUT_PULLUP);

  // Set up interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON1), color_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), mode_interrupt, FALLING);

  // Initialize the lights. Otherwise they start in random state
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pixels.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  pixels.clear();
  for (int i=0; i<NUMPIXELS; i++){
    int j = i % 2;
    pixels.setPixelColor(i, pixels.Color(colors[color_mode][j][0],
                                         colors[color_mode][j][1],
                                         colors[color_mode][j][2]));
    delay(20);
    pixels.show();
  }
}
