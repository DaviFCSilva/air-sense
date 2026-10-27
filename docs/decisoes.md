# Registro de decisões técnicas

Por que o projeto é como é. Cada decisão registra o contexto, a escolha e o
que foi deixado de lado. Ao mudar alguma delas, adicione uma entrada nova em
vez de apagar a antiga.

---

### D1 · ESP32 DevKit C como microcontrolador
**Contexto:** era preciso ler vários sensores, mostrar dados em display e, no
futuro, enviar dados pela rede.
**Decisão:** ESP32. Tem Wi-Fi integrado, ADC de 12 bits, I2C/SPI/UART, é barato
e é suportado pelo Wokwi.
**Por que não um Arduino Uno:** não tem Wi-Fi e tem pouca memória para HTTPS.

### D2 · Limites da ANVISA RE nº 9/2003
**Contexto:** a primeira versão usava limites de origens diferentes (poeira
15/35 µg/m³, temperatura 20–26 °C).
**Decisão:** seguir a norma brasileira para ambientes climatizados, a mesma
citada no roteiro: CO2 ≤ 1000 ppm, aerodispersóides ≤ 80 µg/m³, 23–26 °C e
40–65 % de umidade. O nível ATENÇÃO fica em 80 % do limite.
**Consequência:** firmware (`config.h`), painel (`api/static/index.html`) e
`enviar_teste.py` precisam ter os mesmos valores. Ao mudar um, mude os três.

### D3 · Sinais analógicos só no ADC1
**Contexto:** o ADC2 do ESP32 não funciona com o Wi-Fi ligado.
**Decisão:** potenciômetros (e, no futuro, o GP2Y1010) nos GPIO 32, 34 e 35, que
pertencem ao ADC1.

### D4 · Falha do DHT22 não vira zero
**Contexto:** a versão inicial trocava leituras inválidas por 0 °C / 0 %, o que
disparava ATENÇÃO falso.
**Decisão:** manter `NaN`, mostrar `--` no display, enviar `null` para a API e
deixar temperatura e umidade fora da classificação até o sensor voltar.

### D5 · Loop sem `delay()`
**Contexto:** com o envio para a API, um `delay(1000)` mais o POST travavam a
tela e prolongavam o bipe.
**Decisão:** cada tarefa (ler, bipar, enviar) tem o seu intervalo medido com
`millis()`. O envio espera o bipe terminar. Reconexões de Wi-Fi acontecem no
máximo a cada 30 s.
**Próximo passo:** trocar o intervalo de leitura por um timer com interrupção.

### D6 · Buzzer por PWM (LEDC)
**Contexto:** o bipe era gerado alternando o pino em software, ocupando a CPU.
**Decisão:** usar o periférico LEDC (`ledcWriteTone`), que gera a onda em
hardware. O código trata as APIs das versões 2.x e 3.x do core do ESP32.

### D7 · Credenciais fora do código (`secrets.h`)
**Contexto:** o repositório é público, e o código de Wi-Fi tinha a chave da
API escrita no próprio arquivo.
**Decisão:** credenciais em `secrets.h`, que está no `.gitignore`, com um modelo
em `secrets.example.h`. Sem o arquivo, o dispositivo funciona normalmente, só
não envia dados. Do lado da API, a chave fica no `api/.env`.

### D8 · API em Python (FastAPI + SQLite)
**Contexto:** era preciso receber as leituras, guardar o histórico e mostrar um
painel, de forma fácil de rodar em qualquer computador do grupo.
**Decisão:** FastAPI, que gera documentação automática em `/docs` e valida os
dados, com SQLite, que dispensa servidor de banco. O painel é uma página HTML
servida pela própria API.
**Alternativas:** n8n ou Node-RED (menos código, mas mais difícil de versionar e
testar) e PostgreSQL (desnecessário para o volume do projeto).
**Segurança:** só a gravação exige chave. A chave é comparada em tempo
constante, e o servidor recusa gravações se estiver sem chave configurada.

### D9 · Túnel Cloudflare para o Wokwi alcançar a API
**Contexto:** no wokwi.com, o ESP32 sai pela rede `Wokwi-GUEST` e não alcança o
`localhost` do computador. O gateway privado do site é pago.
**Decisão:** `cloudflared tunnel --url http://localhost:8000`, que é gratuito e
não exige conta. No VS Code, o gateway da extensão permite usar
`host.wokwi.internal` sem túnel.
**Limitação:** a URL muda a cada execução do túnel. O `https` usa
`setInsecure()`, aceitável em simulação, mas não em produção.
