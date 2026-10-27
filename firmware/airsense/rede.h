// =============================================================================
// rede.h — Wi-Fi e envio das leituras para a API
//
// A cada INTERVALO_ENVIO_MS, a leitura atual é enviada por HTTP POST, em JSON,
// para API_URL, com a chave no cabeçalho "x-api-key". O contrato completo
// está em api/README.md.
// =============================================================================
#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "config.h"
#include "sensores.h"
#include "alertas.h"

static unsigned long ultimaTentativaWiFi = 0;

bool apiConfigurada() { return strlen(API_URL) > 0; }
bool wifiConectado()  { return WiFi.status() == WL_CONNECTED; }

// Tenta conectar por até TIMEOUT_WIFI_MS. Usada no setup e nas reconexões.
bool conectarWiFi() {
  ultimaTentativaWiFi = millis();
  Serial.printf("Conectando ao Wi-Fi \"%s\"", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA, WIFI_CANAL);
  unsigned long inicio = millis();
  while (!wifiConectado() && millis() - inicio < TIMEOUT_WIFI_MS) {
    delay(250);
    Serial.print(".");
  }
  if (wifiConectado()) {
    Serial.printf("\nWi-Fi conectado! IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  }
  Serial.println("\nSem Wi-Fi: seguindo sem enviar dados");
  return false;
}

// Monta o JSON da leitura. Se o DHT22 falhou, temperatura e umidade vão como
// null, porque NaN não é um valor válido em JSON.
void montarJson(char* buf, size_t tam, const Leitura& l, Nivel n) {
  char temp[12] = "null", umid[12] = "null";
  if (l.dhtOk) {
    snprintf(temp, sizeof(temp), "%.1f", l.temperatura);
    snprintf(umid, sizeof(umid), "%.1f", l.umidade);
  }
  snprintf(buf, tam,
           "{\"device\":\"%s\",\"temperatura\":%s,\"umidade\":%s,"
           "\"co2\":%.0f,\"poeira\":%.1f,\"pressao\":%.1f,\"nivel\":%d}",
           DEVICE_ID, temp, umid, l.co2, l.poeira, l.pressao, (int)n);
}

void enviarParaAPI(const Leitura& l, Nivel n) {
  if (!apiConfigurada()) return;

  if (!wifiConectado()) {
    // Reconectar trava o loop por até 15 s, então só tentamos a cada 30 s.
    if (millis() - ultimaTentativaWiFi < INTERVALO_RECONEXAO_MS) return;
    if (!conectarWiFi()) return;
  }

  char json[256];
  montarJson(json, sizeof(json), l, n);

  HTTPClient http;
  WiFiClientSecure clienteSeguro;
  WiFiClient clienteSimples;

  if (strncmp(API_URL, "https://", 8) == 0) {
    // Simulação/protótipo: aceita qualquer certificado. Em produção, validar
    // o certificado do servidor com clienteSeguro.setCACert(...).
    clienteSeguro.setInsecure();
    http.begin(clienteSeguro, API_URL);
  } else {
    http.begin(clienteSimples, API_URL);
  }
  http.setTimeout(TIMEOUT_HTTP_MS);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", API_KEY);

  int codigo = http.POST((uint8_t*)json, strlen(json));
  if (codigo > 0) {
    // 201 = gravado; 401 = chave errada; 422 = dados fora do formato esperado
    Serial.printf("POST %s -> %d\n", API_URL, codigo);
  } else {
    Serial.printf("POST %s falhou: %s\n", API_URL, http.errorToString(codigo).c_str());
  }
  http.end();
}
