/*
 * Boilerplate ESP-IDF 5.x - display SPI 320x240 (landscape) no ESP32
 * Pinos: SCK=14, MOSI=13, CS=27, RST=25, DC=LCD_DC_GPIO (AJUSTE!), BL direto em 3.3V
 *
 * Controlador: ILI9341 (padrão) ou ST7789 -> altere LCD_CTRL_ILI9341 abaixo.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

/* ------------------------- Seleção do controlador ------------------------- */
#define LCD_CTRL_ILI9341 1   /* 1 = ILI9341 | 0 = ST7789 */

#if LCD_CTRL_ILI9341
#include "esp_lcd_ili9341.h"
#endif

/* ------------------------------ Configuração ------------------------------ */
#define LCD_HOST        SPI2_HOST       /* HSPI: pinos 14/13 são os nativos (IOMUX) */
#define LCD_PIN_SCK     14
#define LCD_PIN_MOSI    13
#define LCD_PIN_MISO    (-1)            /* não usado */
#define LCD_PIN_CS      27
#define LCD_PIN_RST     25
#define LCD_PIN_DC      26              /* <<< AJUSTE: você não informou o pino DC */

#define LCD_PIXEL_CLK_HZ (40 * 1000 * 1000)

/* Resolução lógica em paisagem (largura 320, altura 240) */
#define LCD_H_RES       240
#define LCD_V_RES       320
#define LCD_DRAW_LINES  300          /* linhas por transferência DMA */

static const char *TAG = "lcd";

static SemaphoreHandle_t s_flush_done;

/* Chamada (ISR) quando a transferência de cor termina */
static bool IRAM_ATTR on_color_trans_done(esp_lcd_panel_io_handle_t io,
                                          esp_lcd_panel_io_event_data_t *edata,
                                          void *user_ctx)
{
    BaseType_t high_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(s_flush_done, &high_task_woken);
    return high_task_woken == pdTRUE;
}

esp_lcd_panel_handle_t lcd_init(void)
{
    s_flush_done = xSemaphoreCreateBinary();

    /* 1) Barramento SPI */
    spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = LCD_PIN_MISO,
        .sclk_io_num = LCD_PIN_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_DRAW_LINES * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    /* 2) Panel IO (SPI + DC + CS) */
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_PIN_CS,
        .dc_gpio_num = LCD_PIN_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PIXEL_CLK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = on_color_trans_done,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8, 
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
                                             &io_config, &io_handle));

    /* 3) Driver do painel */
    esp_lcd_panel_handle_t panel = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,  /* se cores trocadas: use _RGB */
        .bits_per_pixel = 16,
    };
#if LCD_CTRL_ILI9341
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel));
#else
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));

#if !LCD_CTRL_ILI9341
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel, true));  /* ST7789 geralmente precisa */
#endif

    /* Orientação: painel nativo é 240x320 (retrato) -> paisagem 320x240.
     * Se a imagem sair espelhada/invertida, ajuste os flags do mirror. */
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel, false, false));

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    ESP_LOGI(TAG, "Display inicializado (%dx%d)", LCD_H_RES, LCD_V_RES);
    return panel;
}
static const uint8_t font8x8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, /* ! */
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, /* " */
    {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00}, /* # */
    {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00}, /* $ */
    {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00}, /* % */
    {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00}, /* & */
    {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00}, /* ' */
    {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00}, /* ( */
    {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00}, /* ) */
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, /* * */
    {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00}, /* + */
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06}, /* , */
    {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00}, /* - */
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00}, /* . */
    {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, /* / */
    {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00}, /* 0 */
    {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00}, /* 1 */
    {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00}, /* 2 */
    {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00}, /* 3 */
    {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00}, /* 4 */
    {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00}, /* 5 */
    {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00}, /* 6 */
    {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00}, /* 7 */
    {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00}, /* 8 */
    {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00}, /* 9 */
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00}, /* : */
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06}, /* ; */
    {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00}, /* < */
    {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00}, /* = */
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, /* > */
    {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00}, /* ? */
    {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00}, /* @ */
    {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00}, /* A */
    {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00}, /* B */
    {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00}, /* C */
    {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00}, /* D */
    {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00}, /* E */
    {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00}, /* F */
    {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00}, /* G */
    {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00}, /* H */
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, /* I */
    {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00}, /* J */
    {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00}, /* K */
    {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00}, /* L */
    {0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00}, /* M */
    {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00}, /* N */
    {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00}, /* O */
    {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00}, /* P */
    {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00}, /* Q */
    {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00}, /* R */
    {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00}, /* S */
    {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, /* T */
    {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00}, /* U */
    {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00}, /* V */
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, /* W */
    {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00}, /* X */
    {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00}, /* Y */
    {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00}, /* Z */
    {0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00}, /* [ */
    {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, /* \ */
    {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00}, /* ] */
    {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00}, /* ^ */
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, /* _ */
    {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, /* ` */
    {0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00}, /* a */
    {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00}, /* b */
    {0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00}, /* c */
    {0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0x00}, /* d */
    {0x00,0x00,0x1E,0x33,0x3F,0x03,0x1E,0x00}, /* e */
    {0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00}, /* f */
    {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F}, /* g */
    {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00}, /* h */
    {0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00}, /* i */
    {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E}, /* j */
    {0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00}, /* k */
    {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, /* l */
    {0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00}, /* m */
    {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00}, /* n */
    {0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00}, /* o */
    {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F}, /* p */
    {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78}, /* q */
    {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00}, /* r */
    {0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00}, /* s */
    {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00}, /* t */
    {0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00}, /* u */
    {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00}, /* v */
    {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, /* w */
    {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00}, /* x */
    {0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F}, /* y */
    {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00}, /* z */
    {0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00}, /* { */
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, /* | */
    {0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00}, /* } */
    {0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00}, /* ~ */
};
 
/* Desenha uma string na posição (x, y) em pixels. scale = fator de ampliação (1..4+). */
void lcd_draw_text(esp_lcd_panel_handle_t panel, int x, int y, const char *str,
                          uint16_t fg, uint16_t bg, int scale)
{
    const int size = 8 * scale;
    int len = strlen(str);
    int max_chars = (LCD_H_RES - x) / size;
    if (len > max_chars) len = max_chars;
    if (len <= 0 || y + size > LCD_V_RES) return;

    const int w = len * size;
    uint16_t *buf = (uint16_t *)heap_caps_malloc(w * size * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!buf) return;

    for (int i = 0; i < len; i++) {
        char c = str[i];
        const uint8_t *glyph = font8x8[(c >= 0x20 && c <= 0x7E) ? c - 0x20 : '?' - 0x20];
        for (int row = 0; row < 8; row++) {
            for (int sy = 0; sy < scale; sy++) {
                uint16_t *dst = &buf[(row * scale + sy) * w + i * size];
                for (int col = 0; col < 8; col++) {
                    uint16_t px = ((glyph[row] >> col) & 1) ? fg : bg;
                    for (int sx = 0; sx < scale; sx++) *dst++ = px;
                }
            }
        }
    }
    esp_lcd_panel_draw_bitmap(panel, x, y, x + w, y + size, buf);
    xSemaphoreTake(s_flush_done, portMAX_DELAY);
    free(buf);
}

/* Preenche a tela inteira com uma cor RGB565 */
void lcd_fill(esp_lcd_panel_handle_t panel, uint16_t color, uint16_t *buf)
{
    for (size_t i = 0; i < LCD_H_RES * LCD_DRAW_LINES; i++) {
        buf[i] = color;
    }
    for (int y = 0; y < LCD_V_RES; y += LCD_DRAW_LINES) {
        int y_end = (y + LCD_DRAW_LINES > LCD_V_RES) ? LCD_V_RES : y + LCD_DRAW_LINES;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_H_RES, y_end, buf));
        xSemaphoreTake(s_flush_done, portMAX_DELAY);  /* espera DMA antes de reutilizar buf */
    }
}

uint16_t *lcd_config(){
    

    uint16_t *buf = (uint16_t *)heap_caps_malloc(LCD_H_RES * LCD_DRAW_LINES * sizeof(uint16_t),
                                     MALLOC_CAP_DMA);
    assert(buf);
    return buf;

}












/**
 * 
 * @file display.cpp
 * @brief Inicialização do SSD1306 e renderização do layout LVGL estático.
 

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
        .control_phase_bytes = 1, // 1 byte extra para indicar Data/Command
        .dc_bit_offset = 6, // Data-Command está na posição 6
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .scl_speed_hz = I2C_CLOCK_SPEED_HZ, 
    }; // !! Todas as configurações são padrão. Não alterar !!
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL; 
    esp_lcd_panel_dev_config_t panel_config = { 
        .reset_gpio_num = OLED_RST_PIN, 
        .bits_per_pixel = 1, // Monocromático (apagado ou aceso)
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
}*/