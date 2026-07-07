# API — SFP Optic Power Meter

Backend REST em FastAPI. Faz ponte entre o `sfp-daemon` (Unix socket) e os clientes (GUI web, display, exportação de dados). Persiste histórico de leituras no MongoDB e expõe endpoints de gerenciamento de containers Docker.

## Tecnologias

| Lib | Versão | Uso |
|---|---|---|
| FastAPI | 0.115.5 | Framework REST |
| uvicorn | 0.34.0 | ASGI server |
| pymongo | 4.10.1 | Cliente MongoDB |
| pydantic | 2.10.4 | Validação / serialização |
| docker | 7.1.0 | SDK Docker (gerenciamento de containers) |

## Estrutura

```
api/
├── main.py             # Todos os endpoints, socket client, mappers
├── database/
│   ├── __init__.py     # Exports: get_mongo_client, CurrentReading, HistoryPoint, etc.
│   ├── models.py       # Modelos Pydantic (CurrentReading, HistoryPoint, ReadingBase)
│   └── migrations.py   # Criação de índices TTL e campos
├── requirements.txt
└── Dockerfile
```

## Variáveis de Ambiente

| Variável | Padrão | Descrição |
|---|---|---|
| `MONGO_URI` | `mongodb://admin:admin123@mongo:27017/optic_power_meter?authSource=admin` | URI de conexão MongoDB |
| `SFP_DAEMON_SOCKET` | `/run/sfp-daemon/sfp.sock` | Caminho do Unix socket do daemon |
| `SFP_SOCKET_TIMEOUT` | `3` | Timeout de conexão ao socket (segundos) |
| `CONTAINER_API_KEY` | `` (vazio) | Chave para endpoints destrutivos de containers (deixar vazio = sem auth) |

## Executar Localmente (desenvolvimento)

```bash
cd api
python -m venv .venv
source .venv/bin/activate        # Linux/Mac
# .venv\Scripts\activate         # Windows

pip install -r requirements.txt
uvicorn main:app --reload --port 8000
```

Swagger UI: `http://localhost:8000/docs`

## Docker

```bash
# Build da imagem
docker build -t optic-api ./api

# Executar (requer daemon rodando e socket acessível)
docker run --rm \
  -v /run/sfp-daemon:/run/sfp-daemon \
  -v /var/run/docker.sock:/var/run/docker.sock \
  -e MONGO_URI="mongodb://admin:admin123@localhost:27017/optic_power_meter?authSource=admin" \
  -p 8000:8000 \
  optic-api
```

Via docker compose (recomendado):

```bash
docker compose up api
```

## Protocolo do Daemon

A API comunica com o `sfp-daemon` via Unix socket em `/run/sfp-daemon/sfp.sock`.

**Formato de requisição:** `<COMANDO>\n`

**Formato de resposta:**
```
STATUS 200 OK\n
{ ... JSON ... }\n
```

**Comandos suportados:**

| Comando | Descrição |
|---|---|
| `GET CURRENT` | Estado completo (A0h + A2h) |
| `GET STATIC` | Apenas A0h (dados estáticos do módulo) |
| `GET DYNAMIC` | Apenas A2h (leituras em tempo real) |
| `GET STATE` | Estado FSM + timestamps |
| `PING` | Health check, retorna uptime do daemon |

**Campos A2h retornados pelo daemon:**

```json
{
  "temp_c": 45.2,
  "voltage_v": 3.302,
  "tx_bias_ma": 12.5,
  "tx_power_dbm": -2.5,
  "tx_power_mw": 0.562,
  "tx_power_uw": 562.0,
  "rx_power_dbm": -8.3,
  "rx_power_mw": 0.148,
  "rx_power_uw": 148.0,
  "data_ready": true
}
```

**Estados FSM do daemon:**

| Estado | Significado |
|---|---|
| `ABSENT` | SFP não detectado no I²C. Resposta: `status: "not_found"` |
| `PRESENT` | SFP detectado e lendo normalmente. Resposta: `status: "ok"` |
| `ERROR` | Falhas I²C consecutivas, tentando recuperar. Resposta: `status: "error"` |

## Cache e Concorrência

O socket é **single-threaded** no daemon. Para evitar contenção:

- **Cache interno** (`_socket_cache`): TTL de 5 segundos por comando. Múltiplas requisições simultâneas retornam do cache sem abrir nova conexão.
- **Lock global** (`_socket_lock`): Garante que apenas uma goroutine/thread acessa o socket por vez.
- **Retry automático**: 3 tentativas com delays de 0.3s, 0.6s antes de retornar 503.

## Background Sampler

Task assíncrona que roda a cada 5 segundos:

1. Envia `GET CURRENT` ao daemon
2. Se SFP ausente (`status: "not_found"` ou `"error"`), pula a inserção
3. Mapeia a resposta para `CurrentReading`
4. Insere no MongoDB

Isso garante histórico contínuo mesmo sem ninguém abrindo o frontend.

## Endpoints

### Leitura SFP

```
GET /health
```
```json
{
  "status": "ok",
  "db_connected": true,
  "daemon_ok": true
}
```

---

```
GET /api/v1/current
```
Retorna `CurrentReading` mapeado (campos normalizados):
```json
{
  "timestamp": "2024-01-01T12:00:00+00:00",
  "rx_power_dbm": -8.3,
  "tx_power_dbm": -2.5,
  "temperature_c": 45.2,
  "voltage_v": 3.302,
  "bias_ma": 12.5,
  "signal_quality": "OK",
  "module": {
    "vendor_name": "CISCO",
    "vendor_pn": "SFP-10G-SR",
    "connector_type": "LC",
    "wavelength_nm": 850
  }
}
```

---

```
GET /api/v1/raw/current     # Resposta raw do daemon (GET CURRENT)
GET /api/v1/raw/dynamic     # Resposta raw (GET DYNAMIC / A2h)
GET /api/v1/raw/static      # Resposta raw (GET STATIC / A0h)
GET /api/v1/raw/state       # FSM state + timestamps
GET /api/v1/ping            # PING ao daemon
GET /api/v1/debug/all       # Todos os comandos em uma requisição
```

---

### Histórico

```
GET /api/v1/history?limit=30
```
Retorna array de `{ timestamp, rx_power_dbm }` ordenado por tempo crescente.

```
GET /api/v1/export/csv
```
Download CSV com todas as leituras.

---

### Containers Docker

```
GET  /api/v1/containers                     # Lista todos os containers
GET  /api/v1/containers/{name}/stats        # CPU% e RAM
GET  /api/v1/containers/{name}/logs?lines=100
POST /api/v1/containers/{name}/start        # Requer X-Api-Key header
POST /api/v1/containers/{name}/stop         # Requer X-Api-Key header
POST /api/v1/containers/{name}/restart      # Requer X-Api-Key header
```

Para endpoints com autenticação, enviar header:
```
X-Api-Key: <valor de CONTAINER_API_KEY>
```

Se `CONTAINER_API_KEY` estiver vazio, autenticação é desabilitada.

## MongoDB

**Collection:** `readings`

**Documento:**
```json
{
  "_id": "ObjectId",
  "timestamp": "2024-01-01T12:00:00+00:00",
  "created_at": "2024-01-01T12:00:00+00:00",
  "rx_power_dbm": -8.3,
  "tx_power_dbm": -2.5,
  "temperature_c": 45.2,
  "voltage_v": 3.302,
  "bias_ma": 12.5,
  "signal_quality": "OK",
  "module": { ... }
}
```

**Índices criados automaticamente nas migrations:**
- `timestamp` (descrescente) — queries de histórico
- TTL opcional para limpeza automática de dados antigos

## Troubleshooting

| Erro | Causa | Solução |
|---|---|---|
| `503 Socket error` | Daemon não rodando ou socket ausente | `sudo systemctl status sfp-daemon` |
| `502 Missing JSON body` | Daemon ocupado, timeout de leitura | Aumentar `SFP_SOCKET_TIMEOUT` |
| `db_connected: false` | MongoDB inacessível | `docker compose logs mongo` |
| `daemon_ok: false` no `/health` | Socket file não existe | Verificar se daemon está rodando |
| Docker SDK indisponível | `/var/run/docker.sock` não montado | Verificar volumes no docker-compose.yml |
