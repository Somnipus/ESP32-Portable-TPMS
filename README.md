# ESP32 PTPMS - Portable Tire Pressure Monitoring System
*Aferidor de Pressão para Pneus Portátil*

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.3.5-red.svg)](https://docs.espressif.com/projects/esp-idf/en/v5.3.5/esp32/)
[![LVGL](https://img.shields.io/badge/LVGL-v9.6.0-blue.svg)](https://lvgl.io/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20FreeRTOS-orange.svg)](https://www.freertos.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

🇺🇸 **[English Version](#english-version)** | 🇧🇷 **[Versão em Português](#versão-em-português)**

---

## 🇺🇸 English Version

### About the Project
This project is a portable Tire Pressure Monitoring System (PTPMS) developed for the Projeto Integrador III course in the Embedded Systems Higher Technology program at FATEC Jundiaí. It reads real-time pressure data from a hydrostatic sensor (SMP3011) and temperature data from an environmental sensor (BMP280) over a shared I2C bus, converts readings into clear units (PSI and bar; °C and °F), and displays them on a color TFT LCD screen (ILI9341 via SPI) managed with LVGL v9.6.0.

Designed for portability and real-time responsiveness, the firmware is structured in object-oriented C++ with FreeRTOS tasks and a mutex protecting I2C bus access, alongside audio-visual alerts when pressure falls outside safe limits (28 to 36 PSI).

### Hardware Specifications
*   **Microcontroller Board:** ESP32
    *   **CPU:** Xtensa® Dual-Core 32-bit LX6 @ 240 MHz
    *   **Memory:** 4 MB Flash | 520 KB RAM | 448 KB ROM
    *   **Connectivity:** Wi-Fi 802.11 b/g/n (2.4 GHz) & Bluetooth Low Energy (BLE) 4.2
*   **Display:** Color TFT LCD driven by ILI9341 (240×320 native resolution in landscape mode, RGB565, via SPI2_HOST at 40 MHz with DMA)
*   **Pressure Sensor:** **SMP3011** (I2C hydrostatic gas pressure sensor, address `0x78`).
*   **Temperature / Barometric Sensor:** BMP280 (I2C sensor, address `0x76`, with factory calibration compensation)

### Software Stack
*   **Framework:** ESP-IDF v5.3.5 (C++ OOP)
*   **Graphics Engine:** LVGL v9.6.0 + `esp_lcd_ili9341` v2.1.0 with DMA partial buffering
*   **Architecture:** Modular C++ classes (`SMP3011`, `BMP280`, `Hardware`) and `main.cpp` orchestrating FreeRTOS tasks with an I2C `Mutex`

### 📄 Full Documentation
For an in-depth analysis of our methodology, sensor calculations, pinout, and circuit assembly, please refer to our official technical report:
* [Technical Report - PTPMS (English)](docs/Technical_Report_PTPMS.pdf)

---

## 🇧🇷 Versão em Português

### Sobre o Projeto
Este projeto é um Aferidor Portátil de Pressão de Pneus (PTPMS), desenvolvido para a disciplina de Projeto Integrador III do curso Superior de Tecnologia em Sistemas Embarcados da FATEC Jundiaí. O sistema realiza leituras em tempo real de pressão por meio do sensor hidrostático SMP3011 e de temperatura por meio do sensor BMP280 em um barramento I2C compartilhado, converte os valores para unidades legíveis (PSI e bar; °C e °F) e os apresenta em um display TFT LCD colorido (ILI9341 via SPI) renderizado com LVGL v9.6.0.

Refatorado para C++ Orientado a Objetos com FreeRTOS e proteção do barramento I2C via Mutex, o sistema também conta com sinalização visual por cores e alertas sonoros quando a pressão está fora da faixa segura (28 a 36 PSI).

### Especificações de Hardware
*   **Placa de Desenvolvimento:** ESP32
    *   **Processador:** Xtensa® Dual-Core 32-bit LX6 (até 240 MHz)
    *   **Memória:** 4 MB Flash | 520 KB RAM | 448 KB ROM
    *   **Conectividade:** Wi-Fi 802.11 b/g/n (2,4 GHz) e Bluetooth Low Energy (BLE) 4.2
*   **Display:** TFT LCD colorido com controlador ILI9341 (240×320 nativo em paisagem, RGB565, via SPI2_HOST a 40 MHz com DMA)
*   **Sensor de Pressão:** SMP3011 (Sensor hidrostático via I2C, endereço `0x78`)
*   **Sensor de Temperatura / Pressão Atmosférica:** BMP280 (via I2C, endereço `0x76`, com compensação pelos coeficientes de calibração internos)

### Tecnologias Utilizadas
*   **Framework:** ESP-IDF v5.3.5 (C++ POO)
*   **Motor Gráfico:** LVGL v9.6.0 + `esp_lcd_ili9341` v2.1.0 com double buffering parcial via DMA
*   **Arquitetura:** Classes modulares em C++ (`SMP3011`, `BMP280`, `Hardware`) e `main.cpp` com tarefas do FreeRTOS e Mutex no barramento I2C

### 📄 Documentação Completa
Para uma análise detalhada da metodologia, cálculos dos sensores, mapeamento de pinos e montagem do circuito, consulte o relatório técnico oficial:
* [Relatório Técnico - PTPMS (Português)](docs/Relatorio_Tecnico_PTPMS.pdf)

---

## 👥 Authors / Autores
*   **Felipe Nascimento**
*   **Gustavo Copette**
*   **Iago de Oliveira**

*Special thanks to TEX equipamentos eletrônicos for providing hardware subsidies and technical instruction during the development of this prototype.*