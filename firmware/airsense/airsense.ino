/*
 * =============================================================================
 * AirSense — Monitor de qualidade do ar para ambientes internos (ESP32)
 * Projeto extensionista de Microcontroladores · Estácio Ribeirânia · Grupo 12
 * =============================================================================
 *
 * O que faz, a cada segundo:
 *   1. lê temperatura, umidade, CO2, poeira e pressão;
 *   2. classifica o ar em OK / ATENCAO / ALERTA (limites da ANVISA RE 9/2003);
 *   3. acende o LED do nível, apita em ALERTA e atualiza o display e o serial.
 * A cada 5 segundos, envia a leitura para a API pelo Wi-Fi (se configurada).
 *
 * Arquivos do firmware:
 *   airsense.ino       este arquivo: setup, loop e display
 *   config.h           pinos, limiares, tempos e rede
 *   sensores.h         leitura dos sensores
 *   alertas.h          classificação, LEDs e buzzer (PWM)
 *   rede.h             Wi-Fi e envio para a API
 *   secrets.h          credenciais (opcional, fora do Git; veja secrets.example.h)
 *
 * O loop não usa delay(): cada tarefa roda no seu próprio intervalo, medido
 * com millis(), para que o buzzer, a tela e o envio não travem uns aos outros.
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "sensores.h"
#include "alertas.h"
#include "rede.h"

Adafruit_SSD1306 display(OLED_LARGURA, OLED_ALTURA, &Wire, -1);

Leitura leituraAtual;
Nivel nivelAtual = NIVEL_OK;
bool temLeitura = false;
unsigned long ultimaLeitura = 0;
unsigned long ultimoEnvio = 0;

// Desenha as leituras no OLED de 128x64 (6 linhas de texto)
void mostrarNaTela(const Leitura& l, Nivel n) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("AirSense [%s]", NOME_NIVEL[n]);
  if (apiConfigurada()) {
    display.setCursor(122, 0);
    display.print(wifiConectado() ? "W" : "-");   // W = Wi-Fi conectado
  }
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  display.setCursor(0, 14);
  if (l.dhtOk) display.printf("Temp: %.1f C", l.temperatura);
  else         display.print("Temp: -- (erro)");
  display.setCursor(0, 24);
  if (l.dhtOk) display.printf("Umid: %.0f %%", l.umidade);
  else         display.print("Umid: -- (erro)");
  display.setCursor(0, 34); display.printf("CO2 : %.0f ppm", l.co2);
  display.setCursor(0, 44); display.printf("Poeira: %.0f ug/m3", l.poeira);
  display.setCursor(0, 54); display.printf("Press: %.0f hPa", l.pressao);
  display.display();
}

// Uma linha por leitura no monitor serial (115200 baud)
void registrarNoSerial(const Leitura& l, Nivel n) {
  if (!l.dhtOk) Serial.println("Falha na leitura do DHT22");
  char temp[12] = "--", umid[12] = "--";
  if (l.dhtOk) {
    snprintf(temp, sizeof(temp), "%.1f", l.temperatura);
    snprintf(umid, sizeof(umid), "%.0f", l.umidade);
  }
  Serial.printf("T=%sC U=%s%% CO2=%.0fppm PM=%.0fug/m3 P=%.0fhPa nivel=%d (%s)\n",
                temp, umid, l.co2, l.poeira, l.pressao, (int)n, NOME_NIVEL[n]);
}

void setup() {
  Serial.begin(115200);
  iniciarAlertas();
  iniciarSensores();

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ENDERECO)) {
    Serial.println("OLED nao encontrado");
    while (true) delay(1000);
  }
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("AirSense");
  display.println(apiConfigurada() ? "Conectando ao Wi-Fi..." : "Iniciando...");
  display.display();

  if (apiConfigurada()) {
    conectarWiFi();
  } else {
    Serial.println("API_URL vazia: rodando sem enviar dados (veja secrets.example.h)");
    delay(1500);   // tempo para ler a tela de boas-vindas
  }
}

void loop() {
  unsigned long agora = millis();

  // Tarefa 1: ler, classificar e mostrar (a cada INTERVALO_LEITURA_MS)
  if (!temLeitura || agora - ultimaLeitura >= INTERVALO_LEITURA_MS) {
    ultimaLeitura = agora;
    leituraAtual = lerSensores();
    nivelAtual = classificar(leituraAtual);
    temLeitura = true;
    sinalizar(nivelAtual);
    mostrarNaTela(leituraAtual, nivelAtual);
    registrarNoSerial(leituraAtual, nivelAtual);
  }

  // Tarefa 2: encerrar o bipe na hora certa
  atualizarBuzzer();

  // Tarefa 3: enviar para a API (a cada INTERVALO_ENVIO_MS). Espera o bipe
  // acabar, porque o POST pode levar alguns segundos e prolongaria o som.
  if (agora - ultimoEnvio >= INTERVALO_ENVIO_MS && !buzzerTocando()) {
    ultimoEnvio = agora;
    enviarParaAPI(leituraAtual, nivelAtual);
  }
}
