/**
 * @file smp3011.cpp
 * @brief Implementação da classe SMP3011.
 */

#include "smp3011.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SMP3011";

// Implementação do Construtor[cite: 19]
SMP3011::SMP3011(i2c_master_bus_handle_t bus_handle) {
    this->offset_zero = 0.0f; // Inicializa a tara em zero
    this->calibrado = false; // Temp, talvez tirar

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, // I2C usa 7 bits ao invés de 10
        .device_address = SMP3011_I2C_ADDR,
        .scl_speed_hz = 400000, 
    };
    
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &this->smp3011_handle)); 
    ESP_LOGI(TAG, "Sensor SMP3011 instanciado com sucesso!");
}

// Implementação do Método de Leitura
float SMP3011::ler_pressao_psi() {
    uint8_t comando[1] = { 0xAC }; 
    uint8_t buffer[4] = { 0 }; // Ler 4 bytes (Status + MSB + CSB + LSB)
    
    i2c_master_transmit(this->smp3011_handle, comando, sizeof(comando), -1); // Envia ao sensor
    vTaskDelay(pdMS_TO_TICKS(20));
    i2c_master_receive(this->smp3011_handle, buffer, sizeof(buffer), -1); // Recebe do sensor
    
    // Buffer 1 anda 16 bits e adiciona buffer 2 (8 bits) e adiciona buffer 3
    // para reconstruir o valor bruto lido pelo sensor
    uint32_t bruta = ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
    float ppt = (float)bruta / 16777216.0f; 
    
    ESP_LOGI(TAG,
    "RAW: %lu | PPT: %.6f | OFFSET: %.6f",
    (unsigned long)bruta,
    ppt,
    this->offset_zero
    ); // TEMP DE DEBUG: Remover em produção

    // Tara automática (Captura valor em estado de repouso)
    if (!this->calibrado) {
    this->offset_zero = ppt;
    this->calibrado = true;
    } // Temp, funciona bem, mas caso dê problemas, retornar para o anterior:
    //if (this->offset_zero == 0.0f) {
    //    this->offset_zero = ppt; 
    //}
    
    float pressao_kpa = ((ppt - this->offset_zero) / 0.70f) * 500.0f; 

    if (pressao_kpa < 0.0f) {
        pressao_kpa = 0.0f;
    } // Filtro para evitar exibição de número negativo na tela

    ESP_LOGI(TAG,
    "PRESSAO KPA: %.2f | PRESSAO PSI: %.2f",
    pressao_kpa,
    pressao_kpa * 0.145038f
    ); // TEMP DE DEBUG: Remover em produção

    return pressao_kpa * 0.145038f; // Converte o valor de KPA para PSI
}