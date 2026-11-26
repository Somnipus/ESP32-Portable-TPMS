/**
 * @file main.cpp
 * @brief This project is designed to read pressure and temperature from the SMP3011 and BMP280 sensors, respectively, and display the information on an ILI9341 TFT LCD using the LVGL library. It also includes a buzzer alert system for pressure anomalies. Used in the Projeto Integrador III course-
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

// Application modules
#include "hardware.hpp"
#include "smp3011.hpp"
#include "bmp280.hpp"
#include "display.hpp"

#define I2C_PORT_NUM           0
#define I2C_SDA_PIN            GPIO_NUM_5
#define I2C_SCL_PIN            GPIO_NUM_4

static const char *TAG = "MAESTRO";


uint16_t *buf;

// Objects Instances
Hardware* hw;
SMP3011* sensor;
BMP280* ambiente;


// Mutex to protect the I2C bus
SemaphoreHandle_t i2c_mutex;

// Shared variables between tasks
volatile float pressao_atual_psi = 0.0f;
volatile float temperatura_atual_c = 0.0f;

// ---------------- THREAD 1: READING AND ALARM ----------------
void task_leitura_sensor(void *pvParameters) {
    while (1) {
        // Requests exclusive access to I2C once per cycle
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            pressao_atual_psi = sensor->ler_pressao_psi();
            temperatura_atual_c = ambiente->ler_temperatura_celsius();
            xSemaphoreGive(i2c_mutex); // Makes the I2C bus availabe
        }
        
        // Decision of alarms (Independent of I2C)
        bool alerta = (pressao_atual_psi < PRESSAO_MINIMA_PSI || pressao_atual_psi > PRESSAO_MAXIMA_PSI); 
        hw->buzzer_alerta(alerta);

        vTaskDelay(pdMS_TO_TICKS(500)); // Reads every 500ms
    }
}

// ---------------- THREAD 2: GRAPHICAL INTERFACE ----------------
void task_display(void *pvParameters) {
    lvgl_config();
    esp_lcd_panel_handle_t panel = lcd_init();
    buf = lcd_config();
    //lcd_fill(panel, 0xFFFF, buf); // Clenas up the screen with white color
    while (1) {
        desenhar_tela_lvgl();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// -------------- ENTRY POINT --------------
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Sistema (PI III)..."); // Starting System (PI III)...

    // 1. FreeRTOS Mutex Initialization
    i2c_mutex = xSemaphoreCreateMutex();

    // 2. I2C main bus initialization
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_master_bus_config_t bus_config = { 
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT, 
        .glitch_ignore_cnt = 7, // Ignores glitches shorter than 7 cycles
        .flags = { .enable_internal_pullup = true } // Sets internal pull-ups for SDA and SCL lines
        
    };
    bus_config.flags.enable_internal_pullup = true;
    
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    // 3. Objects Dynamic Instantiation
    hw = new Hardware();
    hw->inicializar();
    sensor = new SMP3011(i2c_bus_handle);

    // Setting and configuration of the BMP280 temperature sensor
    ambiente = new BMP280(i2c_bus_handle);
    ambiente->inicializar();

    



    // 4. FreeRTOS tasks creation
    xTaskCreate(task_leitura_sensor, "TaskLeitura", 4096, NULL, 5, NULL);
    xTaskCreate(task_display, "TaskDisplay", 4096, NULL, 5, NULL);
    
}