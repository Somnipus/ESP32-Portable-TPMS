/**
 * @file bmp280.hpp
 * @brief Biblioteca para o sensor de temperatura e pressão BMP280 via I2C.
 */

#ifndef BMP280_HPP
#define BMP280_HPP

#include "driver/i2c_master.h"

#define BMP280_I2C_ADDR 0x76 // TEMPORARIO, coloquei 0x76 mas pode ser 0x78 ou outro

class BMP280 {
private:
    i2c_master_dev_handle_t bmp280_handle;

public:
    // Construtor
    BMP280(i2c_master_bus_handle_t bus_handle);

    // Métodos de inicialização e leitura
    void inicializar();
    float ler_temperatura_celsius();
};

#endif // BMP280_HPP