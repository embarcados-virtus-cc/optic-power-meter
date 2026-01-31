# Setup e Uso do Daemon SFP

Este guia explica como compilar, instalar e usar o daemon SFP passo a passo.

## Pré-requisitos

### Dependências do Sistema

1. **cJSON**: Biblioteca para serialização JSON
   ```bash
   # Debian/Ubuntu
   sudo apt-get update
   sudo apt-get install libcjson-dev

   # Arch Linux
   sudo pacman -S libcjson
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
   sudo apt-get install build-essential
   ```

## Compilação

### Compilar o Daemon

```bash
cd packages/sfp-interface
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

### Exemplo de Resposta

```
STATUS 200 OK
{"status":"ok","state":"PRESENT","generation_id":1,"timestamps":{"first_detected":1704067200,"last_a0_read":1704067200,"last_a2_read":1704067250},"a0":{"valid":true,"identifier":3,"vendor_name":"CISCO","vendor_pn":"SFP-10G-SR","wavelength_nm":850},"a2":{"valid":true,"temperature_c":45.2,"voltage_v":3.3,"tx_power_dbm":-2.5,"rx_power_dbm":-8.3,"alarms":{"temp_alarm_high":false,"temp_alarm_low":false,"rx_power_alarm_low":false}}}
```

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
        
        # Ler JSON
        json_data = sock.recv(4096).decode().strip()
        data = json.loads(json_data)
        return data
    finally:
        sock.close()

# Exemplos
current = send_command("GET CURRENT")
print(f"Estado: {current['state']}")
print(f"Generation ID: {current['generation_id']}")

if current['state'] == 'PRESENT':
    print(f"Vendor: {current['a0']['vendor_name']}")
    print(f"TX Power: {current['a2']['tx_power_dbm']} dBm")
    print(f"RX Power: {current['a2']['rx_power_dbm']} dBm")
```

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

