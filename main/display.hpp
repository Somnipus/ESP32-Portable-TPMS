/**
 * @file display.hpp
 * @brief Graphical interface module for ILI9341 TFT LCD display using the LVGL library.
 */

#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "driver/i2c_master.h"
#include "lvgl.h"
#include "esp_lcd_io_spi.h"

#define OLED_RST_PIN           GPIO_NUM_16
#define I2C_CLOCK_SPEED_HZ     (400 * 1000) 

// Pressure limits for the UI
#define PRESSAO_MINIMA_PSI     28.0f
#define PRESSAO_MAXIMA_PSI     36.0f 
#define PSI_PARA_BAR_FATOR     0.0689476f

esp_lcd_panel_handle_t lcd_init();
void lcd_fill(esp_lcd_panel_handle_t panel, uint16_t color, uint16_t *buf);
uint16_t *lcd_config();
void lcd_draw_text(esp_lcd_panel_handle_t panel, int x, int y, const char *str,
                          uint16_t fg, uint16_t bg, int scale);

void lvgl_config();
void desenhar_tela_lvgl();
class DisplayOLED {
private:
    // LVGL pointers (presets)
    lv_disp_t *tela_lvgl; 
    lv_obj_t  *label_status; //< Pressure status (low, high, good)
    lv_obj_t  *label_pressao; //< Numeric preset for pressure
    lv_obj_t  *label_acao; //< Recommendation (Recalibrate or OK)
    lv_obj_t  *label_mode; //< Unit selection indicator (arrow)
    lv_obj_t  *btn_ui_psi;
    lv_obj_t  *btn_ui_bar;

public:
    // Visual update of the display based on pressure readings and unit selection
    void atualizarDados(float pressao_psi, bool is_unit_psi);

};



#endif // DISPLAY_HPP