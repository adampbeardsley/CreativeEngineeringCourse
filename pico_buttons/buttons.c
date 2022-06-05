#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25
#define BUTTON1 12
#define BUTTON2 13
#define LED1 14
#define LED2 15
#define DEBOUNCE_TIME 5000000  // roughly number of clock cycles

uint8_t button1Pressed = 0;
uint8_t button2Pressed = 0;

uint8_t debouncePress(int button){
  int i = 0;
  if (!gpio_get(button)) {
    while (i < DEBOUNCE_TIME){ i++; }
    if (!gpio_get(button)){
      return 1;
    }
  }
  return 0;
}

void button_update(){

  if (debouncePress(BUTTON1)){
    if (button1Pressed == 0) {
      button1Pressed = 1;
      gpio_put(LED1, !gpio_get(LED1));
    }
  } else {
    button1Pressed = 0;
  }
  if (debouncePress(BUTTON2)){
    if (button2Pressed == 0) {
      button2Pressed = 1;
      gpio_put(LED2, !gpio_get(LED2));
    }
  } else {
    button2Pressed = 0;
  }
}

void button_clear(){
  if (gpio_get(BUTTON1)){
    button1Pressed = 0;
  }
  if (gpio_get(BUTTON2)){
    button2Pressed = 0;
  }
}

int main() {

  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_init(BUTTON1);
  gpio_init(BUTTON2);
  gpio_init(LED1);
  gpio_init(LED2);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_set_dir(BUTTON1, GPIO_IN);
  gpio_pull_up(BUTTON1);
  gpio_set_dir(BUTTON2, GPIO_IN);
  gpio_pull_up(BUTTON2);
  gpio_set_dir(LED1, GPIO_OUT);
  gpio_set_dir(LED2, GPIO_OUT);

  gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, &button_update);
  gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_FALL, true, &button_update);
  gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_RISE, true, &button_clear);
  gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_RISE, true, &button_clear);

  while (1) {

    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    gpio_put(LED_PIN, 1);
    puts("Hello World\n");
    sleep_ms(1000);
  }
}
