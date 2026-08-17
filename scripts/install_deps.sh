#!/usr/bin/env bash
#
# install_deps.sh - instala las dependencias del proyecto AHT21B_bh1750.
#
# Instala el toolchain básico (g++, make, git) y la librería bcm2835
# (v1.71) compilándola desde la fuente oficial de Mike McCauley, de forma que
# funciona igual en Raspberry Pi OS de 32 bits (armhf) y de 64 bits (arm64).
#
# Uso:
#   sudo bash scripts/install_deps.sh
#
# La librería queda en /usr/local/include/bcm2835.h y
# /usr/local/lib/libbcm2835.a, que es donde el Makefile la busca por defecto.

set -euo pipefail

BCM2835_VERSION="1.71"
BCM2835_URL="http://www.airspayce.com/mikem/bcm2835/bcm2835-${BCM2835_VERSION}.tar.gz"

# Detecta la arquitectura para informar (no cambia el flujo: bcm2835 compila
# igual en armhf y arm64).
ARCH="$(uname -m)"
echo "==> Arquitectura detectada: ${ARCH} (32-bit armhf o 64-bit arm64 soportados)"

# 1) Paquetes base del sistema.
echo "==> Instalando paquetes base (build-essential, git, wget)..."
apt-get update
apt-get install -y build-essential git wget curl

# 2) bcm2835: compilar e instalar desde la fuente oficial.
if [[ -f /usr/local/include/bcm2835.h && -f /usr/local/lib/libbcm2835.a ]]; then
    echo "==> bcm2835 ya instalado en /usr/local (omitiendo)."
else
    echo "==> Descargando y compilando bcm2835 v${BCM2835_VERSION}..."
    TMP="$(mktemp -d)"
    trap 'rm -rf "${TMP}"' EXIT
    wget -q -O "${TMP}/bcm2835.tar.gz" "${BCM2835_URL}"
    tar -xzf "${TMP}/bcm2835.tar.gz" -C "${TMP}"
    cd "${TMP}/bcm2835-${BCM2835_VERSION}"
    ./configure --disable-docs >/dev/null
    make >/dev/null
    make install >/dev/null
    ldconfig || true
    echo "==> bcm2835 v${BCM2835_VERSION} instalado en /usr/local."
fi

echo "==> Dependencias listas. Compile con:  make clean && make -j4"
echo "    Ejecute la app con:                 ./bin/App --version"
