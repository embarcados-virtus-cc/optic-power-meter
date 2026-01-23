#!/bin/bash

# Inicia backend e frontend (faz setup automático se necessário)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Cria .env se não existir
if [ ! -f .env ] && [ -f env_config.txt ]; then
    cp env_config.txt .env
    echo "Arquivo .env criado"
fi

# Verifica e compila biblioteca se necessário
if [ ! -f sfp-interface/libsfp.so ]; then
    echo "Compilando biblioteca..."
    cd sfp-interface
    make clean && make lib
    cd ..
fi

# Verifica e configura Python se necessário
if [ ! -d .venv ]; then
    echo "Configurando Python..."
    python3 -m venv .venv
    .venv/bin/pip install --upgrade pip --quiet
    .venv/bin/pip install -r requirements.txt --quiet
fi

# Verifica e configura frontend se necessário
if [ ! -d client/node_modules ]; then
    echo "Configurando frontend..."
    if ! command -v pnpm &> /dev/null; then
        echo "Instalando pnpm..."
        sudo npm install -g pnpm
    fi
    cd client
    pnpm install --frozen-lockfile
    cd ..
fi

# Carrega variáveis de ambiente
if [ -f .env ]; then
    export $(cat .env | grep -v '^#' | xargs)
fi

echo "Iniciando backend..."
.venv/bin/uvicorn sfp_api:app --host 0.0.0.0 --port 8000 &
BACKEND_PID=$!

sleep 2

echo "Iniciando frontend..."
cd client
pnpm dev --host 0.0.0.0 --port 3000 &
FRONTEND_PID=$!

echo "Backend: http://localhost:8000"
echo "Frontend: http://localhost:3000"
echo "Pressione Ctrl+C para parar"

trap "kill $BACKEND_PID $FRONTEND_PID 2>/dev/null; exit" INT TERM
wait

