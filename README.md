# SFP Optic Power Meter

Sistema embarcado de monitoramento de transceptores ópticos SFP/SFP+ para Raspberry Pi. Lê parâmetros críticos via I²C (norma SFF-8472), expõe uma API REST e exibe os dados em um display ST7789 e interface web React.

## Arquitetura

```
┌─────────────────────────────────────────────────────────────────┐
│                         Raspberry Pi                            │
│                                                                 │
│  ┌──────────────┐    Unix Socket     ┌──────────────────────┐  │
│  │  sfp-daemon  │◄──────────────────►│   FastAPI (Docker)   │  │
│  │  (C / I²C)   │  /run/sfp-daemon/  │   porta 8001 local   │  │
│  └──────┬───────┘  sfp.sock          └──────────┬───────────┘  │
│         │ I²C                                    │ HTTP         │
│         ▼                                        ▼              │
│  ┌──────────────┐                    ┌──────────────────────┐  │
│  │  SFP/SFP+    │                    │  Nginx + React GUI   │  │
│  │  0x50 / 0x51 │                    │  (Docker) porta 8080 │  │
│  └──────────────┘                    └──────────────────────┘  │
│                                                                 │
│  ┌──────────────┐    HTTP localhost  ┌──────────────────────┐  │
│  │  Display TUI │◄──────────────────│   MongoDB (Docker)   │  │
│  │  ST7789 SPI  │    porta 8080      │   porta 27017        │  │
│  └──────────────┘                    └──────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### Subsistemas

| Diretório | Linguagem | Função |
|---|---|---|
| `sfp-interface/` | C | Driver I²C, parser SFF-8472, daemon Unix socket |
| `api/` | Python / FastAPI | API REST, persistência MongoDB, gerenciamento Docker |
| `gui/` | TypeScript / React | Dashboard web, gráficos em tempo real, página de sistema |
| `display/` | Python / Pillow | TUI interativa no display ST7789 via SPI |

## Pré-requisitos de Hardware

- Raspberry Pi (qualquer modelo com I²C e SPI)
- Display ST7789 320×240 conectado via SPI
- Módulo SFP/SFP+ com adaptador I²C (endereços `0x50` e `0x51`)
- Teclado USB (opcional, com suporte a hot-plug)

## Instalação Rápida

### 1. Clonar o repositório

```bash
git clone https://github.com/embarcados-virtus-cc/optic-power-meter.git
cd optic-power-meter
```

### 2. Configurar variáveis de ambiente

```bash
cp .env.example .env
nano .env
```

Variáveis disponíveis:

```env
# MongoDB
MONGO_USER=admin
MONGO_PASSWORD=troque_esta_senha
MONGO_URI=mongodb://admin:troque_esta_senha@mongo:27017/optic_power_meter?authSource=admin

# Daemon
SFP_DAEMON_SOCKET=/run/sfp-daemon/sfp.sock
SFP_SOCKET_TIMEOUT=3

# Container management (deixar vazio para desabilitar auth)
CONTAINER_API_KEY=
```

### 3. Compilar e instalar o daemon SFP

```bash
cd sfp-interface
sudo apt-get install -y build-essential libcjson-dev
make daemon
sudo make install-daemon
cd ..
```

```ini
# /etc/systemd/system/sfp-daemon.service
[Unit]
Description=SFP Daemon
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/sfp-daemon -f
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now sfp-daemon
sudo systemctl status sfp-daemon
```

### 4. Habilitar I²C e SPI no Raspberry Pi

```bash
sudo raspi-config
# Interface Options → I2C → Enable
# Interface Options → SPI → Enable
sudo reboot
```

Verificar:

```bash
sudo i2cdetect -y 1
# Deve mostrar 50 e 51 quando SFP conectado
```

### 5. Subir os containers Docker

```bash
docker compose up -d
docker compose ps
# mongo: healthy, api: healthy, gui: started
```

Interface web disponível em: `http://<IP_DO_PI>:8080`

### 6. Instalar e iniciar o display

```bash
cd display
pip install -r requirements.txt
```

```ini
# /etc/systemd/system/display.service
[Unit]
Description=SFP Display TUI
After=network.target sfp-daemon.service

[Service]
Type=simple
WorkingDirectory=/home/pedro/optic-power-meter/display
ExecStart=/usr/bin/python3 main.py
Restart=always
RestartSec=3
Environment=SFP_API_URL=http://localhost:8080/api/v1/raw/current

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now display.service
```

## Estrutura de Diretórios

```
optic-power-meter/
├── sfp-interface/          # Driver C + daemon
│   ├── daemon/             # Daemon (FSM, socket, I²C, config)
│   ├── a0h.c / a0h.h      # Parser registrador A0h (estático)
│   ├── a2h.c / a2h.h      # Parser registrador A2h (dinâmico)
│   ├── i2c.c / i2c.h      # Comunicação I²C raw
│   ├── Makefile
│   ├── SETUP.md            # Guia detalhado do daemon
│   └── README.md
├── api/                    # Backend FastAPI
│   ├── database/           # Modelos, migrações, conexão MongoDB
│   ├── main.py
│   ├── requirements.txt
│   ├── Dockerfile
│   └── README.md
├── gui/                    # Frontend React
│   ├── src/
│   │   ├── components/
│   │   ├── routes/
│   │   ├── stores/
│   │   └── lib/api.ts
│   ├── Dockerfile
│   └── README.md
├── display/                # Display ST7789
│   ├── main.py
│   ├── menu_system.py
│   ├── sfp_reader.py
│   ├── keyboard.py
│   ├── config.py
│   ├── diagnostics.py
│   ├── network.py
│   ├── hardware.py
│   └── README.md
├── docker-compose.yml
├── .env.example
└── README.md
```

## API REST — Endpoints Principais

| Método | Endpoint | Descrição |
|---|---|---|
| GET | `/health` | Status da API e daemon |
| GET | `/api/v1/current` | Leitura atual mapeada |
| GET | `/api/v1/raw/current` | Resposta raw do daemon |
| GET | `/api/v1/history?limit=30` | Histórico de leituras |
| GET | `/api/v1/export/csv` | Export CSV completo |
| GET | `/api/v1/ping` | Health check do daemon |
| GET | `/api/v1/containers` | Lista containers Docker |
| POST | `/api/v1/containers/{name}/restart` | Reinicia container |

Documentação interativa: `http://<IP>:8001/docs`

## Hot-swap

- **SFP**: Detecção automática pelo daemon (FSM `ABSENT ↔ PRESENT ↔ ERROR`). Sem necessidade de reiniciar nenhum serviço.
- **Teclado USB**: Re-scan a cada 3 segundos. Plug/unplug detectado automaticamente; banner de aviso aparece/some no display.

## Licença

MIT — Virtus CC / Embarcados
