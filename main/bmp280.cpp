/**
 * @file bmp280.cpp
 * @brief Implementação da classe BMP280.
 */

#include "bmp280.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <limits>

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
    ESP_ERROR_CHECK(i2c_master_transmit(
        this->bmp280_handle, config_meas, sizeof(config_meas), -1));

    uint8_t reg_addr = 0x88;
    uint8_t data[6] = {};
    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        this->bmp280_handle, &reg_addr, 1, data, sizeof(data), -1));

    this->calibracao.T1 = (uint16_t)(((uint16_t)data[1] << 8) | data[0]);
    this->calibracao.T2 = (int16_t)(((uint16_t)data[3] << 8) | data[2]);
    this->calibracao.T3 = (int16_t)(((uint16_t)data[5] << 8) | data[4]);
    ESP_ERROR_CHECK(this->calibracao.T1 == 0 ? ESP_ERR_INVALID_RESPONSE : ESP_OK);
    this->calibracao_carregada = true;

    ESP_LOGI(TAG, "Calibracao temperatura T1=%u T2=%d T3=%d",
             (unsigned)this->calibracao.T1,
             this->calibracao.T2,
             this->calibracao.T3);
    ESP_LOGI(TAG, "BMP280 configurado em modo normal.");
}

float BMP280::ler_temperatura_celsius() {
    ESP_ERROR_CHECK(this->calibracao_carregada ? ESP_OK : ESP_ERR_INVALID_STATE);

    uint8_t reg_addr = 0xFA; // Registrador dos dados brutos de temperatura
    uint8_t buffer[3] = { 0 };

    ESP_ERROR_CHECK(i2c_master_transmit_receive(
        this->bmp280_handle, &reg_addr, 1, buffer, sizeof(buffer), -1));

    int32_t adc_T = ((int32_t)buffer[0] << 12) |
                    ((int32_t)buffer[1] << 4) |
                    ((int32_t)buffer[2] >> 4);
    if (adc_T == 0x80000) {
        ESP_LOGW(TAG, "ADC de temperatura invalido (0x80000)");
        return std::numeric_limits<float>::quiet_NaN();
    }

    int32_t var1 = ((((adc_T >> 3) - ((int32_t)this->calibracao.T1 << 1))) *
                    (int32_t)this->calibracao.T2) >> 11;
    int32_t delta = (adc_T >> 4) - (int32_t)this->calibracao.T1;
    int32_t var2 = (((((delta * delta) >> 12) *
                      (int32_t)this->calibracao.T3)) >> 14);
    int32_t t_fine = var1 + var2;
    float temperatura = (float)((t_fine * 5 + 128) >> 8) / 100.0f;

    if (temperatura < -40.0f || temperatura > 85.0f) {
        ESP_LOGW(TAG, "Leitura incorreta: %.2f", temperatura);
        return std::numeric_limits<float>::quiet_NaN();
    }

    ESP_LOGD(TAG, "ADC_T=%ld temperatura=%.2f C", (long)adc_T, temperatura);
    return temperatura;
}