// =============================================================================
// sensores.h — Leitura dos sensores
//
// DHT22 (temperatura e umidade) é real no Wokwi. CO2, poeira e pressão são
// simulados por potenciômetros enquanto MODO_SIMULACAO = 1.
// =============================================================================
#pragma once
#include <DHT.h>
#include "config.h"

// Uma leitura completa do ambiente
struct Leitura {
  float temperatura;  // °C    (NaN se o DHT22 falhar)
  float umidade;      // %     (NaN se o DHT22 falhar)
  float co2;          // ppm
  float poeira;       // ug/m3
  float pressao;      // hPa
  bool  dhtOk;        // false quando a leitura do DHT22 falhou
};

DHT dht(PIN_DHT, DHT22);

void iniciarSensores() {
  analogReadResolution(12);   // ADC de 12 bits: valores de 0 a 4095
  dht.begin();
}

#if MODO_SIMULACAO
// Converte a posição do potenciômetro (0–4095) para a faixa do sensor real
float lerCO2()     { return map(analogRead(PIN_POT_CO2), 0, 4095, 400, 2000); }      // 400–2000 ppm
float lerPoeira()  { return analogRead(PIN_POT_POEIRA) * 150.0f / 4095.0f; }         // 0–150 ug/m3
float lerPressao() { return 950.0f + analogRead(PIN_POT_PRESSAO) * 100.0f / 4095.0f; } // 950–1050 hPa
#else
  // Próximo passo: SCD40 e BMP280 pelo I2C e GP2Y1010 pelo ADC (veja docs/arquitetura.md)
  #error "Leitura dos sensores reais ainda não implementada. Use MODO_SIMULACAO 1 em config.h."
#endif

Leitura lerSensores() {
  Leitura l;
  l.temperatura = dht.readTemperature();
  l.umidade     = dht.readHumidity();
  // Uma falha do DHT22 devolve NaN. Mantemos o NaN e marcamos dhtOk = false,
  // em vez de trocar por 0 (o que geraria um alerta falso).
  l.dhtOk   = !(isnan(l.temperatura) || isnan(l.umidade));
  l.co2     = lerCO2();
  l.poeira  = lerPoeira();
  l.pressao = lerPressao();
  return l;
}
