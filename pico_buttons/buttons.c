#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LED_PIN 25
#define BUTTON1 12
#define BUTTON2 13
#define LED1 14
#define LED2 15

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

  while (1) {
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    gpio_put(LED_PIN, 1);
    puts("Hello World\n");
    sleep_ms(1000);
  }
}
