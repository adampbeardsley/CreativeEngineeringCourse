#include <stdio.h>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

const uint LED_PIN = 25;
static volatile bool fired = false;

static void LED_flash(void){
  fired = true;
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];

    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
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
    rtc_set_alarm(&t0, &LED_flash);

    // Print the time
    while (true) {
      if (fired){
        printf("ALARM!\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        fired = false;
      }
      rtc_get_datetime(&t);
      datetime_to_str(datetime_str, sizeof(datetime_buf), &t);
      printf("\r%s      \n", datetime_str);
      sleep_ms(1000);
    }

    return 0;
}
