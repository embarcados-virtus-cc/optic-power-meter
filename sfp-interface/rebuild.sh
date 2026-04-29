#!/bin/bash
set -e

SOCKET_PATH="/tmp/sfp_daemon.sock"

echo "Parando sfp-daemon..."
pkill -x sfp-daemon 2>/dev/null || true

echo "Removendo socket $SOCKET_PATH..."
rm -f "$SOCKET_PATH"

echo "Limpando binários..."
make clean

echo "Compilando..."
make all
make daemon

echo "Iniciando sfp-daemon..."
./sfp-daemon &

echo "Pronto."
