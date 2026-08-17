# DEPLOY — Despliegue en la Raspberry Pi

## Desde la PC de desarrollo (SSH)

El flujo recomendado es compilar **en la Pi** (el código se edita en la PC):

```bash
# 1. Actualizar el código en la Pi
ssh pi@<ip> "cd AHT21B_bh1750 && git pull --ff-only"

# 2. Compilar y probar remoto
ssh pi@<ip> "cd AHT21B_bh1750 && make clean && make -j4 && ./bin/App --version"
```

## Copiar un binario ya compilado (sin repo en la Pi)

```bash
scp bin/App pi@<ip>:/tmp/
ssh pi@<ip> "sudo /tmp/App --version"      # la versión va grabada en el binario
ssh pi@<ip> "sudo /tmp/App"                # con OLED necesita root (/dev/mem)
```

> **Importante**: la versión se inyecta en compilación (`-DVERSION`), así que
> un binario copiado reporta la versión con la que se construyó, aunque en la
> Pi no exista el archivo `VERSION`.

## Instalación permanente

```bash
sudo make install          # copia bin/App a /usr/local/bin/App
```

## Configuración en producción

- Ajustar `config/config.cfg` (intervalos y sensores habilitados).
- Ejecutar como servicio (ej. systemd) si se quiere arranque automático:
  `ExecStart=/usr/local/bin/App`, `User=pi`.
