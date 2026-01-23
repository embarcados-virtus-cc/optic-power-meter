# Optic Power Meter

Power meter óptico usando transceptor SFF-8472.

## Primeira vez na Raspberry Pi

1. Instale dependências do sistema:
```bash
sudo apt update
sudo apt install -y build-essential gcc make i2c-tools libi2c-dev python3 python3-venv python3-pip curl git nodejs npm
sudo npm install -g pnpm
```

2. Habilite I2C:
```bash
sudo raspi-config
# Interface Options → I2C → Enable
sudo usermod -aG i2c $USER
sudo reboot
```

## Uso

Após o setup inicial, **só precisa rodar**:

```bash
./start.sh
```

O script faz tudo automaticamente:
- Compila a biblioteca se necessário
- Configura Python se necessário  
- Configura frontend se necessário
- Inicia backend e frontend

O banco de dados será criado automaticamente quando o backend iniciar.

## URLs

- Frontend: http://localhost:3000
- Backend API: http://localhost:8000
- Docs API: http://localhost:8000/docs

## Estrutura

- `interface/` - Biblioteca C (A0h e A2h)
- `sfp_api.py` - Backend FastAPI
- `view/` - Frontend React
- `env_config.txt` - Configuração com URL do Supabase (senha já codificada)
