#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#define LED_PIN 25
#define BUTTON1 12
#define BUTTON2 13
#define LED1 14
#define LED2 15
#define DEBOUNCE_TIME 50

unsigned long time1 = to_ms_since_boot(get_absolute_time());
unsigned long time2 = to_ms_since_boot(get_absolute_time());
bool state1;
bool state2;

void button_update(uint gpio, uint32_t events) {
  if (gpio == BUTTON1){
    if ((to_ms_since_boot(get_absolute_time()) - time1) > DEBOUNCE_TIME) {
      time1 = to_ms_since_boot(get_absolute_time());
      state1 = !state1;
      gpio_put(LED1, state1);
    }
  } else {
    if ((to_ms_since_boot(get_absolute_time()) - time2) > DEBOUNCE_TIME) {
      time2 = to_ms_since_boot(get_absolute_time());
      state2 = !state2;
      gpio_put(LED2, state2);
    }
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

  while (1) {

    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    gpio_put(LED_PIN, 1);
    puts("Hello World\n");
    sleep_ms(1000);
  }
}
