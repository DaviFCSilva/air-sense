// AirSense - Simulação no Wokwi (ESP32)
// Reais no simulador: ESP32, DHT22, OLED SSD1306
// Simulados com potenciômetros: CO2 (SCD40), poeira (GP2Y1010), pressão (BMP280)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------- Pinos ----------
#define DHT_PIN     15
#define POT_CO2     34   // ADC1
#define POT_DUST    35   // ADC1
#define POT_PRESS   32   // ADC1
#define LED_GREEN   25
#define LED_YELLOW  26
#define LED_RED     27
#define BUZZER      33

// ---------- OLED ----------
#define OLED_W 128
#define OLED_H 64
Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, -1);

// ---------- DHT22 ----------
DHT dht(DHT_PIN, DHT22);

// ---------- Limiares (ajuste conforme o referencial teórico) ----------
const float CO2_ATENCAO  = 800;   // ppm
const float CO2_ALERTA   = 1000;  // ppm
const float DUST_ATENCAO = 15;    // ug/m3
const float DUST_ALERTA  = 35;    // ug/m3
const float TEMP_MIN = 20, TEMP_MAX = 26;   // °C (conforto)
const float UMID_MIN = 40, UMID_MAX = 60;   // % (conforto)

// Faixas simuladas dos potenciômetros
float lerCO2()   { return map(analogRead(POT_CO2),   0, 4095, 400, 2000); }
float lerPoeira(){ return analogRead(POT_DUST)  * 150.0 / 4095.0; }          // 0-150 ug/m3
float lerPressao(){ return 950.0 + analogRead(POT_PRESS) * 100.0 / 4095.0; } // 950-1050 hPa

// 0 = OK, 1 = atenção, 2 = alerta
int classificar(float co2, float poeira, float t, float u) {
  int nivel = 0;
  if (co2 > CO2_ALERTA || poeira > DUST_ALERTA) nivel = 2;
  else if (co2 > CO2_ATENCAO || poeira > DUST_ATENCAO) nivel = 1;
  if (nivel < 1 && (t < TEMP_MIN || t > TEMP_MAX || u < UMID_MIN || u > UMID_MAX)) nivel = 1;
  return nivel;
}

void apitar() {
  for (int i = 0; i < 200; i++) {      // ~2 kHz por ~100 ms
    digitalWrite(BUZZER, HIGH); delayMicroseconds(250);
    digitalWrite(BUZZER, LOW);  delayMicroseconds(250);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  analogReadResolution(12);

  dht.begin();
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED nao encontrado");
    while (true) delay(1000);
  }
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("AirSense");
  display.println("Iniciando...");
  display.display();
  delay(1500);
}

void loop() {
  float t = dht.readTemperature();
  float u = dht.readHumidity();
  if (isnan(t) || isnan(u)) { t = 0; u = 0; }

  float co2    = lerCO2();
  float poeira = lerPoeira();
  float pressao = lerPressao();
  int nivel = classificar(co2, poeira, t, u);

  digitalWrite(LED_GREEN,  nivel == 0);
  digitalWrite(LED_YELLOW, nivel == 1);
  digitalWrite(LED_RED,    nivel == 2);
  if (nivel == 2) apitar();

  const char* status[] = {"OK", "ATENCAO", "ALERTA"};

  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("AirSense  [%s]\n", status[nivel]);
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  display.setCursor(0, 14); display.printf("Temp: %.1f C\n", t);
  display.setCursor(0, 24); display.printf("Umid: %.0f %%\n", u);
  display.setCursor(0, 34); display.printf("CO2 : %.0f ppm\n", co2);
  display.setCursor(0, 44); display.printf("Poeira: %.0f ug/m3\n", poeira);
  display.setCursor(0, 54); display.printf("Press: %.0f hPa\n", pressao);
  display.display();

  Serial.printf("T=%.1fC U=%.0f%% CO2=%.0fppm PM=%.0fug/m3 P=%.0fhPa nivel=%d\n",
                t, u, co2, poeira, pressao, nivel);

  delay(1000);
}
