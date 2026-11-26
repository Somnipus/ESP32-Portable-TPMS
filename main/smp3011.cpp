/**
 * @file smp3011.cpp
 * @brief Implementation of the SMP3011 class.
 */

#include "smp3011.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SMP3011";

// Constructor Implementation
SMP3011::SMP3011(i2c_master_bus_handle_t bus_handle) {
    this->offset_zero = 0.0f; // Starts the offset at zero
    this->calibrado = false; // Callibration flag set to false to avoid using uncalibrated values

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, // I2C uses 7 bits instead of 10
        .device_address = SMP3011_I2C_ADDR,
        .scl_speed_hz = 400000, 
    };
    
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &this->smp3011_handle)); 
    ESP_LOGI(TAG, "Sensor SMP3011 instanciado com sucesso!"); // SMP3011 sensor instantiated successfully!
}

// Reading Method Implementation
float SMP3011::ler_pressao_psi() {
    uint8_t comando[1] = { 0xAC }; 
    uint8_t buffer[4] = { 0 }; // Read 4 bytes (Status + MSB + CSB + LSB)
    
    i2c_master_transmit(this->smp3011_handle, comando, sizeof(comando), -1); // Send command to the sensor
    vTaskDelay(pdMS_TO_TICKS(20));
    i2c_master_receive(this->smp3011_handle, buffer, sizeof(buffer), -1); // Receive from sensor
    
    // Buffer 1 shifts 16 bits and adds buffer 2 (8 bits) and adds buffer 3
    // to reconstruct the raw value read by the sensor
    uint32_t bruta = ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
    float ppt = (float)bruta / 16777216.0f; 
    
    ESP_LOGI(TAG,
    "RAW: %lu | PPT: %.6f | OFFSET: %.6f",
    (unsigned long)bruta,
    ppt,
    this->offset_zero
    ); // Debug for raw value, ppt, and offset (can be removed in production)

    // Automatic off-set tare (Captures value in resting state)
    if (!this->calibrado) {
    this->offset_zero = ppt;
    this->calibrado = true;
    }
    
    float pressao_kpa = ((ppt - this->offset_zero) / 0.70f) * 500.0f; 

    if (pressao_kpa < 0.0f) {
        pressao_kpa = 0.0f;
    } // Filter to avoid displaying negative numbers on the screen

    ESP_LOGI(TAG,
    "PRESSAO KPA: %.2f | PRESSAO PSI: %.2f",
    pressao_kpa,
    pressao_kpa * 0.145038f
    ); // Debug for pressure in KPA and PSI (can be removed in production)

    return pressao_kpa * 0.145038f; // Converts the value from KPA to PSI
}