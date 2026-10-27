// =============================================================================
// config.h — Configurações do AirSense
//
// Tudo que pode ser ajustado sem mexer na lógica fica aqui: modo de operação,
// pinos, limiares de qualidade do ar, tempos e rede.
// =============================================================================
#pragma once

// ---------- Modo de operação ----------
// 1 = simulação no Wokwi: potenciômetros fazem o papel do SCD40, GP2Y1010 e BMP280
// 0 = hardware real (leitura dos sensores reais ainda não implementada)
#define MODO_SIMULACAO 1

// ---------- Pinos ----------
#define PIN_DHT           15
#define PIN_POT_CO2       34   // ADC1_CH6 (somente entrada)
#define PIN_POT_POEIRA    35   // ADC1_CH7 (somente entrada)
#define PIN_POT_PRESSAO   32   // ADC1_CH4
#define PIN_LED_VERDE     25
#define PIN_LED_AMARELO   26
#define PIN_LED_VERMELHO  27
#define PIN_BUZZER        33   // PWM pelo periférico LEDC
#define PIN_I2C_SDA       21
#define PIN_I2C_SCL       22
// Todos os sinais analógicos ficam no ADC1: o ADC2 não funciona com o Wi-Fi ligado.

// ---------- Display OLED ----------
#define OLED_LARGURA   128
#define OLED_ALTURA    64
#define OLED_ENDERECO  0x3C

// ---------- Limiares ----------
// Referência: ANVISA, Resolução RE nº 9/2003 (qualidade do ar interior em
// ambientes climatizados de uso coletivo), conforme a seção 2.4 do roteiro.
// "ALERTA" = limite da norma; "ATENCAO" = pré-alerta em 80% do limite.
const float CO2_ATENCAO  = 800;    // ppm   (80% do limite)
const float CO2_ALERTA   = 1000;   // ppm   (RE 9/2003: <= 1000 ppm)
const float POEIRA_ATENCAO = 64;   // ug/m3 (80% do limite)
const float POEIRA_ALERTA  = 80;   // ug/m3 (RE 9/2003: aerodispersóides <= 80 ug/m3)
const float TEMP_MIN = 23, TEMP_MAX = 26;   // °C (RE 9/2003, condições de verão)
const float UMID_MIN = 40, UMID_MAX = 65;   // %  (RE 9/2003, condições de verão)

// ---------- Tempos ----------
const unsigned long INTERVALO_LEITURA_MS   = 1000;   // lê sensores e atualiza a tela
const unsigned long INTERVALO_ENVIO_MS     = 5000;   // envia uma leitura para a API
const unsigned long DURACAO_BIPE_MS        = 100;
const unsigned int  FREQ_BIPE_HZ           = 2000;
const unsigned long TIMEOUT_WIFI_MS        = 15000;  // espera máxima ao conectar
const unsigned long INTERVALO_RECONEXAO_MS = 30000;  // sem Wi-Fi: tenta de novo só a cada 30 s
const unsigned int  TIMEOUT_HTTP_MS        = 5000;

// ---------- Rede ----------
// Credenciais e URL da API ficam em secrets.h, que NÃO vai para o Git.
// Copie secrets.example.h para secrets.h e preencha. Sem secrets.h, o
// AirSense funciona normalmente, só que sem enviar dados.
#if __has_include("secrets.h")
  #include "secrets.h"
#endif

#ifndef WIFI_SSID
  #define WIFI_SSID "Wokwi-GUEST"   // rede virtual do Wokwi, sem senha
#endif
#ifndef WIFI_SENHA
  #define WIFI_SENHA ""
#endif
#ifndef API_URL
  #define API_URL ""                // vazio = não envia dados
#endif
#ifndef API_KEY
  #define API_KEY ""
#endif
#ifndef DEVICE_ID
  #define DEVICE_ID "airsense-01"
#endif

// No Wokwi, informar o canal 6 pula a varredura de redes e economiza ~4 s.
// No hardware real, 0 deixa o ESP32 procurar o canal sozinho.
#if MODO_SIMULACAO
  #define WIFI_CANAL 6
#else
  #define WIFI_CANAL 0
#endif
