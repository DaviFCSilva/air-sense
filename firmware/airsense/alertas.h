// =============================================================================
// alertas.h — Classificação do ar e sinalização (LEDs e buzzer)
// =============================================================================
#pragma once
#include "config.h"
#include "sensores.h"

enum Nivel { NIVEL_OK = 0, NIVEL_ATENCAO = 1, NIVEL_ALERTA = 2 };
const char* const NOME_NIVEL[] = {"OK", "ATENCAO", "ALERTA"};

// Regras:
//  1. ALERTA  se CO2 ou poeira passam do limite da norma
//  2. ATENCAO se CO2 ou poeira passam de 80% do limite
//  3. ATENCAO se, estando OK, temperatura ou umidade saem da faixa de conforto
// Com o DHT22 em falha, temperatura e umidade ficam de fora da classificação.
Nivel classificar(const Leitura& l) {
  if (l.co2 > CO2_ALERTA || l.poeira > POEIRA_ALERTA) return NIVEL_ALERTA;
  if (l.co2 > CO2_ATENCAO || l.poeira > POEIRA_ATENCAO) return NIVEL_ATENCAO;
  if (l.dhtOk && (l.temperatura < TEMP_MIN || l.temperatura > TEMP_MAX ||
                  l.umidade < UMID_MIN || l.umidade > UMID_MAX)) {
    return NIVEL_ATENCAO;
  }
  return NIVEL_OK;
}

// ---------- Buzzer por PWM (periférico LEDC do ESP32) ----------
// O LEDC gera a onda quadrada em hardware, então a CPU fica livre durante o
// bipe. A API do LEDC mudou na versão 3 do core do ESP32; os dois casos são
// tratados abaixo.
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  #define LEDC_BUZZER PIN_BUZZER          // core 3.x: endereça pelo pino
#else
  #define LEDC_BUZZER 0                   // core 2.x: endereça pelo canal
#endif

static bool buzzerLigado = false;
static unsigned long inicioBipe = 0;

void iniciarAlertas() {
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PIN_BUZZER, FREQ_BIPE_HZ, 8);
#else
  ledcSetup(LEDC_BUZZER, FREQ_BIPE_HZ, 8);
  ledcAttachPin(PIN_BUZZER, LEDC_BUZZER);
#endif
  ledcWriteTone(LEDC_BUZZER, 0);
}

void bipar() {
  ledcWriteTone(LEDC_BUZZER, FREQ_BIPE_HZ);
  buzzerLigado = true;
  inicioBipe = millis();
}

bool buzzerTocando() { return buzzerLigado; }

// Chamada a cada volta do loop: desliga o buzzer quando o bipe termina
void atualizarBuzzer() {
  if (buzzerLigado && millis() - inicioBipe >= DURACAO_BIPE_MS) {
    ledcWriteTone(LEDC_BUZZER, 0);
    buzzerLigado = false;
  }
}

// Acende o LED do nível atual e, em ALERTA, dispara um bipe
void sinalizar(Nivel n) {
  digitalWrite(PIN_LED_VERDE,    n == NIVEL_OK);
  digitalWrite(PIN_LED_AMARELO,  n == NIVEL_ATENCAO);
  digitalWrite(PIN_LED_VERMELHO, n == NIVEL_ALERTA);
  if (n == NIVEL_ALERTA) bipar();
}
