#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include <stdio.h>

ssd1306_t oled;

#define BUZZER_PIN 21
#define JOYSTICK_Y 27  
#define BUTTON_A 5

#define I2C_SDA 14
#define I2C_SCL 15

repeating_timer_t timer_30_min;
repeating_timer_t timer_2_horas;
bool alarm_active = false;
bool system_started = false;
absolute_time_t buzzer_start_time;
bool buzzer_on = false;

bool alerta_30_min(repeating_timer_t *t) {
    alarm_active = true; 
    return true;
}

bool alerta_2_horas(repeating_timer_t *t) {
    alarm_active = true;
    return true;
}

void process_alarm() {
    if (alarm_active && !buzzer_on) {
        buzzer_start_time = get_absolute_time();
        gpio_put(BUZZER_PIN, 1);
        buzzer_on = true;
    }

    if (buzzer_on && absolute_time_diff_us(buzzer_start_time, get_absolute_time()) > 1000000) {
        gpio_put(BUZZER_PIN, 0);
        buzzer_on = false;
    }
}

void display_start_screen(ssd1306_t *oled) {
    ssd1306_clear(oled);
    ssd1306_draw_string(oled, 20, 25, 1, "Pressione A");
    ssd1306_draw_string(oled, 35, 40, 1, "para iniciar");
    ssd1306_show(oled);
}

void display_menu(ssd1306_t *oled, int selected_option) {
    ssd1306_clear(oled);
    ssd1306_draw_string(oled, 10, 10, 1, selected_option == 0 ? "> Parar Alarme" : "  Parar Alarme");
    ssd1306_draw_string(oled, 10, 20, 1, selected_option == 1 ? "> Reiniciar Timer" : "  Reiniciar Timer");
    ssd1306_show(oled);
}

void menu_interativo(ssd1306_t *oled) {
    int option = 0;
    
    while (alarm_active) { 
        display_menu(oled, option);

        uint16_t joystick_val = adc_read();

        if (joystick_val < 1000) { 
            option = (option - 1 + 2) % 2;
            sleep_ms(200);
        } 
        else if (joystick_val > 3000) { 
            option = (option + 1) % 2;
            sleep_ms(200);
        }

        if (gpio_get(BUTTON_A) == 0) {  
            if (option == 0) { 
                gpio_put(BUZZER_PIN, 0); 
                alarm_active = false;
                buzzer_on = false;
            }
            else if (option == 1) { 
                add_repeating_timer_ms(1800000, alerta_30_min, NULL, &timer_30_min);
                add_repeating_timer_ms(7200000, alerta_2_horas, NULL, &timer_2_horas);
                alarm_active = false;
                buzzer_on = false;
            }
            sleep_ms(500);
        }
        sleep_ms(100);
    }
}

void wait_for_start() {
    display_start_screen(&oled);
    
    while (!system_started) {
        printf("botao pressionado");
        if (gpio_get(BUTTON_A) == 0) {  
            system_started = true;
            sleep_ms(500);
        }
        sleep_ms(100);
    }
}

int main() {
    stdio_init_all();

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    adc_init();
    adc_gpio_init(JOYSTICK_Y);
    adc_select_input(1);

    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    if (!ssd1306_init(&oled, 128, 64, 0x3C, i2c1)) {
        printf("Erro ao inicializar o OLED!\n");
        return 1;
    }

    // Aguarda o usuário pressionar "A" para começar
    wait_for_start();

    printf("passou daqui");

    // Inicia os timers apenas após o usuário iniciar
    add_repeating_timer_ms(1800000, alerta_30_min, NULL, &timer_30_min);
    add_repeating_timer_ms(7200000, alerta_2_horas, NULL, &timer_2_horas);

    while (1) {
        menu_interativo(&oled);
        process_alarm();
        sleep_ms(100);
    }

    return 0;
}
