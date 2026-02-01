# API SFP Power Meter

API REST em Python (FastAPI) que consome o daemon SFP via socket UNIX e expõe endpoints HTTP para a aplicação.

## Instalação

```bash
# Instalar dependências
pip install -r requirements.txt
```

## Execução

### Desenvolvimento

```bash
# Executar servidor de desenvolvimento
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

### Produção

```bash
# Usar gunicorn com workers uvicorn
gunicorn app.main:app -w 4 -k uvicorn.workers.UvicornWorker --bind 0.0.0.0:8000
```

## Configuração

A API lê configurações de variáveis de ambiente:

- `SFP_DAEMON_SOCKET`: Caminho do socket UNIX (padrão: `/run/sfp-daemon/sfp.sock`)
- `API_HOST`: Host do servidor (padrão: `0.0.0.0`)
- `API_PORT`: Porta do servidor (padrão: `8000`)
- `SOCKET_TIMEOUT`: Timeout para operações de socket em segundos (padrão: `5.0`)

## Endpoints

### GET /api/current

Retorna o estado completo do SFP (A0h + A2h + metadados).

**Resposta:**
- `200`: JSON com dados completos
- `404`: SFP não encontrado
- `503`: Daemon indisponível

### GET /

Health check básico da API.

### GET /health

Health check detalhado incluindo status do daemon.

## Documentação

A documentação interativa da API está disponível em:

- Swagger UI: `http://localhost:8000/docs`
- ReDoc: `http://localhost:8000/redoc`

## Estrutura

```
api/
├── app/
│   ├── __init__.py
│   ├── main.py              # Entry point FastAPI
│   ├── config.py            # Configurações
│   ├── daemon_client.py     # Cliente socket UNIX
│   └── routes/
│       └── current.py       # Rota /api/current
├── requirements.txt
└── README.md
```

