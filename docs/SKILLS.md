# SKILLS — Conocimiento del proyecto

## 1. Visión general

`AHT21B_bh1750` es una aplicación C++17 para Raspberry Pi que lee
**temperatura/humedad** (AHT21B, I2C 0x38) y **luz ambiente** (BH1750, I2C
0x23) usando la librería **bcm2835**, con **OLED SSD1306** opcional
(0x3C). Funciona en Raspberry Pi OS 32-bit y 64-bit.

## 2. Puntos clave

- **Versión en tiempo de compilación**: el Makefile inyecta
  `-DVERSION="$(cat VERSION)"`; el binario muestra `v0.1.0` al iniciar y con
  `--version`. Nunca se lee el archivo VERSION en runtime.
- **Bus compartido**: todos los dispositivos van en I2C-1; cada driver hace
  `bcm2835_i2c_setSlaveAddress()` antes de cada transacción.
- **AHT21B**: comando 0xAC → esperar 80 ms → leer 6 bytes → CRC8 (0x31).
  Decodificación 20-bit: `T = (v/2^20)*200-50`, `RH = (v/2^20)*100`.
- **BH1750**: 0x01 (power on) + 0x10 (continua H-res) → leer 2 bytes →
  `lux = valor / 1.2`.
- **Tolerancia a fallos**: sin `/dev/mem`, sin sensor o sin OLED la app sigue
  corriendo y muestra `no data`.

## 3. Estructura

```
include/
  engine/Device_t.hpp        # motor (Device::Device_t)
  drivers/AHT21B.hpp         # temp/humedad
  drivers/BH1750.hpp         # luz
  HMC5883L.hpp               # magnetómetro (esqueleto)
  oled/                      # librería SSD1306 (Gavin Lyons)
  nlohmann/json.hpp          # config JSON
src/
  main.cpp                   # make_unique<Device::Device_t>(argc, argv)
  engine/Device_t.cpp
  drivers/AHT21B.cpp, BH1750.cpp
  oled/                      # fuentes SSD1306
config/config.cfg            # app (JSON)
config/hardware.cfg          # bus I2C (JSON)
scripts/install_deps.sh      # bcm2835 + toolchain
scripts/setup_git.sh         # repo Git interactivo
```

## 4. Comandos útiles

```bash
sudo bash scripts/install_deps.sh   # deps (una vez)
make clean && make -j4              # compilar
./bin/App --version                 # versión compilada
./bin/App                           # bucle de lecturas (sudo si hay OLED)
i2cdetect -y 1                      # verificar 0x23/0x38/0x3C
```

## 5. Reglas del proyecto

Ver `docs/LEARNINGS.md` (tags + VERSION), `docs/RULES.md` (resumen) y
`docs/CONTRIBUTING.md` (flujo de contribución).
