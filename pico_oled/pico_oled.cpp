#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "../../pico-ssd1306/ssd1306.h"
#include "../../pico-ssd1306/textRenderer/TextRenderer.h"
#include "hardware/i2c.h"

#define LED_PIN 25
#define I2C_SDA 4
#define I2C_SCL 5

using namespace pico_ssd1306;

int main() {
  stdio_init_all();
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  i2c_init(i2c0, 1000000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  sleep_ms(250);
  SSD1306 display = SSD1306(i2c0, 0x3D, Size::W64xH48);

  // for (int y = 0; y < 5; y++){
  //   display.setPixel(0, y);
  // }
  drawText(&display, font_5x8, "TEST text", 0 ,20);

  display.sendBuffer();
  while (1) {
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    gpio_put(LED_PIN, 1);
    puts("Hello World\n");
    sleep_ms(1000);
  }
}
