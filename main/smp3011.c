/**
 * @file smp3011.c
 * @brief Implementação da comunicação I2C e cálculo matemático do sensor SMP3011.
 */

#include "smp3011.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SMP3011";
static i2c_master_dev_handle_t smp3011_handle; 

void smp3011_inicializar(i2c_master_bus_handle_t bus_handle) {
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7, // I2C usa 7 bits ao invés de 10
        .device_address = SMP3011_I2C_ADDR, 
        .scl_speed_hz = 400000, // Frequência do clock
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &smp3011_handle)); 
    ESP_LOGI(TAG, "Sensor SMP3011 adicionado com sucesso!");
}

float smp3011_ler_pressao_psi(void) {
    uint8_t comando[1] = { 0xAC }; // Prepara a mensagem do ESP ao sensor
    uint8_t buffer[4] = { 0 }; // Ler 4 bytes (Status + MSB + CSB + LSB)
    
    // ESP envia a mensagem ao sensor via I2C
    i2c_master_transmit(smp3011_handle, comando, sizeof(comando), -1); 

    vTaskDelay(pdMS_TO_TICKS(20));

    // ESP recebe a resposta do sensor
    i2c_master_receive(smp3011_handle, buffer, sizeof(buffer), -1);
    
    // Buffer 1 anda 16 bits e adiciona buffer 2 (8 bits) e adiciona buffer 3
    // para reconstruir o valor bruto lido pelo sensor
    uint32_t bruta = ((uint32_t)buffer[1] << 16) | ((uint32_t)buffer[2] << 8) | buffer[3];
    float ppt = (float)bruta / 16777216.0f; 
    
    // Tara automática na primeira execução (Captura valor em estado de repouso)
    static float offset_zero = 0.0f;
    if (offset_zero == 0.0f) {
        offset_zero = ppt; 
    } 
    
    float pressao_kpa = ((ppt - offset_zero) / 0.70f) * 500.0f; 

    // Filtro para evitar exibição de número negativo na tela
    if (pressao_kpa < 0.0f) {
        pressao_kpa = 0.0f;
    } 

    return pressao_kpa * 0.145038f; // Converte o valor de KPA para PSI
}