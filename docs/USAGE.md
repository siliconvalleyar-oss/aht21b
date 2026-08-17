# USAGE — Uso de la aplicación

```
Usage: App [options]

Reads temperature/humidity (AHT21B) and light (BH1750) over I2C using
bcm2835, with an optional SSD1306 OLED display.

Options:
  --version, -v   Show the application version and exit
  --help, -h      Show this help and exit

The version is baked in at compile time from the VERSION file.
```

## Ejemplos

```bash
./bin/App --version        # App v0.1.1
./bin/App --help           # ayuda
sudo ./bin/App             # lecturas de sensores + OLED (bcm2835 I2C necesita root)
./bin/App                  # sin root: modo consola (sin sensores ni OLED), sin crash
```

> **Root**: los sensores AHT21B/BH1750 y el OLED usan el I2C de bcm2835, que
> requiere `/dev/mem` (root). Sin root la app arranca igual pero en modo
> consola (avisa por stderr y muestra `no data`).

## Salida típica

```
AHT21B_bh1750 v0.1.1 - AHT21B temp/humidity + BH1750 light (I2C)
[2026-08-17 10:00:05] loop=5 | Temp 25.42 C | RH 45.3 % | Lux 320.5
[2026-08-17 10:00:07] loop=10 | Temp 25.40 C | RH 45.4 % | Lux 321.0
```

## Configuración (`config/config.cfg`)

| Clave | Default | Descripción |
|---|---|---|
| `loop_delay_ms` | 1000 | Pausa entre iteraciones del bucle |
| `print_interval_ms` | 2000 | Cada cuánto se imprime/refresca el OLED |
| `use_oled` | true | Habilitar el display SSD1306 |
| `use_aht21b` | true | Habilitar el sensor AHT21B |
| `use_bh1750` | true | Habilitar el sensor BH1750 |
| `aht21b_address` | 56 | Dirección I2C del AHT21B (0x38) |
| `bh1750_address` | 35 | Dirección I2C del BH1750 (0x23) |
| `oled_address` | 60 | Dirección I2C del OLED (0x3C) |

## Configuración (`config/hardware.cfg`)

| Clave | Default | Descripción |
|---|---|---|
| `i2c_bus` | 1 | Bus I2C (1 en la mayoría de las Pi) |
| `i2c_baudrate_hz` | 100000 | Velocidad del bus I2C |
| `bcm2835_i2c_divider` | 2500 | Divisor de reloj alternativo |
| `sda_pin` / `scl_pin` | 2 / 3 | Pines informativos (no modificables) |

Si un archivo de configuración falta o es inválido, la app usa los defaults y
avisa por stderr.
