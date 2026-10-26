# Firmware para o hardware físico

Esta pasta vai receber a versão do firmware que roda na placa real, com os
sensores SCD40, GP2Y1010 e BMP280 no lugar dos potenciômetros da simulação.

Até lá, o código de referência está em
[`simulacao/wokwi/sketch.ino`](../simulacao/wokwi/sketch.ino). As mudanças
previstas para a versão física estão em
[docs/arquitetura.md – Melhorias planejadas](../docs/arquitetura.md#melhorias-planejadas).

## Organização prevista

```
firmware/
└── airsense/
    └── airsense.ino   # o arduino-cli exige que a pasta tenha o mesmo nome do .ino
```

## Gravar na placa (quando existir)

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/airsense
arduino-cli upload  --fqbn esp32:esp32:esp32 -p /dev/ttyUSB0 firmware/airsense
```

No Windows, troque `/dev/ttyUSB0` pela porta COM da placa (por exemplo, `COM3`).
