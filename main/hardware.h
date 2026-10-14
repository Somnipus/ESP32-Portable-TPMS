/**
 * @file hardware.h
 * @brief Definições e assinaturas para controle de periféricos básicos (GPIOs).
 * 
 * PROJETO INTEGRADOR II - FATEC JUNDIAI
 * Tema: Prototipo de sensor de pressao para pneus.
 * Microcontrolador: ESP32
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include "driver/gpio.h"

// =============== CONFIGURACOES DO HARDWARE ===============

/** @brief Pino do piezo buzzer */
#define BUZZER_PIN             GPIO_NUM_18 
/** @brief Pino do botão de seleção de unidade */
#define BTN_UNIT_PIN           GPIO_NUM_19 

/** @brief Variável exposta para checar se a unidade atual é PSI */
extern volatile bool is_unit_psi;

/**
 * @brief Configura e inicializa os pinos de entrada e saída.
 */
void gpios_inicializar(void);

/**
 * @brief Controla o acionamento do buzzer de alerta.
 * @param alerta Booleano que define se o alerta deve soar (true) ou silenciar (false).
 */
void hardware_buzzer_alerta(bool alerta);

#endif // HARDWARE_H