// =============================================================================
// secrets.example.h — Modelo das credenciais do AirSense
//
// 1. Copie este arquivo para "secrets.h" (no Wokwi: crie uma aba secrets.h).
// 2. Preencha os valores abaixo.
// O secrets.h está no .gitignore e nunca deve ser enviado para o GitHub.
// =============================================================================
#pragma once

// No Wokwi, use a rede virtual "Wokwi-GUEST" (sem senha).
#define WIFI_SSID   "Wokwi-GUEST"
#define WIFI_SENHA  ""

// URL PÚBLICA da API, terminando em /leituras. "localhost" NÃO funciona no
// wokwi.com: exponha a API com um túnel (veja docs/como-rodar.md).
// No Wokwi do VS Code, dá para usar http://host.wokwi.internal:8000/leituras
#define API_URL     "https://SEU-TUNEL.trycloudflare.com/leituras"

// Mesma chave definida em API_KEY no arquivo api/.env
#define API_KEY     "cole-aqui-a-chave-do-api-env"

// Identificação deste dispositivo no painel
#define DEVICE_ID   "airsense-01"
