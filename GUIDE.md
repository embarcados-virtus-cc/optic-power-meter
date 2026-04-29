# Guia de Configuração — Optic Power Meter (Raspberry Pi)

Este guia cobre a instalação completa do sistema **optic-power-meter** em um Raspberry Pi
do zero: habilitação do I²C, compilação do daemon, instalação como serviço systemd e
uso da TUI.

---

## Sumário

1. [Hardware necessário](#1-hardware-necessário)
2. [Configuração do Raspberry Pi OS](#2-configuração-do-raspberry-pi-os)
3. [Instalar dependências de software](#3-instalar-dependências-de-software)
4. [Clonar o repositório](#4-clonar-o-repositório)
5. [Compilar o sfp-interface](#5-compilar-o-sfp-interface)
6. [Executar o daemon manualmente](#6-executar-o-daemon-manualmente)
7. [Instalar como serviço systemd](#7-instalar-como-serviço-systemd)
8. [Configurar e usar a TUI](#8-configurar-e-usar-a-tui)
9. [Arquivo de configuração do daemon](#9-arquivo-de-configuração-do-daemon)
10. [Resolução de problemas](#10-resolução-de-problemas)

---

## 1. Hardware necessário

| Item | Descrição |
|------|-----------|
| Raspberry Pi | Qualquer modelo com I²C (3B, 3B+, 4, 5, Zero 2W) |
| Módulo SFP/SFP+ | Com suporte a DDM/DOM (SFF-8472) |
| Adaptador SFP para I²C | Ex.: board breakout SFP com pinos SDA (byte A0h→0x50) e SCL |
| Jumpers | Para conexão SDA, SCL, GND, 3.3V entre o Pi e o adaptador |

### Pinout I²C (Raspberry Pi)

```
GPIO 2 (SDA) → SFP SDA
GPIO 3 (SCL) → SFP SCL
GND          → SFP GND
3.3V         → SFP VCC (se o adaptador não tiver regulador próprio)
```

> **Atenção:** Módulos SFP operam a 3.3V. Não conecte ao pino de 5V diretamente.

---

## 2. Configuração do Raspberry Pi OS

Use o **Raspberry Pi OS Lite** (64-bit recomendado).

### 2.1 Habilitar o I²C via raspi-config

```bash
sudo raspi-config
# Interface Options → I2C → Enable
sudo reboot
```

Ou edite diretamente:

```bash
# Adicione ao final de /boot/firmware/config.txt (Pi 5) ou /boot/config.txt (Pi ≤ 4)
dtparam=i2c_arm=on
```

### 2.2 Verificar que o barramento I²C está disponível

Após o reboot:

```bash
ls /dev/i2c-*
# Esperado: /dev/i2c-1
```

Escanear o barramento para confirmar que o módulo SFP está visível (endereços 0x50 e 0x51):

```bash
sudo apt install i2c-tools -y
i2cdetect -y 1
# Deve mostrar 50 e 51 na tabela
```

---

## 3. Instalar dependências de software

```bash
sudo apt update && sudo apt upgrade -y

# Ferramentas de compilação
sudo apt install -y build-essential gcc make

# Biblioteca cJSON (para serialização JSON no daemon)
sudo apt install -y libcjson-dev

# Python 3 e pip (para a TUI)
sudo apt install -y python3 python3-pip python3-venv

# i2c-tools (opcional, para diagnóstico)
sudo apt install -y i2c-tools
```

---

## 4. Clonar o repositório

```bash
git clone <URL_DO_REPOSITORIO> ~/optic-power-meter
cd ~/optic-power-meter
```

---

## 5. Compilar o sfp-interface

```bash
cd ~/optic-power-meter/sfp-interface

# Compilar o leitor standalone (sfp-reader)
make all

# Compilar o daemon (sfp-daemon)
make daemon

# Para limpar os artefatos de compilação
make clean
```

Os binários gerados serão:
- `sfp-interface/sfp-reader` — leitor interativo (modo terminal)
- `sfp-interface/sfp-daemon` — daemon com socket Unix

---

## 6. Executar o daemon manualmente

Crie o diretório do socket antes de iniciar:

```bash
sudo mkdir -p /run/sfp-daemon
sudo chown $USER:$USER /run/sfp-daemon
```

Execute em primeiro plano para ver os logs no terminal:

```bash
cd ~/optic-power-meter/sfp-interface
sudo ./sfp-daemon --foreground
```

Para rodar em um I²C diferente do padrão:

```bash
sudo ./sfp-daemon --foreground --config /etc/sfp-daemon.conf
```

Verifique que o socket foi criado:

```bash
ls -la /run/sfp-daemon/sfp.sock
```

---

## 7. Instalar como serviço systemd

### 7.1 Instalar o binário

```bash
cd ~/optic-power-meter/sfp-interface
sudo make install-daemon
# Instala em /usr/local/bin/sfp-daemon
```

### 7.2 Criar o arquivo de serviço

```bash
sudo nano /etc/systemd/system/sfp-daemon.service
```

Cole o conteúdo:

```ini
[Unit]
Description=SFP Interface Daemon
After=network.target

[Service]
Type=forking
ExecStartPre=/bin/mkdir -p /run/sfp-daemon
ExecStart=/usr/local/bin/sfp-daemon
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

### 7.3 Habilitar e iniciar o serviço

```bash
sudo systemctl daemon-reload
sudo systemctl enable sfp-daemon
sudo systemctl start sfp-daemon

# Verificar status
sudo systemctl status sfp-daemon

# Ver logs em tempo real
sudo journalctl -u sfp-daemon -f
```

---

## 8. Configurar e usar a TUI

### 8.1 Criar ambiente virtual Python

```bash
cd ~/optic-power-meter/tui
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### 8.2 Executar a TUI

```bash
cd ~/optic-power-meter/tui
source .venv/bin/activate
python3 menu.py
```

A TUI se conecta automaticamente ao socket Unix em `/run/sfp-daemon/sfp.sock` e exibe
os dados do módulo SFP em tempo real (temperatura, tensão, TX bias, TX power, RX power em µW e dBm).

---

## 9. Arquivo de configuração do daemon

O daemon lê o arquivo `/etc/sfp-daemon.conf` (criado manualmente se necessário).
Parâmetros disponíveis com seus valores padrão:

```ini
# Dispositivo I²C
i2c_device=/dev/i2c-1

# Caminho do socket Unix
socket_path=/run/sfp-daemon/sfp.sock

# Intervalos de polling (ms)
poll_absent_ms=500
poll_present_ms=2000
poll_error_ms=5000

# Tolerância a erros
max_i2c_errors=3
max_recovery_attempts=10

# Número máximo de conexões simultâneas ao socket
max_connections=10

# Executar como daemon (true/false)
daemonize=true
```

---

## 10. Resolução de problemas

### I²C: Endereço não encontrado (`i2cdetect` vazio)

- Verifique as conexões físicas (SDA, SCL, GND, VCC)
- Confirme que o módulo SFP está inserido corretamente
- Cheque se `dtparam=i2c_arm=on` está no `config.txt` e reiniciou

### Daemon não inicia (erro de socket)

```bash
# Garanta que o diretório do socket existe e tem permissão
sudo mkdir -p /run/sfp-daemon
sudo chown root:root /run/sfp-daemon
sudo chmod 755 /run/sfp-daemon
```

### Erro de compilação: `libcjson not found`

```bash
sudo apt install libcjson-dev
# Ou instale manualmente via:
sudo apt install pkg-config
pkg-config --cflags --libs libcjson
```

### RX power sempre -40 dBm

- Verifique se o módulo SFP suporta DDM (Byte 92 do A0h — DMI Implemented)
- Confirme a fibra óptica conectada na saída do módulo
- O valor `-40 dBm` é o piso quando a potência lida é ≤ 0 µW

### TUI mostra `Connection refused` ou `timeout`

```bash
# Verifique se o daemon está rodando
sudo systemctl status sfp-daemon

# Confirme o caminho do socket
ls -la /run/sfp-daemon/sfp.sock
```
