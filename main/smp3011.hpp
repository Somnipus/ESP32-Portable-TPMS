/**
 * @file smp3011.hpp
 * @brief Bibliotéca para o sensor de pressão hidrostática SMP3011 via I2C.
 */

#ifndef SMP3011_HPP
#define SMP3011_HPP

#include "driver/i2c_master.h"

#define SMP3011_I2C_ADDR 0x78

class SMP3011 {
private:
    // Encapsulamento
    i2c_master_dev_handle_t smp3011_handle; 
    float offset_zero;

public:
    // Método Construtor
    SMP3011(i2c_master_bus_handle_t bus_handle);

    // Método de leitura
    float ler_pressao_psi();
};

#endif // SMP3011_HPP