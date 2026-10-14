/**
 * @file hardware.c
 * @brief Implementação do controle de GPIOs e interrupções.
 */

#include "hardware.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HARDWARE";

// Boolean de armazenamento para checar se a unidade é PSI
volatile bool is_unit_psi = true; 
// Estado anterior do botão para detecção da borda de descida (debounce)
volatile uint32_t last_btn_press_time = 0; 

/**
 * @brief Rotina de interrupção (ISR) para o botão de unidade.
 * @note Possui debounce de 300ms para evitar duplicatas de pressão.
 */
static void IRAM_ATTR btn_isr_handler(void* arg) {
    uint32_t current_time = xTaskGetTickCountFromISR(); // Equivalente ao millis()
    
    // Checa se o clique aconteceu pelo menos 300ms desde o último aperto
    if (current_time - last_btn_press_time > pdMS_TO_TICKS(300)) { 
        is_unit_psi = !is_unit_psi; // Alterna a grandeza
        last_btn_press_time = current_time; // Atualiza o armazenamento
    }
}

void gpios_inicializar(void) {
    ESP_LOGI(TAG, "Configurando GPIOs de Controle...");
    
    // 1. Configuração do Buzzer
    gpio_reset_pin(BUZZER_PIN); 
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT); 
    gpio_set_level(BUZZER_PIN, 0); // Garante que inicia desligado

    // 2. Configuração do Botão
    gpio_reset_pin(BTN_UNIT_PIN); 
    gpio_set_direction(BTN_UNIT_PIN, GPIO_MODE_INPUT); 
    gpio_set_pull_mode(BTN_UNIT_PIN, GPIO_PULLUP_ONLY); // PULL UP INTERNO (NÃO ALTEREM)
    gpio_set_intr_type(BTN_UNIT_PIN, GPIO_INTR_NEGEDGE); // Interrupção na borda de descida

    // 3. Instala e associa a interrupção
    esp_err_t err = gpio_install_isr_service(0); // 0 sinaliza sem flag adicional
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) { 
         ESP_LOGE(TAG, "Falha ao instalar serviço ISR: %s", esp_err_to_name(err)); 
    }
    gpio_isr_handler_add(BTN_UNIT_PIN, btn_isr_handler, NULL); 
}

void hardware_buzzer_alerta(bool alerta) {
    if (alerta) {
        // Bipe intermitente seguro usando o Tick Count (aprox. 2Hz) a cada 1 segundo
        int ms_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) % 1000; 
        // Se antes da metade do segundo: bit 1 (ligado). Se depois: bit 0 (desligado)
        gpio_set_level(BUZZER_PIN, (ms_time < 500) ? 1 : 0); 
    } else {
        gpio_set_level(BUZZER_PIN, 0); // Silencia o buzzer
    }
}