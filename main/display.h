/**
 * @file display.h
 * @brief Módulo de interface gráfica para display OLED SSD1306 com biblioteca LVGL.
 * 
 * PROJETO INTEGRADOR II - FATEC JUNDIAI
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "driver/i2c_master.h"
#include "lvgl.h"

// =============== CONFIGURACOES DO DISPLAY ===============

/** @brief Pino de RESET padrão para OLED */
#define OLED_RST_PIN           GPIO_NUM_16
/** @brief 400kHz de velocidade padrão para o OLED */
#define I2C_CLOCK_SPEED_HZ     (400 * 1000) 
/** @brief Endereço de BUS para o OLED */
#define SSD1306_I2C_ADDR       0x3C 

#define OLED_LARGURA           128
#define OLED_ALTURA            64

// Objetos Gráficos Expostos (presets)
extern lv_obj_t  *label_status;  /**< Estado da pressão (baixa, alta, boa) */
extern lv_obj_t  *label_pressao; /**< Preset numérico da pressão */
extern lv_obj_t  *label_acao;    /**< Recomendação (Recalibrar ou OK) */
extern lv_obj_t  *label_mode;    /**< Seta indicadora de seleção de unidade */

/**
 * @brief Inicializa o controlador do display (ESP_LCD) e o motor gráfico (LVGL).
 * @param bus_handle Referência do barramento I2C para comunicação gráfica.
 */
void oled_inicializar(i2c_master_bus_handle_t bus_handle);

/** @brief Trava o mutex do LVGL para atualizações seguras de UI. */
bool display_lock(void);

/** @brief Destrava o mutex do LVGL após as atualizações. */
void display_unlock(void);

#endif // DISPLAY_H