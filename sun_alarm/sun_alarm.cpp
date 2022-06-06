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
#include "pico/time.h"

#define LED_PIN 25

#define NUM_PIXELS 15
#define RESET_TIME_US 400
#define START_PIN 1

#define I2C_SDA 4
#define I2C_SCL 5
#define TIMEX 20
#define TIMEY 10
#define ALARMX 100
#define ALARMY 18

#define BUTTON1 14
#define BUTTON2 15
#define DEBOUNCE_TIME 50

#define ALARM_TIME 60 // Time for lights to turn completely on, in seconds
#define ALARM_BUFFER 20 // time for lights to stay completely on, in seconds

int dma_chan;
uint32_t pixels[NUM_PIXELS*24];

static volatile bool alarm_fired = false;

using namespace pico_ssd1306;

unsigned long time1 = to_ms_since_boot(get_absolute_time());
unsigned long time2 = to_ms_since_boot(get_absolute_time());
bool pressed1 = false;
bool pressed2 = false;
uint8_t timeset_state = 0;
datetime_t alarm_dt = {
				.year  = -1,
				.month = -1,
				.day   = -1,
				.dotw  = -1,
				.hour  = 0,
				.min   = 0,
				.sec   = 00
};
datetime_t time_dt = {
				.year  = 2022,
				.month = 6,
				.day   = 6,
				.dotw  = 1,
				.hour  = 0,
				.min   = 0,
				.sec   = 00
};

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

void update_time(SSD1306 display, int8_t hour, int8_t minute, bool set_alarm=false){
	char time_str[5];
	sprintf(time_str, "%02d:%02d", hour, minute);
	if (set_alarm){
		alarm_dt.hour = hour;
		alarm_dt.min = minute;
		fillRect(&display, ALARMX, ALARMY, ALARMX + 5 * 5, ALARMY + 8, WriteMode::SUBTRACT);
		drawText(&display, font_5x8, time_str, ALARMX, ALARMY);
		rtc_set_alarm(&alarm_dt, &alarm_isr);
	} else {
		time_dt.hour = hour;
		time_dt.min = minute;
		fillRect(&display, TIMEX, TIMEY, TIMEX + 5 * 12, TIMEY + 16, WriteMode::SUBTRACT);
		drawText(&display, font_12x16, time_str, TIMEX, TIMEY);
		rtc_set_datetime(&time_dt);
	}
	display.sendBuffer();
	alarm_fired = false;  // If changing time/alarm, old alarm is no good.
}

void update_alarm(SSD1306 display, int8_t hour, int8_t minute){
	update_time(display, hour, minute, true);
}

void refresh_time(SSD1306 display){
	char time_str[5];

	rtc_get_datetime(&time_dt);
	sprintf(time_str, "%02d:%02d", time_dt.hour, time_dt.min);
	fillRect(&display, TIMEX, TIMEY, TIMEX + 5 * 12, TIMEY + 16, WriteMode::SUBTRACT);
	drawText(&display, font_12x16, time_str, TIMEX, TIMEY);
	display.sendBuffer();
}

void button_update(uint gpio, uint32_t events) {
  if (gpio == BUTTON1){
    if ((to_ms_since_boot(get_absolute_time()) - time1) > DEBOUNCE_TIME) {
      pressed1 = true;
    }
  } else {
    if ((to_ms_since_boot(get_absolute_time()) - time2) > DEBOUNCE_TIME) {
      pressed2 = true;
    }
  }
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

	rtc_init();

	update_time(display, 3, 14);
	update_alarm(display, 3, 15);

	gpio_init(BUTTON1);
  gpio_init(BUTTON2);
  gpio_set_dir(BUTTON1, GPIO_IN);
  gpio_pull_up(BUTTON1);
  gpio_set_dir(BUTTON2, GPIO_IN);
  gpio_pull_up(BUTTON2);

  gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, &button_update);
  gpio_set_irq_enabled_with_callback(BUTTON2, GPIO_IRQ_EDGE_FALL, true, &button_update);

	while (true) {

		// State machine for button presses
		switch (timeset_state) {
			case 0:
				// Not setting time or alarm, but check for button press
				if (pressed1){
					// Move to next mode
					timeset_state = 1;
					fillRect(&display, TIMEX, TIMEY + 18, TIMEX + 2 * 12, TIMEY + 19);
					pressed1 = false;
				}
				pressed2 = false;
				break;
			case 1:
				// Setting the hour
				if (pressed2){
					rtc_get_datetime(&time_dt);
					time_dt.hour = (time_dt.hour + 1) % 24;
					rtc_set_datetime(&time_dt);
					// Let's see if the cycle refresh is fast enough
					pressed2 = false;
				}
				if (pressed1){
					// Move to next mode
					timeset_state = 2;
					fillRect(&display, TIMEX, TIMEY + 18, TIMEX + 2 * 12, TIMEY + 19, WriteMode::SUBTRACT);
					fillRect(&display, TIMEX + 3 * 12, TIMEY + 18, TIMEX + 5 * 12, TIMEY + 19);
					pressed1 = false;
				}
				break;
			case 2:
				// Setting the minute
				if (pressed2){
					rtc_get_datetime(&time_dt);
					time_dt.min = (time_dt.min + 1) % 60;
					rtc_set_datetime(&time_dt);
					// Let's see if the cycle refresh is fast enough
					pressed2 = false;
				}
				if (pressed1){
					// Move to next mode
					timeset_state = 3;
					fillRect(&display, TIMEX + 3 * 12, TIMEY + 18, TIMEX + 5 * 12, TIMEY + 19, WriteMode::SUBTRACT);
					fillRect(&display, ALARMX, TIMEY + 18, ALARMX + 2 * 5, TIMEY + 19);
					pressed1 = false;
				}
				break;
			case 3:
				// Setting the alarm hour
				if (pressed2){
					update_alarm(display, (alarm_dt.hour + 1) % 24, alarm_dt.min);
					pressed2 = false;
				}
				if (pressed1){
					// Move to next mode
					timeset_state = 4;
					fillRect(&display, ALARMX, TIMEY + 18, ALARMX + 3 * 5, TIMEY + 19, WriteMode::SUBTRACT);
					fillRect(&display, ALARMX + 3 * 5, TIMEY + 18, ALARMX + 5 * 5, TIMEY + 19);
					pressed1 = false;
				}
				break;
			case 4:
				// Setting the alarm minute
				if (pressed2){
					update_alarm(display, alarm_dt.hour, (alarm_dt.min + 1) % 60);
					pressed2 = false;
				}
				if (pressed1){
					// Move to next mode
					timeset_state = 0;
					fillRect(&display, ALARMX + 3 * 5, TIMEY + 18, ALARMX + 5 * 5, TIMEY + 19, WriteMode::SUBTRACT);
					pressed1 = false;
				}
				break;
		}
		if (alarm_fired){
			rtc_get_datetime(&time_dt);
			float seconds_since_alarm = 3600 * (time_dt.hour - alarm_dt.hour) + 60 * (time_dt.min - alarm_dt.min) + (time_dt.sec - alarm_dt.sec);
			if (seconds_since_alarm > (ALARM_TIME + ALARM_BUFFER)) {
				alarm_fired = false;
				for (int i=0; i<NUM_PIXELS;i++) {
					set_pixel_colour(i,0,0,0);
				}
			} else if (seconds_since_alarm > ALARM_TIME){
				for (int i=0; i<NUM_PIXELS;i++) {
					set_pixel_colour(i, 255, 255, 255);
				}
			} else {
				int bright = seconds_since_alarm / ALARM_TIME * 255;
				for (int i=0; i<NUM_PIXELS;i++) {
					set_pixel_colour(i, bright, bright, bright);
				}
			}
    }
		refresh_time(display);
		sleep_ms(100);
  }
}
