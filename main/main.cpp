/**
 * @file main.cpp
 * @brief Maestro Orientado a Objetos (PI III).
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

// Módulos da aplicação
#include "hardware.hpp"
#include "smp3011.hpp"
//#include "bmp280.hpp"
#include "display.hpp"

#define I2C_PORT_NUM           0
#define I2C_SDA_PIN            GPIO_NUM_5
#define I2C_SCL_PIN            GPIO_NUM_4

static const char *TAG = "MAESTRO";


uint16_t *buf;

// Instâncias dos Objetos
Hardware* hw;
SMP3011* sensor;
//BMP280* ambiente;
//DisplayOLED* display;

// Mutex para proteger o barramento I2C
SemaphoreHandle_t i2c_mutex;

// Variáveis compartilhada entre as tasks
volatile float pressao_atual_psi = 0.0f;
volatile float temperatura_atual_c = 0.0f;

// ---------------- THREAD 1: LEITURA E ALARME ----------------
void task_leitura_sensor(void *pvParameters) {
    while (1) {
        // Solicita acesso exclusivo ao I2C uma única vez por ciclo
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            pressao_atual_psi = sensor->ler_pressao_psi();
            //temperatura_atual_c = ambiente->ler_temperatura_celsius();
            xSemaphoreGive(i2c_mutex); // Libera o barramento
        }
        
        // Avaliação de alarmes (Independente do I2C)
        bool alerta = (pressao_atual_psi < PRESSAO_MINIMA_PSI || pressao_atual_psi > PRESSAO_MAXIMA_PSI); 
        hw->buzzer_alerta(alerta);

        vTaskDelay(pdMS_TO_TICKS(500)); // Lê a cada 500ms
    }
}

// ---------------- THREAD 2: INTERFACE GRÁFICA ----------------
void task_display(void *pvParameters) {
    esp_lcd_panel_handle_t panel = lcd_init();
    buf = lcd_config();
    lcd_fill(panel, 0xFFFF, buf); // Limpa a tela com branco
    while (1) {
        char texto[32];
        snprintf(texto, sizeof(texto), "Pressao: %.2f PSI",
            static_cast<double>(pressao_atual_psi));
        
        
        lcd_draw_text(panel, 10, 10, texto, 0x0000, 0xFFFF, 1);
        
        /*
        // Solicita acesso exclusivo ao I2C
        if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
            bool is_psi = hw->get_is_unit_psi(); // Consulta o estado do botão
            display->atualizarDados(pressao_atual_psi, is_psi);
            xSemaphoreGive(i2c_mutex); // Libera o barramento
        }
        */
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Atualiza UI a cada 50ms para fluidez
    }
}

// -------------- ENTRY POINT --------------
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Sistema (PI III)...");

    // 1. Inicialização do Mutex do FreeRTOS
    i2c_mutex = xSemaphoreCreateMutex();

    // 2. Inicialização do barramento principal I2C
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_master_bus_config_t bus_config = { 
        .i2c_port = I2C_PORT_NUM,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT, 
        .glitch_ignore_cnt = 7, // Ignora interferências menores que 7 ciclos
        .flags = { .enable_internal_pullup = true } // Habilita pull-ups internos
        
    };
    bus_config.flags.enable_internal_pullup = true;
    
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    // 3. Instanciação Dinâmica dos Objetos
    hw = new Hardware();
    hw->inicializar();
    sensor = new SMP3011(i2c_bus_handle);
    //display = new DisplayOLED(i2c_bus_handle);
    // Instanciação e configuração do sensor de temperatura BMP280
    //ambiente = new BMP280(i2c_bus_handle);
    //ambiente->inicializar();

    



    // 4. Tarefas do FreeRTOS
    xTaskCreate(task_leitura_sensor, "TaskLeitura", 4096, NULL, 5, NULL);
    xTaskCreate(task_display, "TaskDisplay", 4096, NULL, 5, NULL);
    
}