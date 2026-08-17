# API — Referencia de la API pública

Todas las clases viven en `include/` y las implementaciones en `src/`. El
acceso al hardware se hace con la librería **bcm2835** (I2C por `/dev/mem`).

## Device::Device_t (`include/engine/Device_t.hpp`)

Motor principal de la aplicación.

| Método | Descripción |
|---|---|
| `Device_t(int argc, char* argv[])` | Constructor. Guarda los argumentos de CLI para `--version`/`--help`. |
| `~Device_t()` | Apaga el OLED y cierra bcm2835. |
| `void run()` | Comprueba `--version`/`--help`, muestra el banner con la versión compilada, inicializa hardware y ejecuta el bucle de lecturas hasta Ctrl+C. |

## AHT21B::AHT21B_t (`include/drivers/AHT21B.hpp`)

Sensor de temperatura y humedad (I2C, dirección 0x38).

| Método | Descripción |
|---|---|
| `AHT21B_t(uint8_t addr = 0x38)` | Construye el driver con la dirección del chip. |
| `bool begin()` | Envía el comando de inicialización (0xBE) y marca el sensor listo. |
| `bool read(float* tempC, float* humPct)` | Dispara una medición (0xAC), espera 80 ms, lee 6 bytes y valida CRC8 (0x31). Rellena temperatura (°C) y humedad relativa (%). |
| `bool isReady()` | true tras un `begin()` exitoso. |

## BH1750::BH1750_t (`include/drivers/BH1750.hpp`)

Sensor de luz ambiente (I2C, dirección 0x23 / 0x5C).

| Método | Descripción |
|---|---|
| `BH1750_t(uint8_t addr = 0x23)` | Construye el driver. |
| `bool begin()` | Enciende el sensor (0x01) y activa el modo continua de alta resolución (0x10). |
| `bool read(float* lux)` | Lee 2 bytes y devuelve la iluminancia en lux (÷1.2). |
| `bool powerDown()` | Vuelve al modo de bajo consumo (0x00). |
| `bool isReady()` | true tras un `begin()` exitoso. |

## Device::HMC5883L_t (`include/HMC5883L.hpp`)

Esqueleto del magnetómetro de 3 ejes (I2C, 0x1E). **No implementado** todavía:
`begin()` y `read()` devuelven `false` (ver `docs/ROADMAP.md`).

## SSD1306 (`include/oled/SSD1306_OLED.hpp`)

Librería de Gavin Lyons (`SSD1306_OLED_RPI`) para el display OLED. Métodos
usados por el motor: `OLEDbegin(speed, addr)`, `OLEDclearBuffer()`,
`setCursor(x, y)`, `setFontNum(...)`, `setTextSize(1)`, `setTextColor(WHITE)`,
`print(...)` (heredado de `Print`) y `OLEDupdate()`.

## Configuración

La app lee `config/config.cfg` y `config/hardware.cfg` (JSON, ver
`docs/USAGE.md`); cualquier clave ausente conserva el valor por defecto.
