/**
 * @file bmp280.hpp
 * @brief Biblioteca para o sensor de temperatura e pressão BMP280 via I2C.
 */

#ifndef BMP280_HPP
#define BMP280_HPP

#include "driver/i2c_master.h"

#define BMP280_I2C_ADDR 0x76

class BMP280 {
private:
    struct calibracao_temperatura_t {
        uint16_t T1 = 0;
        int16_t T2 = 0;
        int16_t T3 = 0;
    };

    i2c_master_dev_handle_t bmp280_handle;
    calibracao_temperatura_t calibracao;
    bool calibracao_carregada = false;

public:
    // Construtor
    BMP280(i2c_master_bus_handle_t bus_handle);

    // Métodos de inicialização e leitura
    void inicializar();
    float ler_temperatura_celsius();
};

#endif // BMP280_HPP