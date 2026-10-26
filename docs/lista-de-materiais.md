# Lista de materiais

Componentes do AirSense. A coluna **Na simulação** mostra como cada peça aparece
no Wokwi hoje. Os sensores de CO2, poeira e pressão não existem no simulador,
então são representados por potenciômetros.

## Placa e microcontrolador

| Item | Qtd | Na simulação | Principais características |
|---|---|---|---|
| ESP32 DevKit C V4 (ESP32-WROOM-32) | 1 | Real (`board-esp32-devkit-c-v4`) | Dual-core até 240 MHz, Wi-Fi e Bluetooth, ADC de 12 bits, I2C/SPI/UART, alimentação por USB |

## Sensores

| Item | Qtd | Grandeza | Interface | Na simulação | Especificações relevantes |
|---|---|---|---|---|---|
| DHT22 (AM2302) | 1 | Temperatura e umidade | Digital, 1 fio | Real (`wokwi-dht22`) | −40 a 80 °C (±0,5 °C); 0 a 100 % UR (±2–5 %) |
| Sensirion SCD40 | 1 | CO2 | I2C | Potenciômetro no GPIO 34 (400–2000 ppm) | Fotoacústico; 400–2000 ppm, ±(50 ppm + 5 %) |
| Sharp GP2Y1010AU0F | 1 | Poeira (material particulado) | Analógica | Potenciômetro no GPIO 35 (0–150 µg/m³) | Alimentação de 5 V; ~0,5 V a cada 0,1 mg/m³; exige pulso no LED IR |
| Bosch BMP280 | 1 | Pressão atmosférica | I2C ou SPI | Potenciômetro no GPIO 32 (950–1050 hPa) | 300–1100 hPa (±1 hPa) |

## Saídas e interface

| Item | Qtd | Na simulação | Observação |
|---|---|---|---|
| Display OLED SSD1306 128×64 (I2C) | 1 | Real (`board-ssd1306`) | Endereço `0x3C` |
| LED 5 mm verde | 1 | Real | Status OK |
| LED 5 mm amarelo | 1 | Real | Status ATENÇÃO |
| LED 5 mm vermelho | 1 | Real | Status ALERTA |
| Resistor 220 Ω | 3 | Real | Limitação de corrente dos LEDs |
| Buzzer | 1 | Real (`wokwi-buzzer`) | Alarme sonoro no nível ALERTA |

## Componentes de apoio (montagem física)

| Item | Qtd | Uso |
|---|---|---|
| Protoboard de 830 pontos | 1 | Montagem do protótipo |
| Jumpers macho-macho e macho-fêmea | ~30 | Ligações |
| Resistor de 150 Ω + capacitor de 220 µF | 1 + 1 | Circuito de pulso do LED do GP2Y1010 (conforme o datasheet) |
| Resistores para divisor de tensão | 2 | Adaptar a saída do GP2Y1010 à faixa de 0–3,3 V do ADC |
| Cabo micro-USB e fonte de 5 V | 1 | Alimentação e gravação |

## Fonte dos recursos

A versão simulada não tem custo. Se o grupo montar o protótipo físico, os
componentes serão comprados com recursos próprios dos integrantes ou
reaproveitados de materiais que já possuem, conforme a seção 2.5 do Roteiro de
Extensão.
