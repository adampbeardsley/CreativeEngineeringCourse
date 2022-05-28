#include <avr/io.h>         /* Defines pins, ports, etc */
// See pinout at https://docs.arduino.cc/hardware/micro
#include <Adafruit_NeoPixel.h>

#define LED0 5
#define LED1 6
#define LED2 7
#define LEDS 9  // LEDs on pin 9
#define NUMPIXELS 50
#define NPIX_USE 7  // number of pixels to light up
#define BUTTON1 2  // color button
#define BUTTON2 3  // pattern button
#define DEBOUNCE_TIME 5
#define NCOLOR_MODES 4
#define NPATTERNS 2
#define FADE_PERIOD 5000  // ms for fading cycle

Adafruit_NeoPixel pixels(NUMPIXELS, LEDS, NEO_GRB + NEO_KHZ800);

uint8_t color_mode = 2;  // White, RG, N. Lights
uint8_t pattern = 1;  // Solid, Fading, (Music)
uint8_t pick_new = 1;  // pick new freq, etc for pattern
float amp[3];
float freq[3];
float phase[3];
int colors[NCOLOR_MODES][3][3] = {
  {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}}, // White
  {{255, 0, 0}, {0, 255, 0}, {255, 0, 0}}, // Green/Red
  {{255, 255, 0}, {255, 100, 0}, {100, 0, 255}}, // J's Northern lights
  {{255, 255, 0}, {255, 125, 0}, {0, 255, 255}} // A's Northern lights
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
  if (debouncePress(BUTTON1)){
    // Update color
    color_mode = (color_mode + 1) % NCOLOR_MODES;
  }
}

void pattern_interrupt(void){
  if (debouncePress(BUTTON2)){
    // Update pattern
    pattern = (pattern + 1) % NPATTERNS;
  }
}

uint32_t color_mix(float x){
  float val[3];
  for (int i=0; i<3; i++){
    float k1 = (colors[color_mode][2][i] - colors[color_mode][0][i]) / 2;
    float k2 = (colors[color_mode][0][i] + colors[color_mode][2][i]) / 2 - colors[color_mode][1][i];
    float k4 = colors[color_mode][1][i];
    val[i] = k1 * x * x * x + k2 * x * x + k4;
  }
  return pixels.Color(val[0], val[1], val[2]);
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
  attachInterrupt(digitalPinToInterrupt(BUTTON2), pattern_interrupt, FALLING);

  // Initialize the lights. Otherwise they start in random state
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);

  pixels.begin();
  randomSeed(analogRead(0));
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (pattern) {
    case 0:
      // Solid color, every other light
      for (int i=0; i<NPIX_USE; i++){
        int j = i % 3;
        pixels.setPixelColor(i, pixels.Color(colors[color_mode][j][0],
                                             colors[color_mode][j][1],
                                             colors[color_mode][j][2]));
        //delay(20);
        pixels.show();
      }
      break;
    case 1:
      // Fade between colors
      if (pick_new){
        for (int i=0; i<3; i++){
          amp[i] = random(25 * NPIX_USE, 50 * NPIX_USE) / 200.0;
          freq[i] = float(random(20 * TWO_PI, 100 * TWO_PI)) / 100000.0;
          phase[i] = random(0, 100 * TWO_PI) / 100.0;
        }
//        amp0 = NPIX_USE / 2.0;
//        freq0 = PI / 4000.0;
//        phase0 = 0;
        pick_new = 0;
      }
      float loc = 0;
      unsigned long myTime = millis();
      for (int i=0; i<3; i++){
        loc += amp[i] * sin(freq[i] * myTime + phase[i]) + NPIX_USE / 2;
      }
      for (int i=0; i<NPIX_USE; i++){
        float x = 1.0 / (abs(i - loc) / 3.0 + 1);
        pixels.setPixelColor(i, color_mix(x));
      }
      pixels.show();
        
  }
}
