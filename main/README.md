# ESP32 PTPMS - Portable Tire Pressure Monitoring System
*Aferidor de Pressão para Pneus Portátil*

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.4.3-red.svg)](https://docs.espressif.com/projects/esp-idf/en/v5.4.3/esp32/)
[![LVGL](https://img.shields.io/badge/LVGL-8.x-blue.svg)](https://lvgl.io/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)

🇺🇸 **[English Version](#english-version)** | 🇧🇷 **[Versão em Português](#versão-em-português)**

---

## 🇺🇸 English Version

### About the Project
This project is a portable Tire Pressure Monitoring System (PTPMS) developed as the final project for the "Projeto Integrador II" course in the Embedded Systems gradutaion program at FATEC Jundiaí. It reads real-time pressure data from a hydrostatic sensor, converts it to readable units (PSI and BAR), and displays the information on an integrated OLED screen with a rich graphical interface. 

Designed with IoT and portability in mind, the system is powered by an onboard 18650 Li-Ion battery, making it a fully autonomous handheld device. It also features auditory and visual alarms to notify the user if the tire pressure is above or below safe limits (28 to 36 PSI).

### Hardware Specifications
*   **Microcontroller:** ESP32 with Integrated OLED and 18650 Battery Shield
    *   **CPU:** Xtensa® Dual-Core 32-bit LX6 @ 240 MHz
    *   **Memory:** 4 MB Flash | 520 KB RAM | 448 KB ROM
    *   **Connectivity:** Wi-Fi 802.11 b/g/n (2.4GHz) & Bluetooth Low Energy (BLE) 4.2
    *   **I/O:** 11 Digital GPIOs (all supporting up to 16-bit PWM)
*   **Display:** Integrated 0.96" OLED
*   **Power System:** Integrated 18650 Li-Ion battery holder with USB charging circuit.
*   **Pressure Sensor:** SMP3011 (I2C Hydrostatic Sensor)
*   **Actuators/Inputs:** Piezo Buzzer for alarms and a Push-button for unit toggling.

### Software Stack
*   **Framework:** ESP-IDF v5.4.3 (C/C++)
*   **Graphics Engine:** LVGL (Light and Versatile Graphics Library) - utilized for rendering text, numbers, dynamic arrows, and shapes.
*   **Architecture:** Modularized C code, utilizing FreeRTOS tasks and hardware interrupts (ISR) for debounce control.

---

## 🇧🇷 Versão em Português

### Sobre o Projeto
Este projeto é um Aferidor de Pressão  Portatil para Pneus (PTPMS), desenvolvido como requisito para aprovação na disciplina de Projeto Integrador II do cursoTecnólogo em Sistemas Embarcados da FATEC Jundiaí. O sistema lê dados em tempo real de um sensor hidrostático, converte para grandezas legíveis (PSI e BAR) e exibe as informações em uma tela OLED com interface gráfica dinâmica.

Projetado com foco em IoT e portabilidade, o circuito utiliza um sistema de alimentação autônomo baseado em uma bateria 18650, tornando-o um dispositivo móvel completo. O aparelho também conta com alertas visuais e sonoros para notificar o usuário caso a calibração esteja fora da faixa segura (28 a 36 PSI).

### Especificações de Hardware
*   **Microcontrolador:** Placa ESP32 com Display OLED e Suporte para Bateria 18650
    *   **Processador:** Xtensa® Dual-Core 32-bit LX6 (Clock máximo: 240 MHz)
    *   **Memória:** 4 MB Flash programável | 520 KBytes RAM | 448 KBytes ROM
    *   **Conectividade:** Wireless 802.11 b/g/n (2.4GHz) e Bluetooth Low Energy 4.2 integrados
    *   **I/O:** 11 Pinos Digitais GPIO (todos com PWM de resolução ajustável até 16 bits)
*   **Display:** OLED integrado (utilizado para renderizar textos, números, formas e ícones)
*   **Alimentação:** Sistema de alimentação e carregamento integrado via bateria Lítio-Ion 18650 e cabo USB.
*   **Sensor de Pressão:** SMP3011 (Sensor Hidrostático via protocolo I2C)
*   **Atuadores/Entradas:** Piezo Buzzer para alarmes sonoros e Botão físico para seleção de grandezas.

### Tecnologias Utilizadas
*   **Framework:** ESP-IDF v5.4.3 (C/C++)
*   **Motor Gráfico:** LVGL (Light and Versatile Graphics Library)
*   **Arquitetura:** Código modularizado em C, com o uso de tasks do FreeRTOS e interrupções de hardware (ISR) para controle de debounce.

---

## 👥 Authors / Autores
*   **Felipe Nascimento**
*   **Gustavo Copette**
*   **Iago de Oliveira**

*Special thanks to TEX equipamentos eletrônicos for providing hardware subsidies and technical instruction during the development of this prototype.*