# sfp-interface — SFP Daemon

Driver C para leitura de transceptores SFP/SFP+ via I²C conforme norma SFF-8472. Expõe dados via Unix socket com protocolo texto simples. Implementa máquina de estados (FSM) para detecção automática de hot-swap do módulo.

## Pré-requisitos

```bash
# Dependências de sistema (Debian/Raspberry Pi OS)
sudo apt-get update
sudo apt-get install -y build-essential libcjson-dev i2c-tools

# Habilitar I²C via raspi-config
sudo raspi-config
# Interface Options → I2C → Enable

# Usuário no grupo i2c
sudo usermod -aG i2c $USER
# Fazer logout e login novamente
```

Verificar:

```bash
ls -l /dev/i2c-1
# crw-rw---- 1 root i2c ...
```

## Compilação

```bash
cd sfp-interface

# Compilar apenas o daemon (recomendado para produção)
make daemon

# Compilar tudo (daemon + reader standalone)
make all

# Build com debug
make debug

# Limpar
make clean
```

Executável gerado: `sfp-interface/sfp-daemon`

## Instalação

```bash
# Copiar executável
sudo make install-daemon
# Instala em /usr/local/bin/sfp-daemon

# Verificar
sfp-daemon --help
```

## Configuração

Arquivo opcional em `/etc/sfp-daemon.conf`:

```ini
i2c_device=/dev/i2c-1
socket_path=/run/sfp-daemon/sfp.sock
poll_absent_ms=500
poll_present_ms=2000
poll_error_ms=5000
max_i2c_errors=3
max_recovery_attempts=10
max_connections=10
daemonize=true
```

Se o arquivo não existir, valores padrão são usados (ver tabela abaixo).

| Parâmetro | Padrão | Descrição |
|---|---|---|
| `i2c_device` | `/dev/i2c-1` | Path do dispositivo I²C |
| `socket_path` | `/run/sfp-daemon/sfp.sock` | Path do Unix socket |
| `poll_absent_ms` | `500` | Intervalo de detecção quando SFP ausente (ms) |
| `poll_present_ms` | `2000` | Intervalo de leitura A2h quando SFP presente (ms) |
| `poll_error_ms` | `5000` | Intervalo de recuperação em estado de erro (ms) |
| `max_i2c_errors` | `3` | Erros consecutivos antes de entrar em ERROR |
| `max_recovery_attempts` | `10` | Tentativas de recuperação antes de ir para ABSENT |
| `max_connections` | `10` | Conexões simultâneas ao socket |
| `daemonize` | `true` | Fork para background |

## Execução

```bash
# Foreground (debug)
sudo sfp-daemon -f

# Background (produção, requer daemonize=true)
sudo sfp-daemon

# Com arquivo de config específico
sudo sfp-daemon -c /etc/sfp-daemon.conf
```

## Systemd

```ini
# /etc/systemd/system/sfp-daemon.service
[Unit]
Description=SFP Daemon — I2C Reader
After=network.target
Wants=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/sfp-daemon -f
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now sfp-daemon
sudo systemctl status sfp-daemon
sudo journalctl -u sfp-daemon -f
```

## Protocolo do Socket

**Requisição:** `<COMANDO>\n`

**Resposta:** `STATUS <código> <msg>\n` + JSON + `\n`

```bash
# Testar manualmente
echo "GET CURRENT" | nc -U /run/sfp-daemon/sfp.sock

# Ou via Python
python3 - <<'EOF'
import socket, json
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/run/sfp-daemon/sfp.sock")
s.sendall(b"GET CURRENT\n")
data = b""
while True:
    chunk = s.recv(4096)
    if not chunk: break
    data += chunk
    if data.strip().endswith(b"}"): break
lines = data.decode().split("\n", 1)
print(lines[0])                    # STATUS 200 OK
print(json.loads(lines[1])["state"])  # PRESENT / ABSENT / ERROR
s.close()
EOF
```

### Comandos

| Comando | Descrição |
|---|---|
| `GET CURRENT` | Estado FSM + A0h completo + A2h em tempo real |
| `GET STATIC` | Apenas A0h (dados estáticos, lidos uma vez na inserção) |
| `GET DYNAMIC` | Apenas A2h (leituras em tempo real) |
| `GET STATE` | Estado FSM + timestamps sem dados do módulo |
| `PING` | Health check, retorna `{"status":"ok","uptime":<segundos>}` |

### Estrutura de resposta `GET CURRENT`

```json
{
  "status": "ok",
  "state": "PRESENT",
  "generation_id": 1,
  "timestamps": {
    "first_detected": 1704067200,
    "last_a0_read": 1704067200,
    "last_a2_read": 1704067250
  },
  "a0": {
    "valid": true,
    "identifier_type": "SFP/SFP+",
    "connector_type": "LC",
    "vendor_name": "CISCO",
    "vendor_pn": "SFP-10G-SR",
    "vendor_sn": "FNS1234567",
    "vendor_rev": "A",
    "wavelength_nm": 850,
    "cc_base_valid": true,
    "dmi_implemented": true,
    "calibration_type": "Internal",
    "ext_compliance_desc": "100GBASE-SR4 ou 25GBASE-SR"
  },
  "a2": {
    "valid": true,
    "temp_c": 45.2,
    "voltage_v": 3.302,
    "tx_bias_ma": 12.5,
    "tx_power_dbm": -2.5,
    "tx_power_mw": 0.5623,
    "tx_power_uw": 562.3,
    "rx_power_dbm": -8.3,
    "rx_power_mw": 0.1479,
    "rx_power_uw": 147.9,
    "data_ready": true
  }
}
```

Quando SFP ausente:

```json
{
  "status": "not_found",
  "state": "ABSENT",
  "generation_id": 0,
  "a0": { "valid": false },
  "a2": { "valid": false }
}
```

## Máquina de Estados (FSM)

```
          ┌──────────────────────────────────────┐
          │                INIT                  │
          └─────────────────┬────────────────────┘
                            │ (sempre)
                            ▼
          ┌──────────────────────────────────────┐
    ┌────►│              ABSENT                  │◄──────┐
    │     │  poll: 500ms                         │       │
    │     │  detecta 0x50 + 0x51 no I²C          │       │
    │     └─────────────────┬────────────────────┘       │
    │                       │ presença detectada          │
    │                       ▼                             │
    │     ┌──────────────────────────────────────┐       │
    │     │              PRESENT                 │       │
    │     │  lê A0h na entrada                   │       │
    │     │  lê A2h a cada 2s                    │       │
    │     │  verifica presença a cada 5s          │       │
    │     └──────┬───────────────────┬───────────┘       │
    │            │ sem presença      │ erros I²C >= 3     │
    │            ▼                   ▼                    │
    │       ABSENT ◄─────  ┌───────────────────┐         │
    │                      │       ERROR        │─────────┘
    │                      │  tenta recuperar  │  sem presença
    │                      │  até 10x          │  após max tentativas
    │                      └───────────────────┘
    │                              │ recuperou
    │                              ▼
    │                        PRESENT ────────────────────►(loop)
    └─────────────────────────────────────────────────────(ausente)
```

`generation_id` incrementa a cada transição `ABSENT → PRESENT`, permitindo que clientes detectem troca de módulo.

## Estrutura de Código

```
sfp-interface/
├── daemon/
│   ├── daemon_main.c     # Loop principal, main(), daemonize()
│   ├── daemon_config.c/h # Parse de /etc/sfp-daemon.conf
│   ├── daemon_state.c/h  # Estrutura de estado compartilhado (mutex)
│   ├── daemon_fsm.c/h    # Transições da máquina de estados
│   ├── daemon_i2c.c/h    # Detecção de presença, leitura A0h/A2h
│   └── daemon_socket.c/h # Servidor Unix socket, serialização JSON
├── a0h.c / a0h.h         # Parser completo do registrador A0h (256 bytes)
├── a2h.c / a2h.h         # Parser completo do registrador A2h (256 bytes)
├── i2c.c / i2c.h         # Leitura raw I²C (ioctl)
├── sfp_init.c / sfp_init.h
├── defs.h                # Macros de conversão (TEMP_TO_DEGC, BIAS_TO_MA, etc.)
├── Makefile
└── SETUP.md              # Guia detalhado (original)
```

## Norma SFF-8472

O daemon implementa o padrão **SFF-8472 rev 12.4**:

- **A0h (0x50)**: Dados estáticos — identificação, fabricante, compliance codes, comprimento de onda, checksums
- **A2h (0x51)**: Dados dinâmicos (DDM) — temperatura, tensão, corrente de bias, potências TX/RX, alarmes, thresholds

Os campos de calibração (`calibration_type`) suportados são:
- `Internal`: Valores já calibrados pelo módulo
- `External`: Requer aplicação de constantes de calibração do A0h

## Troubleshooting

| Problema | Causa provável | Solução |
|---|---|---|
| `Failed to open I²C device` | I²C não habilitado | `sudo raspi-config → Interface Options → I2C` |
| `ioctl: Permission denied` | Usuário não no grupo i2c | `sudo usermod -aG i2c $USER` + relog |
| SFP não detectado no scan | Módulo desconectado ou sem alimentação | Verificar conexão física e alimentação |
| `pkg-config: libcjson not found` | cJSON não instalado | `sudo apt-get install libcjson-dev` |
| Socket não criado em `/run/sfp-daemon/` | Diretório não existe ou sem permissão | `sudo mkdir -p /run/sfp-daemon && sudo chmod 755 /run/sfp-daemon` |
| Daemon entra em loop ABSENT/PRESENT | Barramento I²C instável | Verificar resistores pull-up, velocidade I²C, cabo |
| `state: "error"` persistente | Muitos erros I²C consecutivos | Aumentar `max_i2c_errors` ou verificar hardware |

## Verificar Funcionamento

```bash
# SFP detectado no barramento
sudo i2cdetect -y 1
# Deve mostrar 50 e 51

# Daemon rodando
sudo systemctl status sfp-daemon

# Testar socket
echo "PING" | nc -U /run/sfp-daemon/sfp.sock

# Ver estado atual
echo "GET STATE" | nc -U /run/sfp-daemon/sfp.sock

# Logs em tempo real
sudo journalctl -u sfp-daemon -f
```
