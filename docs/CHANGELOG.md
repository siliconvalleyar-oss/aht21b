# CHANGELOG

Formato basado en [Keep a Changelog](https://keepachangelog.com/es/1.0.0/).
La versión coincide siempre con el archivo `VERSION` y con el último tag
(`vX.Y.Z`, ver `LEARNINGS.md`).

## [0.1.0] - 2026-08-17

### Añadido
- Esqueleto del proyecto con la estructura de carpetas completa (config,
  docs, include, src, scripts, examples).
- Driver **AHT21B** (temperatura/humedad por I2C, CRC8 0x31 validado).
- Driver **BH1750** (luz ambiente por I2C, modo continua de alta resolución).
- Clase esqueleto **HMC5883L** (magnetómetro, pendiente de implementar).
- Motor **Device::Device_t** con bucle de muestreo, configuración JSON
  (`config/*.cfg`) y soporte de OLED SSD1306 opcional.
- **Versión en tiempo de compilación** (`-DVERSION`) y `--version`/`--help`.
- Makefile con objetivos `all/run/install/clean/distclean/help`, árbol `obj/`
  jerárquico y soporte 32/64 bits.
- Scripts `install_deps.sh` (bcm2835) y `setup_git.sh` (repo Git interactivo).
- Documentación completa en `docs/` (26 archivos).

### Pendiente (ver ROADMAP.md)
- Implementación del magnetómetro HMC5883L.
- Ejemplos de uso en `examples/`.
- Pruebas en hardware real (Pi + sensores) y unit tests del motor.
