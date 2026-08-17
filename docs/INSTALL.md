# INSTALL — Instalación

## Requisitos

- Raspberry Pi (toda la familia BCM2835/BCM2711) con Raspberry Pi OS
  **32-bit (armhf)** o **64-bit (arm64)**.
- ~30 MB de disco e internet para la primera instalación.

## 1. Instalar dependencias

```bash
cd AHT21B_bh1750
sudo bash scripts/install_deps.sh
```

El script instala `build-essential`, `git`, `wget` y la librería **bcm2835**
v1.71 (descargada y compilada desde la fuente oficial en `/usr/local`). Es
idempotente: si `bcm2835.h`/`libbcm2835.a` ya existen, los omite.

## 2. Habilitar I2C en el sistema (si no está)

```bash
# En Raspberry Pi OS: raspi-config > Interface Options > I2C > Enable
sudo raspi-config
# O por config.txt:
echo "dtparam=i2c_arm=on" | sudo tee -a /boot/config.txt
sudo reboot
```

## 3. Compilar e instalar

```bash
make clean && make -j4          # genera bin/App
sudo make install               # copia a /usr/local/bin/App (opcional)
```

## 4. Configurar el repositorio Git (opcional)

```bash
bash scripts/setup_git.sh       # usa gh si está instalado, o guía el paso manual
```

Ver [BUILD.md](BUILD.md) para más detalles de compilación y
[SETUP.md](SETUP.md) para la puesta en marcha.
