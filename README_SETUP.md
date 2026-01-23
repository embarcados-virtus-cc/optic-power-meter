# Setup do Optic Power Meter - Raspberry Pi 5

Este guia explica como configurar o Optic Power Meter na sua Raspberry Pi 5.

## Pré-requisitos

- Raspberry Pi 5 com Raspberry Pi OS (ou similar)
- Módulo SFP conectado via I2C
- Acesso à internet para baixar dependências

## Setup Automatizado (Recomendado)

Execute o script de setup:

```bash
sudo ./setup.sh
```

O script irá:
1. ✅ Instalar dependências do sistema (build tools, I2C, Python, Node.js)
2. ✅ Habilitar I2C no boot
3. ✅ Configurar permissões I2C
4. ✅ Compilar a biblioteca C (`libsfp.so`)
5. ✅ Configurar ambiente Python (venv + dependências)
6. ✅ Configurar frontend (pnpm + dependências)
7. ✅ Criar arquivo `.env` de exemplo
8. ✅ Criar script `start.sh` para iniciar tudo

### Após o Setup

1. **Configure o banco de dados** (opcional, para Supabase):
   ```bash
   nano .env
   ```
   Adicione sua URL do Supabase:
   ```
   OPM_DATABASE_URL=postgresql+psycopg://user:password@host:5432/dbname
   ```

2. **Reinicie o sistema** (para aplicar configurações I2C):
   ```bash
   sudo reboot
   ```

3. **Após reiniciar, inicie os serviços**:
   ```bash
   ./start.sh
   ```

   Ou inicie manualmente:
   ```bash
   # Terminal 1 - Backend
   .venv/bin/uvicorn sfp_api:app --host 0.0.0.0 --port 8000

   # Terminal 2 - Frontend
   cd client && pnpm dev --host 0.0.0.0 --port 3000
   ```

4. **Acesse a dashboard**:
   - Frontend: http://localhost:3000
   - Backend API: http://localhost:8000
   - Documentação API: http://localhost:8000/docs

## Setup Manual

Se preferir fazer manualmente:

### 1. Instalar Dependências do Sistema

```bash
sudo apt update
sudo apt install -y build-essential gcc make i2c-tools libi2c-dev python3 python3-venv python3-pip curl git
```

### 2. Habilitar I2C

```bash
sudo raspi-config
# Interface Options → I2C → Enable
```

Ou edite `/boot/firmware/config.txt` (ou `/boot/config.txt`):
```
dtparam=i2c_arm=on
```

Adicione seu usuário ao grupo i2c:
```bash
sudo usermod -aG i2c $USER
```

Reinicie:
```bash
sudo reboot
```

### 3. Compilar Biblioteca C

```bash
cd sfp-interface
make clean
make lib
cd ..
```

### 4. Configurar Python

```bash
python3 -m venv .venv
.venv/bin/pip install --upgrade pip
.venv/bin/pip install -r requirements.txt
```

### 5. Configurar Frontend

```bash
# Instalar Node.js 20.x (se necessário)
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo bash -
sudo apt install -y nodejs

# Instalar pnpm
sudo npm install -g pnpm

# Instalar dependências
cd client
pnpm install --frozen-lockfile
cd ..
```

### 6. Configurar Variáveis de Ambiente

Crie um arquivo `.env`:
```bash
cp .env.example .env  # Se existir
nano .env
```

## Verificar I2C

Verifique se o módulo SFP está conectado:

```bash
sudo i2cdetect -y 1
```

Você deve ver os endereços `50` e `51` ocupados (0x50 = A0h, 0x51 = A2h).

## Serviço Systemd (Opcional)

Para iniciar o backend automaticamente no boot:

```bash
sudo systemctl enable optic-power-meter
sudo systemctl start optic-power-meter
sudo systemctl status optic-power-meter
```

## Troubleshooting

### I2C não funciona

1. Verifique se I2C está habilitado:
   ```bash
   lsmod | grep i2c
   ```

2. Verifique permissões:
   ```bash
   groups  # Deve incluir 'i2c'
   ```

3. Teste acesso:
   ```bash
   sudo i2cdetect -y 1
   ```

### Biblioteca não compila

1. Verifique se tem todas as dependências:
   ```bash
   sudo apt install -y build-essential gcc make
   ```

2. Verifique erros de compilação:
   ```bash
   cd sfp-interface
   make clean
   make lib 2>&1 | tee build.log
   ```

### Python não encontra libsfp.so

1. Verifique se a biblioteca existe:
   ```bash
   ls -lh sfp-interface/libsfp.so
   ```

2. Verifique se está compilada para a arquitetura correta:
   ```bash
   file sfp-interface/libsfp.so
   # Deve mostrar: ELF 64-bit LSB shared object, ARM aarch64
   ```

### Frontend não inicia

1. Verifique se Node.js está instalado:
   ```bash
   node --version  # Deve ser v20.x ou superior
   ```

2. Reinstale dependências:
   ```bash
   cd client
   rm -rf node_modules pnpm-lock.yaml
   pnpm install
   ```

## Estrutura do Projeto

```
optic-power-meter/
├── setup.sh              # Script de setup automatizado
├── start.sh              # Script para iniciar serviços
├── .env                  # Variáveis de ambiente (criar)
├── requirements.txt      # Dependências Python
├── sfp_api.py           # Backend FastAPI
├── sfp-interface/        # Biblioteca C
│   ├── sfp_8472_a0h.*   # Página A0h (Base ID)
│   ├── sfp_8472_a2h.*   # Página A2h (Diagnósticos)
│   ├── i2c.*            # Interface I2C
│   └── libsfp.so        # Biblioteca compilada
└── client/               # Frontend React
    ├── package.json
    └── src/
```

## Suporte

Para problemas ou dúvidas, verifique:
- Logs do backend: `tail -f api.log`
- Status do serviço: `sudo systemctl status optic-power-meter`
- Documentação da API: http://localhost:8000/docs

