/**
 * @file hardware.hpp
 * @brief Definições para controle de periféricos básicos (GPIOs)
 */

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUZZER_PIN             GPIO_NUM_18 
#define BTN_UNIT_PIN           GPIO_NUM_19 

class Hardware {
private:
    volatile bool is_unit_psi;
    volatile uint32_t last_btn_press_time;
    static void IRAM_ATTR btn_isr_handler(void* arg);

public:
    // Construtor
    Hardware();

    void inicializar();
    void buzzer_alerta(bool alerta);
    
    bool get_is_unit_psi(); 
};

#endif // HARDWARE_HPP