/**
 * @file hardware.cpp
 * @brief Implementation of GPIO control and interrupts.
 */

#include "hardware.hpp"
#include "esp_log.h"

static const char *TAG = "HARDWARE";

Hardware::Hardware() {
    this->is_unit_psi = true;
    this->last_btn_press_time = 0; // Falling edge
}

// Interruption routine (ISR)
void IRAM_ATTR Hardware::btn_isr_handler(void* arg) {
    // Recovers the class instance 'this' passed at the time of registration
    Hardware* instancia = static_cast<Hardware*>(arg);
    
    uint32_t current_time = xTaskGetTickCountFromISR(); // Equivalent to millis()
    
    // 300ms debounce
    if (current_time - instancia->last_btn_press_time > pdMS_TO_TICKS(300)) { 
        instancia->is_unit_psi = !instancia->is_unit_psi; // Changes the unit of measurement
        instancia->last_btn_press_time = current_time; 
    }
}

void Hardware::inicializar() {
    ESP_LOGI(TAG, "Configurando GPIOs de Controle..."); // Configuring Control GPIOs...
    
    // 1. Buzzer Setting
    gpio_reset_pin(BUZZER_PIN); 
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT); 
    gpio_set_level(BUZZER_PIN, 0); 

    // 2. Button Settin
    gpio_reset_pin(BTN_UNIT_PIN); 
    gpio_set_direction(BTN_UNIT_PIN, GPIO_MODE_INPUT); 
    gpio_set_pull_mode(BTN_UNIT_PIN, GPIO_PULLUP_ONLY); // Native pull-up resistor, can be changed if the electrical construction changes
    gpio_set_intr_type(BTN_UNIT_PIN, GPIO_INTR_NEGEDGE); // Falling edge interrupt

    // 3. Registo da Interrupção
    esp_err_t err = gpio_install_isr_service(0); // 0 signs without additional flag
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) { 
         ESP_LOGE(TAG, "Falha ao instalar serviço ISR: %s", esp_err_to_name(err)); // Failed to install ISR service
    }
    
    // Uses the pointer 'this' as an argument so that the static ISR knows which object to modify
    gpio_isr_handler_add(BTN_UNIT_PIN, Hardware::btn_isr_handler, this); 
}

void Hardware::buzzer_alerta(bool alerta) {
    if (alerta) {
        int ms_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) % 1000; // Intermitent beep using Tick Count (approx. 2Hz) every 1 second
        gpio_set_level(BUZZER_PIN, (ms_time < 500) ? 1 : 0);  // If before half a second: bit 1 (on). If after: bit 0 (off)
    } else {
        gpio_set_level(BUZZER_PIN, 0); 
    }
}

bool Hardware::get_is_unit_psi() {
    return this->is_unit_psi;
}