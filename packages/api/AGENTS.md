# Configurações para Desenvolvimento Contínuo da API

## Estrutura do Projeto

A API é uma aplicação Python usando FastAPI que consome o daemon SFP via socket UNIX e expõe endpoints HTTP REST para a aplicação frontend.

### Arquitetura

```
api/
├── app/
│   ├── __init__.py
│   ├── main.py              # Entry point FastAPI
│   ├── config.py            # Configurações da API
│   ├── daemon_client.py     # Cliente socket UNIX para daemon
│   └── routes/
│       └── current.py       # Rota /api/current
├── requirements.txt         # Dependências Python
├── README.md               # Documentação de uso
└── AGENTS.md               # Este arquivo
```

## Padrões de Código

### Nomenclatura

- Módulos: `snake_case` (ex: `daemon_client.py`)
- Classes: `PascalCase` (ex: `DaemonClient`)
- Funções: `snake_case` (ex: `get_current_data`)
- Constantes: `UPPER_SNAKE_CASE` (ex: `DEFAULT_SOCKET_PATH`)

### Estrutura de Rotas

- Rotas organizadas em módulos dentro de `app/routes/`
- Cada rota em arquivo separado
- Prefixo `/api` para todos os endpoints

### Tratamento de Erros

- Usar `HTTPException` do FastAPI para erros HTTP
- Status codes apropriados:
  - `200`: Sucesso
  - `404`: SFP não encontrado
  - `500`: Erro interno
  - `503`: Daemon indisponível
- Sempre retornar JSON estruturado

### Cliente Socket

- Classe `DaemonClient` para comunicação com daemon
- Métodos assíncronos usando `asyncio`
- Timeout configurável para operações de socket
- Tratamento de erros de conexão e parsing

### Logging

- Usar `logging` padrão do Python
- Nível INFO para operações normais
- Nível ERROR para erros
- Nível DEBUG para desenvolvimento

## Dependências

- `fastapi`: Framework web assíncrono
- `uvicorn`: Servidor ASGI
- `pydantic`: Validação de dados e modelos
- `python-multipart`: Para uploads (se necessário)

## Execução

### Desenvolvimento

```bash
# Instalar dependências
pip install -r requirements.txt

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

## Protocolo de Comunicação com Daemon

### Comandos Suportados

- `GET CURRENT`: Estado completo (A0h + A2h + metadados)
- `GET STATIC`: Apenas dados A0h (estáticos)
- `GET DYNAMIC`: Apenas dados A2h (dinâmicos)
- `GET STATE`: Apenas estado da FSM e timestamps
- `PING`: Health check com uptime

### Formato de Resposta do Daemon

```
STATUS <code> <message>
<JSON>
```

Códigos de status:
- `200 OK`: Sucesso
- `404 NOT_FOUND`: SFP ausente
- `500 ERROR`: Erro interno
- `400 BAD_REQUEST`: Comando inválido

### Exemplo de Uso do Cliente

```python
from app.daemon_client import DaemonClient

client = DaemonClient()
data = await client.get_current()
```

## Endpoints

### GET /api/current

Retorna o estado completo do SFP (A0h + A2h + metadados).

**Resposta:**
- `200`: JSON com dados completos
- `404`: SFP não encontrado
- `503`: Daemon indisponível

**Exemplo de resposta:**
```json
{
  "status": "ok",
  "state": "PRESENT",
  "generation_id": 1,
  "timestamps": {
    "first_detected": 1234567890,
    "last_a0_read": 1234567891,
    "last_a2_read": 1234567892
  },
  "a0h": { ... },
  "a2h": { ... }
}
```

## Testes

- Usar `pytest` para testes unitários
- Mock do cliente socket para testes isolados
- Testes de integração com daemon real (opcional)

## Notas Importantes

- A API é stateless - cada requisição consulta o daemon
- O daemon mantém o estado em memória
- Timeout de socket deve ser configurado adequadamente
- Logs devem ser estruturados para facilitar debugging

