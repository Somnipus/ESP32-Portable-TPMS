/**
 * @file display.hpp
 * @brief Módulo de interface gráfica para display OLED SSD1306 com biblioteca LVGL.
 */

#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "driver/i2c_master.h"
#include "lvgl.h"
#include "esp_lcd_io_spi.h"

#define OLED_RST_PIN           GPIO_NUM_16
#define I2C_CLOCK_SPEED_HZ     (400 * 1000) 
#define SSD1306_I2C_ADDR       0x3C 
#define OLED_LARGURA           128
#define OLED_ALTURA            64

// Limites de Pressao para a UI
#define PRESSAO_MINIMA_PSI     28.0f
#define PRESSAO_MAXIMA_PSI     36.0f 
#define PSI_PARA_BAR_FATOR     0.0689476f

esp_lcd_panel_handle_t lcd_init();
void lcd_fill(esp_lcd_panel_handle_t panel, uint16_t color, uint16_t *buf);
uint16_t *lcd_config();
void lcd_draw_text(esp_lcd_panel_handle_t panel, int x, int y, const char *str,
                          uint16_t fg, uint16_t bg, int scale);
class DisplayOLED {
private:
    // Ponteiros LVGL (presets)
    lv_disp_t *tela_lvgl; 
    lv_obj_t  *label_status; //< Estado da pressão (baixa, alta, boa)
    lv_obj_t  *label_pressao; //< Preset numérico da pressão
    lv_obj_t  *label_acao; //< Recomendação (Recalibrar ou OK)
    lv_obj_t  *label_mode; //< Indicador de seleção de unidade (seta)
    lv_obj_t  *btn_ui_psi;
    lv_obj_t  *btn_ui_bar;

public:
    // Construtor
    //DisplayOLED(i2c_master_bus_handle_t bus_handle);

    // Métodos de controle
    //bool lock();
    //void unlock();
    
    // Atualização visual
    void atualizarDados(float pressao_psi, bool is_unit_psi);
    
};



#endif // DISPLAY_HPP