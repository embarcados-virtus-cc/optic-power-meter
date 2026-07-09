# Display — SFP Optic Power Meter

Interface interativa (TUI) para display ST7789 320×240 via SPI. Controlada por teclado USB. Lê dados do SFP via HTTP (API) com fallback para socket Unix direto. Suporta hot-plug de SFP e teclado sem reinicialização.

## Hardware

| Componente | Conexão |
|---|---|
| Display ST7789 320×240 | SPI0 (CE0) |
| DC | GPIO 25 |
| RST | GPIO 27 |
| BLK (backlight) | GPIO 24 |
| SPI Speed | 40 MHz |
| Teclado USB | USB (qualquer porta) |

Pinos configuráveis em `config.py`.

## Dependências

```bash
pip install -r requirements.txt
```

`requirements.txt`:
```
Adafruit-GPIO
numpy
Pillow
netifaces
spidev
lgpio
evdev
```

> **Nota:** A biblioteca `ST7789` e `Adafruit_GPIO` devem estar presentes em `display/ST7789/`. Consultar o fabricante do display ou usar a versão incluída no repositório.

## Estrutura

```
display/
├── main.py           # Loop principal: SPI, teclado, render 20fps
├── menu_system.py    # TUI: estados, render PIL, input handler
├── sfp_reader.py     # Leitura SFP (HTTP → socket fallback)
├── keyboard.py       # Teclado USB evdev com hot-plug (rescan 3s)
├── config.py         # Pinos GPIO, cores, TTLs, project info
├── diagnostics.py    # CPU, RAM, disco, I²C, serviços systemd/Docker
├── network.py        # IP, gateway, DNS, WiFi, ssid, conectividade
├── hardware.py       # Adaptador lgpio para Adafruit_GPIO
├── assets/
│   ├── virtus-cc.png          # Logo boot splash
│   └── MaterialIcons-Regular.ttf  # Ícones Material Design
└── ST7789/           # Driver SPI do display
```

## Executar

```bash
cd display
python3 main.py
```

Via systemd (recomendado):

```ini
# /etc/systemd/system/display.service
[Unit]
Description=SFP Display TUI
After=network.target sfp-daemon.service

[Service]
Type=simple
WorkingDirectory=/home/<usuario>/optic-power-meter/display
ExecStart=/usr/bin/python3 main.py
Restart=always
RestartSec=3
Environment=SFP_API_URL=http://localhost:8080/api/v1/raw/current
Environment=SFP_API_TIMEOUT=1.5

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now display.service
sudo journalctl -u display.service -f
```

## Variáveis de Ambiente

| Variável | Padrão | Descrição |
|---|---|---|
| `SFP_API_URL` | `http://localhost:8080/api/v1/raw/current` | URL da API para leitura SFP |
| `SFP_API_TIMEOUT` | `1.5` | Timeout HTTP em segundos |

## Telas / Estados

```
BOOT
 └── Splash animado (logo + barra de progresso gradiente) → MAIN_MENU

MAIN_MENU
 ├── Leitura SFP    → SFP_VIEW
 ├── Alertas SFP    → SFP_ALARMS
 ├── Servicos       → SERVICES
 ├── Rede & Debug   → NET_DEBUG
 ├── Scan I2C       → LOADING → I2C_SCAN
 ├── Sistema        → SYS_DIAG
 ├── Config WiFi    → LOADING → WIFI_SCAN → WIFI_INPUT
 ├── Sobre          → ABOUT
 ├── Reiniciar      → SHUTTING_DOWN (reboot)
 └── Desligar       → CONFIRM_SHUTDOWN → SHUTTING_DOWN (poweroff)
```

### Detalhes por tela

| Tela | Auto-refresh | Dados exibidos |
|---|---|---|
| `SFP_VIEW` | 2s (SFP_TTL) | RX Power, TX Bias, TX Power, Temp, Tensão, fabricante, P/N, S/N, λ, conector |
| `SFP_ALARMS` | 2s | Alarmes SFF-8472 com limites (high/low/warn) |
| `SERVICES` | 5s | Estado systemd/Docker: sfp-daemon, GUI, MongoDB, API |
| `NET_DEBUG` | 5s | IP, gateway, DNS, WiFi SSID + sinal, IO rede, URLs de acesso |
| `SYS_DIAG` | 3s | Modelo Pi, temp CPU, freq, throttle, RAM, disco, uptime, SSH |
| `I2C_SCAN` | 10s | Endereços detectados no barramento I²C com descrição |
| `WIFI_SCAN` | 30s | Redes WiFi com sinal, segurança, status (ativa/salva) |

## Controles

| Tecla | Ação |
|---|---|
| ↑ / ↓ | Navegar / rolar |
| ← / → | Coluna no menu principal |
| ENTER | Confirmar / entrar |
| ESC | Voltar / cancelar |
| TAB | (WiFi input) Mostrar/ocultar senha |
| BACKSPACE | (WiFi input) Apagar caractere |

## Leitura do SFP

`sfp_reader.py` tenta em ordem:

1. **HTTP** → `SFP_API_URL` (3 tentativas com delay 0.5s)
2. **Socket Unix** → `/run/sfp-daemon/sfp.sock` (fallback se API não disponível)

Cache local de 0.5s evita leituras redundantes quando múltiplos estados renderizam ao mesmo tempo.

Se o daemon retornar `status: "not_found"` (SFP ausente) ou `"error"`, a tela `SFP_VIEW` exibe "Dispositivo não conectado" e o sampler da API não insere leituras falsas no banco.

## Hot-plug

### Teclado USB

- `keyboard.py` escaneia dispositivos a cada **3 segundos**
- Plug-in: novo thread de leitura iniciado automaticamente
- Unplug: thread detecta erro de I/O e remove o dispositivo da lista
- Banner amarelo "Nenhum teclado USB detectado" aparece/some automaticamente

### SFP

- Detecção delegada ao `sfp-daemon` (FSM com polling a cada 100ms)
- Remoção → `ABSENT` → display mostra "Dispositivo não conectado"
- Inserção → `PRESENT` → display volta a mostrar dados em até 2s

## Configuração (`config.py`)

```python
BOOT_DURATION = 3.2      # Duração do splash em segundos

SFP_TTL  = 2.0           # Refresh leitura SFP
NET_TTL  = 5.0           # Refresh rede
SYS_TTL  = 3.0           # Refresh sistema
I2C_TTL  = 10.0          # Refresh scan I²C
WIFI_TTL = 30.0          # Refresh scan WiFi
SVC_TTL  = 5.0           # Refresh serviços

# Pinos SPI
DC_PIN       = 25
RST_PIN      = 27
BLK_PIN      = 24
SPI_PORT     = 0
SPI_DEVICE   = 0
SPI_SPEED_HZ = 40_000_000
```

Para adicionar desenvolvedor na tela "Sobre":

```python
PROJECT_INFO = {
    "devs": [
        {"name": "Pedro Sousa", "github": "pwsousa"},
        {"name": "Novo Dev",    "github": "username"},
    ],
    ...
}
```

## Troubleshooting

| Problema | Causa | Solução |
|---|---|---|
| Tela preta após boot | BLK_PIN errado ou SPI não habilitado | Verificar `raspi-config → SPI` e pinos em `config.py` |
| "Dispositivo não conectado" sem motivo | API não está respondendo | `curl http://localhost:8080/api/v1/raw/current` |
| Teclado não detectado | `evdev` não instalado ou permissões | `pip install evdev` + `sudo usermod -aG input $USER` |
| Display congelado | Processo morto ou erro fatal | `sudo systemctl status display.service` |
| Botões não respondem | Dispositivo evdev errado detectado | Verificar `keyboard.py → _find_keyboards()`, filtros por nome |
