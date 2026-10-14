/**
 * @file smp3011.h
 * @brief Biblioteca para o sensor de pressão hidrostática SMP3011 via I2C.
 * 
 * PROJETO INTEGRADOR II - FATEC JUNDIAI
 */

#ifndef SMP3011_H
#define SMP3011_H

#include "driver/i2c_master.h"

/** @brief Endereço I2C de BUS para o sensor */
#define SMP3011_I2C_ADDR 0x78

/**
 * @brief Inicializa e associa o sensor ao barramento I2C.
 * @param bus_handle Referência (handle) do barramento I2C mestre.
 */
void smp3011_inicializar(i2c_master_bus_handle_t bus_handle);

/**
 * @brief Solicita e calcula a leitura de pressão.
 * @return Valor da pressão convertido em PSI (float).
 */
float smp3011_ler_pressao_psi(void);

#endif // SMP3011_H