/**
 * @file hardware.cpp
 * @brief Implementação do controle de GPIOs e interrupções.
 */

#include "hardware.hpp"
#include "esp_log.h"

static const char *TAG = "HARDWARE";

Hardware::Hardware() {
    this->is_unit_psi = true;
    this->last_btn_press_time = 0; // Borda de descida
}

// Rotina de interrupção (ISR)
void IRAM_ATTR Hardware::btn_isr_handler(void* arg) {
    // Recupera a instância da classe this passada no momento do registo
    Hardware* instancia = static_cast<Hardware*>(arg);
    
    uint32_t current_time = xTaskGetTickCountFromISR(); // Equivalente ao millis()
    
    // Debounce de 300ms
    if (current_time - instancia->last_btn_press_time > pdMS_TO_TICKS(300)) { 
        instancia->is_unit_psi = !instancia->is_unit_psi; // Altera a grandeza
        instancia->last_btn_press_time = current_time; 
    }
}

void Hardware::inicializar() {
    ESP_LOGI(TAG, "Configurando GPIOs de Controlo...");
    
    // 1. Configuração do Buzzer
    gpio_reset_pin(BUZZER_PIN); 
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT); 
    gpio_set_level(BUZZER_PIN, 0); 

    // 2. Configuração do Botão
    gpio_reset_pin(BTN_UNIT_PIN); 
    gpio_set_direction(BTN_UNIT_PIN, GPIO_MODE_INPUT); 
    gpio_set_pull_mode(BTN_UNIT_PIN, GPIO_PULLUP_ONLY); // Pull-Up interno, pode ser alterado se a construção elétrica mudar
    gpio_set_intr_type(BTN_UNIT_PIN, GPIO_INTR_NEGEDGE); // Borda de descida interrompe

    // 3. Registo da Interrupção
    esp_err_t err = gpio_install_isr_service(0); // 0 sinaliza sem flag adicional
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) { 
         ESP_LOGE(TAG, "Falha ao instalar serviço ISR: %s", esp_err_to_name(err)); 
    }
    
    // Passa o ponteiro 'this' como argumento para que a ISR estática saiba qual objeto alterar
    gpio_isr_handler_add(BTN_UNIT_PIN, Hardware::btn_isr_handler, this); 
}

void Hardware::buzzer_alerta(bool alerta) {
    if (alerta) {
        int ms_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) % 1000; // Bipe intermitente usando o Tick Count (aprox. 2Hz) a cada 1 segundo
        gpio_set_level(BUZZER_PIN, (ms_time < 500) ? 1 : 0);  // Se antes da metade do segundo: bit 1 (ligado). Se depois: bit 0 (desligado)
    } else {
        gpio_set_level(BUZZER_PIN, 0); 
    }
}

bool Hardware::get_is_unit_psi() {
    return this->is_unit_psi;
}