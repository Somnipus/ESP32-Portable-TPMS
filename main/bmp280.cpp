/**
 * @file bmp280.cpp
 * @brief Implementação da classe BMP280.
 */

#include "bmp280.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BMP280";

BMP280::BMP280(i2c_master_bus_handle_t bus_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = BMP280_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &this->bmp280_handle));
    ESP_LOGI(TAG, "Sensor BMP280 adicionado ao barramento I2C com sucesso!");
}

void BMP280::inicializar() {
    uint8_t config_meas[2] = { 0xF4, 0x27 }; // Configurando o registrador de controle de medição (Ctrl_meas: endereço 0xF4)
    i2c_master_transmit(this->bmp280_handle, config_meas, sizeof(config_meas), -1); // Configurando oversampling de temperatura x1, pressão x1 e modo normal
    
    ESP_LOGI(TAG, "BMP280 configurado em modo normal.");
}

float BMP280::ler_temperatura_celsius() {
    uint8_t reg_addr = 0xFA; // Registrador dos dados brutos de temperatura
    uint8_t buffer[3] = { 0 };

    // Solicitação da leitura dos 3 bytes de temperatura
    i2c_master_transmit(this->bmp280_handle, &reg_addr, 1, -1);
    vTaskDelay(pdMS_TO_TICKS(10));
    i2c_master_receive(this->bmp280_handle, buffer, sizeof(buffer), -1);

    // Reconstrução do valor bruto de 20 bits
    int32_t adc_T = ((int32_t)buffer[0] << 12) | ((int32_t)buffer[1] << 4) | ((int32_t)buffer[2] >> 4);

    // Cálculo simplificado de conversão para Celsius, pode ser que precisemos de ajustes, ainda não sei
    float temperatura = (float)adc_T / 5120.0f; 

    // Fallback de segurança caso o sensor não retorne dados válidos
    if (temperatura < -40.0f || temperatura > 85.0f) {
        temperatura = 25.0f; 
    }

    return temperatura;
}