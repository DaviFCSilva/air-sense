"""Envia leituras falsas para a API, sem precisar do Wokwi.

Útil para testar a API e o painel. Usa só a biblioteca padrão do Python.

Exemplos:
    python enviar_teste.py                      # 20 leituras, uma por segundo
    python enviar_teste.py --quantidade 100 --intervalo 0.2
    python enviar_teste.py --url https://SEU-TUNEL.trycloudflare.com/leituras
"""

import argparse
import json
import math
import os
import random
import time
import urllib.error
import urllib.request
from pathlib import Path


def ler_chave_do_env() -> str:
    """Lê API_KEY de api/.env, sem precisar do python-dotenv."""
    arquivo = Path(__file__).resolve().parent / ".env"
    if arquivo.exists():
        for linha in arquivo.read_text(encoding="utf-8").splitlines():
            if linha.strip().startswith("API_KEY="):
                return linha.split("=", 1)[1].strip()
    return os.getenv("API_KEY", "")


def classificar(co2: float, poeira: float, temp: float, umid: float) -> int:
    """Mesma regra do firmware (firmware/airsense/alertas.h)."""
    if co2 > 1000 or poeira > 80:
        return 2
    if co2 > 800 or poeira > 64:
        return 1
    if not (23 <= temp <= 26 and 40 <= umid <= 65):
        return 1
    return 0


def gerar_leitura(passo: int, device: str) -> dict:
    # CO2 sobe e desce como numa sala que enche e esvazia
    co2 = 650 + 450 * (0.5 + 0.5 * math.sin(passo / 6)) + random.uniform(-25, 25)
    poeira = max(0.0, 30 + 25 * math.sin(passo / 9 + 1) + random.uniform(-5, 5))
    temp = 24.5 + 1.8 * math.sin(passo / 11) + random.uniform(-0.2, 0.2)
    umid = 55 + 8 * math.sin(passo / 13 + 2) + random.uniform(-1, 1)
    pressao = 1013 + random.uniform(-1.5, 1.5)
    return {
        "device": device,
        "temperatura": round(temp, 1),
        "umidade": round(umid, 1),
        "co2": round(co2),
        "poeira": round(poeira, 1),
        "pressao": round(pressao, 1),
        "nivel": classificar(co2, poeira, temp, umid),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--url", default="http://localhost:8000/leituras")
    parser.add_argument("--chave", default=None, help="padrão: API_KEY do api/.env")
    parser.add_argument("--device", default="simulador-01")
    parser.add_argument("--quantidade", type=int, default=20)
    parser.add_argument("--intervalo", type=float, default=1.0, help="segundos entre envios")
    args = parser.parse_args()

    chave = args.chave or ler_chave_do_env()
    if not chave:
        raise SystemExit("Chave não encontrada: crie o api/.env ou use --chave")

    for passo in range(args.quantidade):
        leitura = gerar_leitura(passo, args.device)
        requisicao = urllib.request.Request(
            args.url,
            data=json.dumps(leitura).encode(),
            headers={"Content-Type": "application/json", "x-api-key": chave},
            method="POST",
        )
        try:
            with urllib.request.urlopen(requisicao, timeout=10) as resposta:
                print(f"{passo + 1:>3}  {resposta.status}  {leitura}")
        except urllib.error.HTTPError as erro:
            print(f"{passo + 1:>3}  {erro.code}  {erro.read().decode()}")
        except urllib.error.URLError as erro:
            raise SystemExit(f"Não consegui falar com {args.url}: {erro.reason}")
        time.sleep(args.intervalo)


if __name__ == "__main__":
    main()
