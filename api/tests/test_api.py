"""Testes da API do AirSense. Rode com: pytest (dentro da pasta api/)."""

import sys
from pathlib import Path

import pytest
from fastapi.testclient import TestClient

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import main  # noqa: E402

CHAVE = "chave-de-teste"
LEITURA = {
    "device": "airsense-01",
    "temperatura": 24.5,
    "umidade": 55.0,
    "co2": 650,
    "poeira": 20.0,
    "pressao": 1013.0,
    "nivel": 0,
}


@pytest.fixture
def cliente(tmp_path, monkeypatch):
    monkeypatch.setenv("API_KEY", CHAVE)
    monkeypatch.setenv("AIRSENSE_DB", str(tmp_path / "teste.db"))
    with TestClient(main.app) as c:
        yield c


def enviar(cliente, dados=None, chave=CHAVE):
    headers = {} if chave is None else {"x-api-key": chave}
    return cliente.post("/leituras", json=dados or LEITURA, headers=headers)


# ---------- Autenticação ----------

def test_sem_chave_recusa(cliente):
    assert enviar(cliente, chave=None).status_code == 401


def test_chave_errada_recusa(cliente):
    assert enviar(cliente, chave="errada").status_code == 401


def test_servidor_sem_chave_configurada_responde_503(cliente, monkeypatch):
    monkeypatch.delenv("API_KEY")
    assert enviar(cliente).status_code == 503


def test_leitura_nao_exige_chave(cliente):
    assert cliente.get("/leituras").status_code == 200


# ---------- Gravação e consulta ----------

def test_grava_e_lista(cliente):
    resposta = enviar(cliente)
    assert resposta.status_code == 201
    corpo = resposta.json()
    assert corpo["id"] == 1 and corpo["co2"] == 650 and corpo["recebido_em"]

    lista = cliente.get("/leituras").json()
    assert len(lista) == 1 and lista[0]["device"] == "airsense-01"


def test_lista_da_mais_recente_para_a_mais_antiga_e_respeita_limite(cliente):
    for co2 in (500, 600, 700):
        enviar(cliente, {**LEITURA, "co2": co2})
    lista = cliente.get("/leituras", params={"limite": 2}).json()
    assert [l["co2"] for l in lista] == [700, 600]


def test_filtra_por_dispositivo(cliente):
    enviar(cliente, {**LEITURA, "device": "sala-a"})
    enviar(cliente, {**LEITURA, "device": "sala-b"})
    lista = cliente.get("/leituras", params={"device": "sala-b"}).json()
    assert [l["device"] for l in lista] == ["sala-b"]


def test_ultima_leitura(cliente):
    assert cliente.get("/leituras/ultima").status_code == 404
    enviar(cliente, {**LEITURA, "co2": 900, "nivel": 1})
    assert cliente.get("/leituras/ultima").json()["co2"] == 900


def test_aceita_null_quando_dht22_falha(cliente):
    """O firmware manda null em temperatura/umidade quando o DHT22 falha."""
    resposta = enviar(cliente, {**LEITURA, "temperatura": None, "umidade": None})
    assert resposta.status_code == 201
    assert resposta.json()["temperatura"] is None


def test_json_igual_ao_do_firmware(cliente):
    """Mesmo texto que firmware/airsense/rede.h monta com snprintf."""
    bruto = ('{"device":"airsense-01","temperatura":null,"umidade":null,'
             '"co2":1150,"poeira":25.0,"pressao":1012.4,"nivel":2}')
    resposta = cliente.post("/leituras", content=bruto,
                            headers={"x-api-key": CHAVE, "Content-Type": "application/json"})
    assert resposta.status_code == 201


# ---------- Validação ----------

@pytest.mark.parametrize("campo, valor", [
    ("nivel", 3),
    ("co2", -1),
    ("umidade", 120),
    ("pressao", 50),
    ("device", "nome com espaço"),
    ("device", ""),
])
def test_rejeita_valores_invalidos(cliente, campo, valor):
    assert enviar(cliente, {**LEITURA, campo: valor}).status_code == 422


def test_rejeita_campo_obrigatorio_ausente(cliente):
    dados = {k: v for k, v in LEITURA.items() if k != "co2"}
    assert enviar(cliente, dados).status_code == 422


# ---------- Outras rotas ----------

def test_saude(cliente):
    assert cliente.get("/saude").json() == {"status": "ok", "chave_configurada": True}


def test_painel(cliente):
    resposta = cliente.get("/")
    assert resposta.status_code == 200 and "AirSense" in resposta.text
