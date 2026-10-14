/**
 * @file main.c
 * @brief Ponto de entrada do sistema (Maestro). Coordena inicializações e a thread principal de leitura.
 * 
 * PROJETO INTEGRADOR II - FATEC JUNDIAI
 * Tema: Prototipo de sensor de pressao para pneus.
 * Microcontrolador: ESP32
 * Sensor: SMP3011 (Pressão Hidrostatica I2C)
 * Tela: OLED SSD1306 I2C (Rodando com ESP_LCD e LVGL)
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

// Módulos da aplicação
#include "hardware.h"
#include "smp3011.h"
#include "display.h"

static const char *TAG = "SENSOR_PNEUS";

/** @brief Porta do I2C mestre */
#define I2C_PORT_NUM           0
/** @brief Fio azul do sensor de pressão */
#define I2C_SDA_PIN            GPIO_NUM_5
/** @brief Fio amarelo do sensor de pressão */
#define I2C_SCL_PIN            GPIO_NUM_4

// Limites de Pressao (SEMPRE PSI NAO MUDEM)
#define PRESSAO_MINIMA_PSI     28.0f
#define PRESSAO_MAXIMA_PSI     36.0f 

/** @brief Fator de Conversão para BAR */
#define PSI_PARA_BAR_FATOR     0.0689476f

i2c_master_bus_handle_t i2c_bus_handle;

void app_main(void) { 
    ESP_LOGI(TAG, "Iniciando Sistema...");
    
    // Inicialização do barramento principal I2C
    i2c_master_bus_config_t bus_config = { 
        .i2c_port = I2C_PORT_NUM, 
        .sda_io_num = I2C_SDA_PIN, 
        .scl_io_num = I2C_SCL_PIN, 
        .clk_source = I2C_CLK_SRC_DEFAULT, // Fonte recomendada pela ESPRESSIF
        .glitch_ignore_cnt = 7, // Ignora interferências menores que 7 ciclos
        .flags.enable_internal_pullup = true, // Evita resistor externo na protoboard
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle)); 

    // Inicialização dos Módulos Especializados
    gpios_inicializar();
    oled_inicializar(i2c_bus_handle);
    smp3011_inicializar(i2c_bus_handle);

    char buf_pressao[32]; // Buffer para armazenar o texto formatado
    float pressao_psi = 0;
    float pressao_exibida = 0;
    TickType_t ultima_leitura_sensor = 0; 

    // Thread Principal do Sistema
    while (1) {
        TickType_t agora = xTaskGetTickCount(); 

        // 1. LEITURA DO SENSOR (A cada 500ms)
        if ((agora - ultima_leitura_sensor) >= pdMS_TO_TICKS(500) || ultima_leitura_sensor == 0) { 
            pressao_psi = smp3011_ler_pressao_psi(); 
            ultima_leitura_sensor = agora; 
        }

        // 2. TRATAMENTO DE GRANDEZAS E UI
        if (display_lock()) {
            if (is_unit_psi) { // Se a unidade for PSI
                pressao_exibida = pressao_psi; 
                snprintf(buf_pressao, sizeof(buf_pressao), "%.1f PSI", pressao_exibida); 
                lv_label_set_text(label_mode, "<---"); // Seta aponta para PSI
            } else { // Se a unidade for BAR
                pressao_exibida = pressao_psi * PSI_PARA_BAR_FATOR; 
                snprintf(buf_pressao, sizeof(buf_pressao), "%.2f BAR", pressao_exibida);
                lv_label_set_text(label_mode, "--->"); // Seta aponta para BAR
            }

            // 3. ATUALIZAÇÃO DOS TEXTOS NA TELA
            lv_label_set_text(label_pressao, buf_pressao); 

            if (pressao_psi < PRESSAO_MINIMA_PSI) { 
                lv_label_set_text(label_status, "ESTADO: BAIXA");
                lv_label_set_text(label_acao, "Recalibrar!");
            } else if (pressao_psi > PRESSAO_MAXIMA_PSI) { 
                lv_label_set_text(label_status, "ESTADO: ALTA");
                lv_label_set_text(label_acao, "Recalibrar!");
            } else { 
                lv_label_set_text(label_status, "ESTADO: OK");
                lv_label_set_text(label_acao, "Pode Rodar");
            }
            display_unlock();
        }

        // 4. AVALIAÇÃO DE ALARMES
        bool alerta = (pressao_psi < PRESSAO_MINIMA_PSI || pressao_psi > PRESSAO_MAXIMA_PSI); 
        hardware_buzzer_alerta(alerta);

        // 5. PAUSA DO LAÇO (Mantém a tela fluida e botão responsivo)
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}