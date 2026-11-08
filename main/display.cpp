/**
 * @file display.cpp
 * @brief Inicialização do SSD1306 e renderização do layout LVGL estático.
 */

#include "display.hpp"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lvgl_port.h"
#include <stdio.h>

static const char *TAG = "DISPLAY";

DisplayOLED::DisplayOLED(i2c_master_bus_handle_t bus_handle) {
    ESP_LOGI(TAG, "Configurando Motor Gráfico (ESP_LCD + LVGL)...");
    
    esp_lcd_panel_io_handle_t io_handle = NULL; 
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = SSD1306_I2C_ADDR, 
        .scl_speed_hz = I2C_CLOCK_SPEED_HZ, 
        .control_phase_bytes = 1, // 1 byte extra para indicar Data/Command
        .lcd_cmd_bits = 8, 
        .lcd_param_bits = 8, 
        .dc_bit_offset = 6, // Data-Command está na posição 6
    }; // !! Todas as configurações são padrão. Não alterar !!
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL; 
    esp_lcd_panel_dev_config_t panel_config = { 
        .bits_per_pixel = 1, // Monocromático (apagado ou aceso)
        .reset_gpio_num = OLED_RST_PIN, 
    };
    
    esp_lcd_panel_ssd1306_config_t ssd1306_config = { 
        .height = OLED_ALTURA, 
    };
    
    panel_config.vendor_config = &ssd1306_config; 
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle)); 
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle)); 
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true)); 

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG(); 
    lvgl_port_init(&lvgl_cfg); 

    const lvgl_port_display_cfg_t disp_cfg = { 
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = OLED_LARGURA * OLED_ALTURA, // 1024 bytes
        .double_buffer = true, // Maior fluidez (Buffer 1 desenha, Buffer 2 envia)
        .hres = OLED_LARGURA, 
        .vres = OLED_ALTURA, 
        .monochrome = true, 
        .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false } 
    };
    
    this->tela_lvgl = lvgl_port_add_disp(&disp_cfg); 

	// Desenho do Layout Fixo
    if (this->lock()) { // Trava o mutex do LVGL
        lv_obj_t *tela_atual = lv_disp_get_scr_act(this->tela_lvgl); 

        // 1. Interface Gráfica de Seleção 
        this->btn_ui_psi = lv_btn_create(tela_atual); 
        lv_obj_set_size(this->btn_ui_psi, 45, 18); 
        lv_obj_align(this->btn_ui_psi, LV_ALIGN_TOP_LEFT, 5, 2); 
        lv_obj_t *lbl_psi = lv_label_create(this->btn_ui_psi); 
        lv_label_set_text(lbl_psi, "PSI"); 
        lv_obj_center(lbl_psi); 

        this->btn_ui_bar = lv_btn_create(tela_atual);
        lv_obj_set_size(this->btn_ui_bar, 45, 18);
        lv_obj_align(this->btn_ui_bar, LV_ALIGN_TOP_RIGHT, -5, 2);
        lv_obj_t *lbl_bar = lv_label_create(this->btn_ui_bar);
        lv_label_set_text(lbl_bar, "BAR");
        lv_obj_center(lbl_bar);
        
        // 2. Elementos de Leitura
        this->label_pressao = lv_label_create(tela_atual);
        lv_label_set_text(this->label_pressao, "--.-");
        lv_obj_align(this->label_pressao, LV_ALIGN_CENTER, 0, -5);

        this->label_status = lv_label_create(tela_atual);
        lv_label_set_text(this->label_status, "Aguardando...");
        lv_obj_align(this->label_status, LV_ALIGN_BOTTOM_MID, 0, -12);

        this->label_acao = lv_label_create(tela_atual);
        lv_label_set_text(this->label_acao, "...");
        lv_obj_align(this->label_acao, LV_ALIGN_BOTTOM_MID, 0, 0);

        // 3. Elementos de Seleção
        this->label_mode = lv_label_create(tela_atual);
        lv_label_set_text(this->label_mode, "--->");
        lv_obj_align(this->label_mode, LV_ALIGN_TOP_MID, 0, 4);

        this->unlock(); 
    }
}

bool DisplayOLED::lock() {
    return lvgl_port_lock(0);
}

void DisplayOLED::unlock() {
    lvgl_port_unlock();
}

void DisplayOLED::atualizarDados(float pressao_psi, bool is_unit_psi) {
    if (this->lock()) {
        char buf_pressao[32];
        
        if (is_unit_psi) { 
            snprintf(buf_pressao, sizeof(buf_pressao), "%.1f PSI", pressao_psi); 
            lv_label_set_text(this->label_mode, "<---"); 
        } else { 
            float pressao_bar = pressao_psi * PSI_PARA_BAR_FATOR; 
            snprintf(buf_pressao, sizeof(buf_pressao), "%.2f BAR", pressao_bar);
            lv_label_set_text(this->label_mode, "--->"); 
        }

        lv_label_set_text(this->label_pressao, buf_pressao); 

        if (pressao_psi < PRESSAO_MINIMA_PSI) { 
            lv_label_set_text(this->label_status, "ESTADO: BAIXA");
            lv_label_set_text(this->label_acao, "Recalibrar!");
        } else if (pressao_psi > PRESSAO_MAXIMA_PSI) { 
            lv_label_set_text(this->label_status, "ESTADO: ALTA");
            lv_label_set_text(this->label_acao, "Recalibrar!");
        } else { 
            lv_label_set_text(this->label_status, "ESTADO: OK");
            lv_label_set_text(this->label_acao, "Pode Rodar");
        }
        
        this->unlock();
    }
}