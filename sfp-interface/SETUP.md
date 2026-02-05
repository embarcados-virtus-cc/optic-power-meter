# Setup e Uso do Daemon SFP

Este guia explica como compilar, instalar e usar o daemon SFP passo a passo.

## Pré-requisitos

### Dependências do Sistema

1. **cJSON**: Biblioteca para serialização JSON
   ```bash
   # Debian
   sudo apt-get update
   sudo apt-get install libcjson-dev

   # Arch Linux
   sudo pacman -S cjson
   ```

2. **I²C habilitado**: O barramento I²C deve estar habilitado no Raspberry Pi
   ```bash
   # Verificar se I²C está habilitado
   lsmod | grep i2c

   # Se não estiver, habilitar via raspi-config
   sudo raspi-config
   # Interface Options -> I2C -> Enable
   ```

3. **Permissões I²C**: Usuário deve estar no grupo `i2c`
   ```bash
   # Adicionar usuário ao grupo i2c
   sudo usermod -aG i2c $USER
   # Logout e login novamente para aplicar
   ```

4. **Ferramentas de compilação**: GCC e Make
   ```bash
   # Debian
   sudo apt-get install build-essential
   
   # Arch Linux
   sudo pacman -S base-devel
   ```

## Compilação

### Compilar o Daemon

```bash
cd sfp-interface
make daemon
```

Isso irá gerar o executável `sfp-daemon` no diretório atual.

### Compilar Tudo

```bash
make all
```

Isso compila tanto o daemon quanto a aplicação standalone (`sfp_reader`).

### Limpar Arquivos de Compilação

```bash
make clean
```

## Instalação

### Instalação Manual

```bash
# Copiar executável
sudo cp sfp-daemon /usr/local/bin/

# Criar diretório do socket (se não existir)
sudo mkdir -p /run/sfp-daemon
sudo chmod 755 /run/sfp-daemon
```

### Instalação via Makefile

```bash
sudo make install-daemon
```

## Configuração (Opcional)

Criar arquivo de configuração em `/etc/sfp-daemon.conf`:

```bash
sudo nano /etc/sfp-daemon.conf
```

Exemplo de conteúdo:

```
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

Se o arquivo não existir, valores padrão são usados.

## Execução

### Modo Foreground (Debug)

Para testar ou debugar, execute em foreground:

```bash
sudo ./sfp-daemon -f
```

Ou:

```bash
sudo ./sfp-daemon --foreground
```

Neste modo, o daemon não faz fork e mantém controle do terminal. Útil para ver logs em tempo real.

### Modo Daemon (Produção)

Para executar como daemon (background):

```bash
sudo ./sfp-daemon
```

O daemon irá:
- Fazer fork e executar em background
- Criar socket em `/run/sfp-daemon/sfp.sock`
- Registrar logs via syslog
- Monitorar continuamente o barramento I²C

### Verificar se Está Rodando

```bash
# Verificar processo
ps aux | grep sfp-daemon

# Verificar socket
ls -l /run/sfp-daemon/sfp.sock

# Ver logs
sudo journalctl -u sfp-daemon -f
# Ou se não estiver usando systemd:
sudo tail -f /var/log/syslog | grep sfp-daemon
```

## Uso do Socket

### Teste Manual com netcat

```bash
# Conectar ao socket
nc -U /run/sfp-daemon/sfp.sock

# Enviar comandos (um por linha):
GET CURRENT
GET STATIC
GET DYNAMIC
GET STATE
PING

# Sair
Ctrl+C
```

### Exemplo de Resposta (GET CURRENT)

```
STATUS 200 OK
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
    "identifier": 3,
    "identifier_type": "SFP/SFP+",
    "ext_identifier": 4,
    "ext_identifier_valid": true,
    "connector": 7,
    "connector_type": "LC",
    "compliance_codes": {
      "byte3_ethernet_infiniband": {
        "eth_10g_base_sr": true,
        "eth_10g_base_lr": false,
        ...
      },
      ...
    },
    "encoding": 1,
    "nominal_rate_mbd": 103,
    "nominal_rate_status": 1,
    "rate_identifier": 0,
    "smf_length_km": 10,
    "smf_length_status": 1,
    "smf_attenuation_db_per_100m": 5.0,
    "om2_length_m": 82,
    "om2_length_status": 1,
    "om1_length_m": 33,
    "om1_length_status": 1,
    "om4_or_copper_length_m": 150,
    "om4_or_copper_length_status": 1,
    "vendor_name": "CISCO",
    "vendor_name_valid": true,
    "ext_compliance_code": 2,
    "ext_compliance_desc": "100GBASE-SR4 ou 25GBASE-SR",
    "vendor_oui": [0, 1, 35],
    "vendor_oui_u32": 291,
    "vendor_oui_valid": true,
    "vendor_pn": "SFP-10G-SR",
    "vendor_pn_valid": true,
    "vendor_rev": "A",
    "variant": 0,
    "wavelength_nm": 850,
    "fc_speed_2_valid": false,
    "cc_base_valid": true,
    "cc_base": 123
  },
  "a2": {
    "valid": true,
    "temperature_valid": true,
    "temperature_c": 45.2,
    "temperature_raw": 11571,
    "voltage_valid": true,
    "voltage_v": 3.3,
    "voltage_raw": 33000,
    "bias_current_valid": true,
    "bias_current_ma": 12.5,
    "bias_current_raw": 6250,
    "tx_power_valid": true,
    "tx_power_dbm": -2.5,
    "tx_power_mw": 0.5623,
    "tx_power_raw": 5623,
    "rx_power_valid": true,
    "rx_power_dbm": -8.3,
    "rx_power_mw": 0.1479,
    "rx_power_raw": 1479,
    "alarms": {
      "temperature": {"high": false, "low": false},
      "voltage": {"high": false, "low": false},
      "bias_current": {"high": false, "low": false},
      "tx_power": {"high": false, "low": false},
      "rx_power": {"high": false, "low": false}
    },
    "warnings": {
      "temperature": {"high": false, "low": false},
      "voltage": {"high": false, "low": false},
      "bias_current": {"high": false, "low": false},
      "tx_power": {"high": false, "low": false},
      "rx_power": {"high": false, "low": false}
    }
  }
}
```

**Nota**: O JSON acima está formatado para legibilidade. Na prática, a resposta vem em uma única linha compacta.

### Script Python de Exemplo

```python
import socket
import json

def send_command(command):
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    try:
        sock.connect("/run/sfp-daemon/sfp.sock")
        sock.sendall(f"{command}\n".encode())
        
        # Ler status line
        status = sock.recv(1024).decode().strip()
        print(f"Status: {status}")
        
        # Ler JSON (pode precisar de múltiplas leituras para JSON grande)
        json_data = b""
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            json_data += chunk
            try:
                # Tenta parsear para ver se está completo
                json.loads(json_data.decode())
                break
            except json.JSONDecodeError:
                continue
        
        data = json.loads(json_data.decode().strip())
        return data
    finally:
        sock.close()

# Exemplos
current = send_command("GET CURRENT")
print(f"Estado: {current['state']}")
print(f"Generation ID: {current['generation_id']}")

if current['state'] == 'PRESENT':
    a0 = current['a0']
    a2 = current['a2']
    
    # Dados A0h
    print(f"Vendor: {a0['vendor_name']}")
    print(f"Part Number: {a0['vendor_pn']}")
    print(f"Identifier Type: {a0['identifier_type']}")
    print(f"Connector: {a0['connector_type']}")
    if a0.get('wavelength_nm'):
        print(f"Wavelength: {a0['wavelength_nm']} nm")
    
    # Dados A2h
    if a2['valid']:
        print(f"Temperature: {a2['temperature_c']} °C")
        print(f"Voltage: {a2['voltage_v']} V")
        print(f"Bias Current: {a2['bias_current_ma']} mA")
        print(f"TX Power: {a2['tx_power_dbm']} dBm ({a2['tx_power_mw']} mW)")
        print(f"RX Power: {a2['rx_power_dbm']} dBm ({a2['rx_power_mw']} mW)")
        
        # Verificar alarmes
        alarms = a2['alarms']
        if any([
            alarms['temperature']['high'] or alarms['temperature']['low'],
            alarms['voltage']['high'] or alarms['voltage']['low'],
            alarms['rx_power']['high'] or alarms['rx_power']['low']
        ]):
            print("ATENÇÃO: Alarmes ativos!")
```

### Estrutura Completa dos Campos JSON

#### GET CURRENT / GET STATIC - Campos A0h

Todos os campos da página A0h (dados estáticos) são retornados:

**Identificação:**
- `identifier`, `identifier_type`: Tipo do módulo (SFP, QSFP, etc)
- `ext_identifier`, `ext_identifier_valid`: Extended Identifier
- `connector`, `connector_type`: Tipo de conector

**Compliance Codes (Bytes 3-10):**
- `compliance_codes.byte3_ethernet_infiniband`: Ethernet 10G e InfiniBand
- `compliance_codes.byte4_escon_sonet`: ESCON e SONET
- `compliance_codes.byte5_sonet`: SONET adicional
- `compliance_codes.byte6_ethernet_1g`: Ethernet 1G
- `compliance_codes.byte7_fc_link_length`: Fibre Channel Link Length
- `compliance_codes.byte8_fc_technology`: FC Technology
- `compliance_codes.byte9_fc_transmission_media`: FC Transmission Media
- `compliance_codes.byte10_fc_channel_speed`: FC Channel Speed

**Características:**
- `encoding`: Tipo de encoding
- `nominal_rate_mbd`, `nominal_rate_status`: Taxa nominal
- `rate_identifier`: Rate Identifier
- `smf_length_km`, `smf_length_status`, `smf_attenuation_db_per_100m`: SMF
- `om2_length_m`, `om2_length_status`: OM2
- `om1_length_m`, `om1_length_status`: OM1
- `om4_or_copper_length_m`, `om4_or_copper_length_status`: OM4/Cobre

**Fabricante:**
- `vendor_name`, `vendor_name_valid`: Nome do fabricante
- `vendor_oui`, `vendor_oui_u32`, `vendor_oui_valid`: OUI
- `vendor_pn`, `vendor_pn_valid`: Part Number
- `vendor_rev`: Revisão
- `ext_compliance_code`, `ext_compliance_desc`: Conformidade estendida

**Outros:**
- `variant`: Variante (óptico, cabo passivo, cabo ativo)
- `wavelength_nm`: Comprimento de onda (se óptico)
- `cable_compliance`: Cable compliance (se cabo)
- `fc_speed_2`, `fc_speed_2_valid`: Fibre Channel Speed 2
- `cc_base`, `cc_base_valid`: Checksum

#### GET CURRENT / GET DYNAMIC - Campos A2h

Todos os campos da página A2h (diagnósticos) são retornados:

**Valores Medidos:**
- `temperature_c`, `temperature_raw`, `temperature_valid`: Temperatura
- `voltage_v`, `voltage_raw`, `voltage_valid`: Tensão
- `bias_current_ma`, `bias_current_raw`, `bias_current_valid`: Corrente de bias
- `tx_power_dbm`, `tx_power_mw`, `tx_power_raw`, `tx_power_valid`: Potência TX
- `rx_power_dbm`, `rx_power_mw`, `rx_power_raw`, `rx_power_valid`: Potência RX

**Alarmes e Warnings:**
- `alarms.temperature`: {high, low}
- `alarms.voltage`: {high, low}
- `alarms.bias_current`: {high, low}
- `alarms.tx_power`: {high, low}
- `alarms.rx_power`: {high, low}
- `warnings.temperature`: {high, low}
- `warnings.voltage`: {high, low}
- `warnings.bias_current`: {high, low}
- `warnings.tx_power`: {high, low}
- `warnings.rx_power`: {high, low}

Para referência completa de todos os campos, consulte a seção "Protocolo do Socket" no arquivo `AGENTS.md`.

## Integração com Systemd

### Criar Service File

Criar arquivo `/etc/systemd/system/sfp-daemon.service`:

```ini
[Unit]
Description=SFP Daemon
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/sfp-daemon
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

### Ativar e Iniciar

```bash
# Recarregar systemd
sudo systemctl daemon-reload

# Habilitar para iniciar no boot
sudo systemctl enable sfp-daemon

# Iniciar serviço
sudo systemctl start sfp-daemon

# Verificar status
sudo systemctl status sfp-daemon

# Ver logs
sudo journalctl -u sfp-daemon -f
```

### Comandos Úteis

```bash
# Parar
sudo systemctl stop sfp-daemon

# Reiniciar
sudo systemctl restart sfp-daemon

# Desabilitar (não iniciar no boot)
sudo systemctl disable sfp-daemon
```

## Troubleshooting

### Daemon não inicia

1. Verificar permissões I²C:
   ```bash
   ls -l /dev/i2c-1
   # Deve mostrar: crw-rw---- 1 root i2c
   ```

2. Verificar se I²C está habilitado:
   ```bash
   sudo i2cdetect -y 1
   # Deve mostrar dispositivos no barramento
   ```

3. Verificar logs:
   ```bash
   sudo journalctl -u sfp-daemon -n 50
   # Ou
   sudo tail -f /var/log/syslog | grep sfp-daemon
   ```

### Socket não encontrado

1. Verificar se daemon está rodando:
   ```bash
   ps aux | grep sfp-daemon
   ```

2. Verificar permissões do diretório:
   ```bash
   ls -ld /run/sfp-daemon
   # Deve ter permissões 755
   ```

3. Verificar se socket foi criado:
   ```bash
   ls -l /run/sfp-daemon/sfp.sock
   ```

### Erros de Compilação

1. Verificar se cJSON está instalado:
   ```bash
   pkg-config --libs libcjson
   ```

2. Se cJSON não for encontrado, instalar:
   ```bash
   sudo apt-get install libcjson-dev
   ```

3. Limpar e recompilar:
   ```bash
   make clean
   make daemon
   ```

### SFP não detectado

1. Verificar conexão física do módulo SFP
2. Verificar se módulo está energizado
3. Testar com aplicação standalone:
   ```bash
   sudo ./sfp_reader /dev/i2c-1
   ```

## Parar o Daemon

### Se executado manualmente

```bash
# Encontrar PID
ps aux | grep sfp-daemon

# Enviar SIGTERM
sudo kill <PID>

# Ou forçar
sudo kill -9 <PID>
```

### Se executado via systemd

```bash
sudo systemctl stop sfp-daemon
```

## Limpeza

Para remover completamente:

```bash
# Parar serviço (se usando systemd)
sudo systemctl stop sfp-daemon
sudo systemctl disable sfp-daemon
sudo rm /etc/systemd/system/sfp-daemon.service
sudo systemctl daemon-reload

# Remover executável
sudo rm /usr/local/bin/sfp-daemon

# Remover socket e diretório
sudo rm /run/sfp-daemon/sfp.sock
sudo rmdir /run/sfp-daemon

# Remover configuração (opcional)
sudo rm /etc/sfp-daemon.conf
```
