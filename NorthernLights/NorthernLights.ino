#include <avr/io.h>         /* Defines pins, ports, etc */
// See pinout at https://docs.arduino.cc/hardware/micro
#include <Adafruit_NeoPixel.h>

#define LEDS 9  // LEDs on pin 9
#define NUMPIXELS 100
#define NPIX_USE 79  // number of pixels to light up
#define BUTTON1 2  // color button
#define BUTTON2 3  // pattern button
#define DEBOUNCE_TIME 5
#define NCOLOR_MODES 5
#define NPATTERNS 2

Adafruit_NeoPixel pixels(NUMPIXELS, LEDS, NEO_GRB + NEO_KHZ800);

uint8_t color_mode = 4;  // White, RG, N. Lights
uint8_t pattern = 1;  // Solid, Fading, (Music)
float freq[4] = {.05, -.065, .065 * PI, -0.05*PI};  // Used jupyter to find decent values
float wlengths[4] = {1.3, 1.7, 1.3, 1.7};  // Used jupyter to find decent values
int colors[NCOLOR_MODES][3][3] = {
  {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}}, // White
  {{255, 0, 0}, {0, 255, 0}, {255, 0, 0}}, // Green/Red
  {{255, 255, 0}, {100, 255, 0}, {0, 100, 255}}, // J's Northern lights
  {{255, 255, 0}, {255, 170, 0}, {255, 0, 255}}, // Autumn with purple
  {{255, 170, 0}, {255, 110, 0}, {255, 0, 0}} // Autumn with red
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
  pinMode(BUTTON1, INPUT_PULLUP); // Use pullup - less wiring
  pinMode(BUTTON2, INPUT_PULLUP);

  // Set up interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON1), color_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), pattern_interrupt, FALLING);

  pixels.begin();
  randomSeed(analogRead(0));

  Serial.begin(9600);
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
      float myTime = millis() / 1000.0;
      
      for (int i=0; i<NPIX_USE; i++){
        float pos = i / 79.0;
        float x = sin(2 * PI * (pos / wlengths[0] - myTime * freq[0]));
        x += sin(2 * PI * (pos / wlengths[1] - myTime * freq[1]));
        x += sin(2 * PI * (pos / wlengths[2] - myTime * freq[2]));
        x += sin(2 * PI * (pos / wlengths[3] - myTime * freq[3]));
        x = constrain(x / 3, -1, 1);
        
        pixels.setPixelColor(i, color_mix(x));
      }
      pixels.show();
        
  }
}
