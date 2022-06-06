#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/rtc.h"
#include "pico_ws2811.pio.h"
#include "pico/util/datetime.h"
#include "../../pico-ssd1306/ssd1306.h"
#include "../../pico-ssd1306/textRenderer/TextRenderer.h"
#include "../../pico-ssd1306/shapeRenderer/ShapeRenderer.h"
#include "hardware/i2c.h"

#define LED_PIN 25

#define NUM_PIXELS 15
#define RESET_TIME_US 400
#define START_PIN 1

#define I2C_SDA 4
#define I2C_SCL 5
#define TIMEX 20
#define TIMEY 10

int dma_chan;
uint32_t pixels[NUM_PIXELS*24];

static volatile bool alarm_fired = false;

using namespace pico_ssd1306;

void set_pixel_colour(int pixel, uint8_t r, uint8_t g, uint8_t b) {

	uint32_t colour_value = (r << 16 | g << 8 | b);

	for(int i=0; i<24;i++) {
		if (colour_value & (1u<<i)) {
			pixels[((pixel + 1)*24) - i - 1] |= 1u;
		}
		else {
			pixels[((pixel + 1)*24) - i - 1] &= ~(1u);
		}
	}
}

int64_t dma_start(alarm_id_t id, void *user_data) {
	dma_channel_set_read_addr(dma_chan, pixels, true);
	return 0;
}

void dma_handler() {
	dma_irqn_acknowledge_channel(DMA_IRQ_0,dma_chan);
	//need a pause to re-set the LED strip before we can start again.
	//can't use sleep in irq handlers as weird things happen
	add_alarm_in_us(RESET_TIME_US, dma_start, NULL, false);
}

void init_ws2811(){
	PIO pio = pio0;
	int sm = 0;

	uint offset = pio_add_program(pio, &ws2811_program);

  ws2811_program_init(pio, sm, offset, START_PIN, 800000);
	dma_chan = dma_claim_unused_channel(true);
  dma_channel_config c = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
	channel_config_set_read_increment(&c, true);
	//See datasheet 2.5.3.1
	channel_config_set_dreq(&c, DREQ_PIO0_TX0);
	dma_channel_configure(
        dma_chan,
        &c,
        &pio0_hw->txf[0], // Write address (only need to set this once)
        NULL,             // Don't provide a read address yet
        NUM_PIXELS*24, 	  // number of transfers
        false             // Don't start yet
    );

	// Tell the DMA to raise IRQ line 0 when the channel finishes a block
  dma_channel_set_irq0_enabled(dma_chan, true);
  irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
  irq_set_enabled(DMA_IRQ_0, true);
	//kick off the initial transfer
	dma_handler();
}

static void alarm_isr(void){
	alarm_fired = true;
}

SSD1306 setup_display(){
	i2c_init(i2c0, 1000000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  SSD1306 display = SSD1306(i2c0, 0x3C, Size::W128xH32);
  display.setOrientation(0);
	return display;
}

void update_time(SSD1306 display, int8_t hour, int8_t minute){
	char time_str[5];
	sprintf(time_str, "%02d:%02d", hour, minute);
	fillRect(&display, TIMEX, TIMEY, TIMEX + 5 * 12, TIMEY + 16, WriteMode::SUBTRACT);
	drawText(&display, font_12x16, time_str, TIMEX, TIMEY);
	datetime_t t = {
					.year  = 2020,
					.month = 06,
					.day   = 05,
					.dotw  = 5,
					.hour  = hour,
					.min   = minute,
					.sec   = 00
	};
	rtc_set_datetime(&t);
	display.sendBuffer();
}

void update_alarm(SSD1306 display, int8_t hour, int8_t minute){
	char time_str[5];
	sprintf(time_str, "%02d:%02d", hour, minute);
	fillRect(&display, TIMEX, TIMEY, TIMEX + 5 * 12, TIMEY + 16, WriteMode::SUBTRACT);
	drawText(&display, font_12x16, time_str, TIMEX, TIMEY);
	datetime_t t = {
					.year  = 2020,
					.month = 06,
					.day   = 05,
					.dotw  = 5,
					.hour  = hour,
					.min   = minute,
					.sec   = 00
	};
	rtc_set_datetime(&t);
	display.sendBuffer();
}

int main() {

	stdio_init_all();
	init_ws2811();

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 0);

	SSD1306 display = setup_display();
	drawText(&display, font_12x16, "00:00", TIMEX, TIMEY);
  drawText(&display, font_5x8, "00:00", 100, 18);
  drawText(&display, font_5x8, "ALARM", 100, 8);
  display.sendBuffer();

	char datetime_buf[256];
	char *datetime_str = &datetime_buf[0];

	// Start on Friday 5th of June 2020 15:45:00
	datetime_t t = {
					.year  = -1,
					.month = -1,
					.day   = -1,
					.dotw  = -1, // 0 is Sunday, so 5 is Friday
					.hour  = 15,
					.min   = 45,
					.sec   = 00
	};

	// Start the RTC
	rtc_init();
	rtc_set_datetime(&t);

	datetime_t t0 = {
					.year  = -1,
					.month = -1,
					.day   = -1,
					.dotw  = -1, // 0 is Sunday, so 5 is Friday
					.hour  = 15,
					.min   = 45,
					.sec   = 10
	};
	rtc_set_alarm(&t0, &alarm_isr);

	update_time(display, 3, 14);

	while (true) {
		for (uint8_t j=0; j<255; j++){
			for (int i=0; i<NUM_PIXELS;i++) {
					set_pixel_colour(i,j,0,0);
				}
			if (alarm_fired){
        printf("ALARM!\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        alarm_fired = false;
      }
			sleep_ms(100);
		}
		for (uint8_t j=255; j>0; j--){
			for (int i=0; i<NUM_PIXELS;i++) {
					set_pixel_colour(i,j,0,0);
				}
			sleep_ms(100);
		}
  }
}
