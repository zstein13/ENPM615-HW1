#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"


#define BUTTON_PIN 15

void toggle_led() {
    bool LED_STATE = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !LED_STATE);
}


int main()
{
    stdio_init_all();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Example to turn on the Pico W LED
    

    while (true) {
        if (!gpio_get(BUTTON_PIN)) {
            printf("Button pressed\n");
            toggle_led();
        }
        sleep_ms(250);
    }
}
