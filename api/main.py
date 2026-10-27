"""API do AirSense.

Recebe as leituras enviadas pelo ESP32, guarda em um banco SQLite e serve o
painel web que mostra os dados.

Rotas:
    POST /leituras          grava uma leitura (exige o cabeçalho x-api-key)
    GET  /leituras          lista as leituras mais recentes
    GET  /leituras/ultima   devolve a leitura mais recente
    GET  /saude             verificação simples de que a API está no ar
    GET  /                  painel web
    GET  /docs              documentação interativa (gerada pelo FastAPI)
"""

import hmac
import os
import sqlite3
from contextlib import asynccontextmanager, closing
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

from dotenv import load_dotenv
from fastapi import Depends, FastAPI, Header, HTTPException, Query, status
from fastapi.responses import FileResponse
from pydantic import BaseModel, Field

PASTA = Path(__file__).resolve().parent
load_dotenv(PASTA / ".env")

CAMPOS = ("device", "temperatura", "umidade", "co2", "poeira", "pressao", "nivel")


# ---------- Configuração (lida a cada uso, para facilitar os testes) ----------

def caminho_banco() -> Path:
    return Path(os.getenv("AIRSENSE_DB", PASTA / "airsense.db"))


def chave_esperada() -> str:
    return os.getenv("API_KEY", "")


# ---------- Banco de dados ----------

def conectar() -> sqlite3.Connection:
    conexao = sqlite3.connect(caminho_banco())
    conexao.row_factory = sqlite3.Row
    return conexao


def criar_tabela() -> None:
    with closing(conectar()) as conexao, conexao:
        conexao.execute(
            """
            CREATE TABLE IF NOT EXISTS leituras (
                id          INTEGER PRIMARY KEY AUTOINCREMENT,
                recebido_em TEXT    NOT NULL,
                device      TEXT    NOT NULL,
                temperatura REAL,
                umidade     REAL,
                co2         REAL    NOT NULL,
                poeira      REAL    NOT NULL,
                pressao     REAL    NOT NULL,
                nivel       INTEGER NOT NULL
            )
            """
        )
        conexao.execute(
            "CREATE INDEX IF NOT EXISTS idx_leituras_device ON leituras (device, id)"
        )


# ---------- Modelos ----------

class LeituraEntrada(BaseModel):
    """Formato do JSON enviado pelo ESP32 (veja firmware/airsense/rede.h)."""

    device: str = Field(min_length=1, max_length=64, pattern=r"^[A-Za-z0-9_-]+$",
                        examples=["airsense-01"])
    temperatura: Optional[float] = Field(None, ge=-40, le=80, description="°C; null se o DHT22 falhar")
    umidade: Optional[float] = Field(None, ge=0, le=100, description="%; null se o DHT22 falhar")
    co2: float = Field(ge=0, le=40000, description="ppm")
    poeira: float = Field(ge=0, le=1000, description="µg/m³")
    pressao: float = Field(ge=300, le=1100, description="hPa")
    nivel: int = Field(ge=0, le=2, description="0 = OK, 1 = ATENÇÃO, 2 = ALERTA")


class LeituraSalva(LeituraEntrada):
    id: int
    recebido_em: str


# ---------- Autenticação ----------

def exigir_chave(x_api_key: Optional[str] = Header(default=None)) -> None:
    """Aceita a requisição só se o cabeçalho x-api-key bater com o API_KEY do .env."""
    esperada = chave_esperada()
    if not esperada:
        raise HTTPException(status.HTTP_503_SERVICE_UNAVAILABLE,
                            "API_KEY não configurada no servidor (arquivo api/.env)")
    # compare_digest evita que o tempo de resposta revele parte da chave
    if x_api_key is None or not hmac.compare_digest(x_api_key.encode(), esperada.encode()):
        raise HTTPException(status.HTTP_401_UNAUTHORIZED, "Chave de API inválida ou ausente")


# ---------- Aplicação ----------

@asynccontextmanager
async def ciclo_de_vida(_app: FastAPI):
    criar_tabela()
    yield


app = FastAPI(
    title="AirSense API",
    description="Recebe e disponibiliza as leituras de qualidade do ar do AirSense.",
    version="1.0.0",
    lifespan=ciclo_de_vida,
)


@app.post("/leituras", status_code=status.HTTP_201_CREATED, response_model=LeituraSalva,
          dependencies=[Depends(exigir_chave)])
def gravar_leitura(leitura: LeituraEntrada) -> LeituraSalva:
    recebido_em = datetime.now(timezone.utc).isoformat(timespec="seconds")
    dados = leitura.model_dump()
    with closing(conectar()) as conexao, conexao:
        cursor = conexao.execute(
            f"INSERT INTO leituras (recebido_em, {', '.join(CAMPOS)}) "
            f"VALUES (?, {', '.join('?' for _ in CAMPOS)})",
            (recebido_em, *(dados[c] for c in CAMPOS)),
        )
    return LeituraSalva(id=cursor.lastrowid, recebido_em=recebido_em, **dados)


@app.get("/leituras", response_model=list[LeituraSalva])
def listar_leituras(
    device: Optional[str] = Query(None, description="Filtra por dispositivo"),
    limite: int = Query(100, ge=1, le=1000, description="Quantidade máxima, da mais recente para a mais antiga"),
) -> list[LeituraSalva]:
    sql = "SELECT * FROM leituras"
    parametros: list = []
    if device:
        sql += " WHERE device = ?"
        parametros.append(device)
    sql += " ORDER BY id DESC LIMIT ?"
    parametros.append(limite)
    with closing(conectar()) as conexao:
        linhas = conexao.execute(sql, parametros).fetchall()
    return [LeituraSalva(**dict(linha)) for linha in linhas]


@app.get("/leituras/ultima", response_model=LeituraSalva)
def ultima_leitura(device: Optional[str] = Query(None)) -> LeituraSalva:
    leituras = listar_leituras(device=device, limite=1)
    if not leituras:
        raise HTTPException(status.HTTP_404_NOT_FOUND, "Nenhuma leitura recebida ainda")
    return leituras[0]


@app.get("/saude")
def saude() -> dict:
    return {"status": "ok", "chave_configurada": bool(chave_esperada())}


@app.get("/", include_in_schema=False)
def painel() -> FileResponse:
    return FileResponse(PASTA / "static" / "index.html")
