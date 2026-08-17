# AHT21B_bh1750

Aplicación C++17 para **Raspberry Pi** que lee **temperatura y humedad** del
sensor **AHT21B** y **luz ambiente** del sensor **BH1750** a través del bus
**I2C** usando la librería **bcm2835**, con **display OLED SSD1306** opcional.
Compatible con Raspberry Pi OS de **32 bits (armhf)** y **64 bits (arm64)**.

```
AHT21B (temp/humedad) ─┐
BH1750 (luz)          ─┼─► I2C-1 (GPIO 2/3) ──► bcm2835 ──► App
SSD1306 OLED (opcional)┘
```

## Requisitos

- Raspberry Pi (toda la familia BCM2835/BCM2711) con Raspberry Pi OS 32-bit
  o 64-bit.
- `build-essential`, `git`, `wget` y la librería **bcm2835** v1.71
  (se instalan con el script).
- I2C habilitado en el sistema (`raspi-config` → I2C → Enable).
- `gh` (GitHub CLI) opcional para `setup_git.sh`.

## Instalación

```bash
# 1. Dependencias (bcm2835 + toolchain)
sudo bash scripts/install_deps.sh

# 2. Habilitar I2C (una vez) y reiniciar
sudo raspi-config        # Interface Options > I2C > Enable

# 3. Compilar
make clean && make -j4
```

## Uso

```bash
./bin/App --version       # App v0.1.0  (versión en tiempo de compilación)
./bin/App --help          # ayuda
./bin/App                 # bucle de lecturas (Ctrl+C para salir)
sudo ./bin/App            # con OLED (bcm2835 necesita /dev/mem)
```

Salida típica:

```
AHT21B_bh1750 v0.1.0 - AHT21B temp/humidity + BH1750 light (I2C)
[2026-08-17 10:00:05] loop=5 | Temp 25.42 C | RH 45.3 % | Lux 320.5
```

La configuración se ajusta en `config/config.cfg` y `config/hardware.cfg`
(JSON); ver [docs/USAGE.md](docs/USAGE.md).

## Versión

La versión se define **en tiempo de compilación**: el Makefile lee el archivo
`VERSION` y la inyecta con `-DVERSION="x.y.z"`. Se muestra al iniciar y con
`./bin/App --version`. El binario siempre reporta la versión con la que se
compiló (aunque se copie a una Pi sin el repo). Reglas de versionado en
[docs/LEARNINGS.md](docs/LEARNINGS.md).

## Configurar el repositorio Git

```bash
bash scripts/setup_git.sh      # interactivo: usa gh si está instalado,
                               # o guía los pasos manuales (remote + push con tag)
```

## Estructura

```
├── Makefile            # -DVERSION, obj/ espejo de src/, bin/App, -lbcm2835
├── VERSION             # fuente de la versión (0.1.0)
├── config/             # config.cfg (app) + hardware.cfg (I2C)
├── include/            # engine/Device_t.hpp, drivers/, HMC5883L.hpp, oled/, nlohmann/
├── src/                # main.cpp, engine/, drivers/, oled/
├── scripts/            # install_deps.sh, setup_git.sh
├── docs/               # documentación completa (26 archivos)
└── examples/           # (vacío) futuros ejemplos
```

## Documentación

| Documento | Contenido |
|---|---|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) / [ARQUITECTURA.md](docs/ARQUITECTURA.md) | Arquitectura (EN/ES) |
| [docs/HARDWARE.md](docs/HARDWARE.md) | Conexionado y direcciones I2C |
| [docs/API.md](docs/API.md) | Referencia de la API |
| [docs/USAGE.md](docs/USAGE.md) | Uso y configuración |
| [docs/BUILD.md](docs/BUILD.md) / [INSTALL.md](docs/INSTALL.md) | Compilación e instalación |
| [docs/TESTING.md](docs/TESTING.md) / [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Pruebas y problemas comunes |
| [docs/LEARNINGS.md](docs/LEARNINGS.md) / [RULES.md](docs/RULES.md) | Reglas y versionado |
| [docs/ROADMAP.md](docs/ROADMAP.md) / [TODO.md](docs/TODO.md) | Pendientes |

## Licencia

MIT — ver [LICENSE](LICENSE).
