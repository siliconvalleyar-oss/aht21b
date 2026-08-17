# ACTIVITY — Registro de actividades

Bitácora cronológica de las actividades del proyecto (generación, cambios,
pruebas y despliegues). Se actualiza en cada sesión de trabajo.

| Fecha | Actividad | Resultado |
|---|---|---|
| 2026-08-17 | Verificación remota en la Pi (rama main, v0.1.1): build, `--version`, runtime con y sin root, `i2cdetect` | ✅ Build OK, `App v0.1.1`; sin crash (sin root → consola; con sudo → I2C operativo); ❌ **bus I2C vacío** — no hay sensores conectados (falta wiring/3V3) |
| 2026-08-17 | Fix v0.1.1: guard de `bcm2835_init()` (sin root, bcm2835 usa /dev/gpiomem y deja `bcm2835_bsc1` en MAP_FAILED → SIGSEGV al usar I2C) | ✅ Sin root ya no crashea; I2C requiere root (documentado en USAGE.md) |
| 2026-08-17 | Build cruzado armhf (GCC 10 + sysroot con bcm2835) de todo el proyecto | ✅ `bin/App` ELF 32-bit ARM (GLIBC_2.4, GLIBCXX_3.4.21); sin errores (2 warnings de la librería SSD1306) |
| 2026-08-17 | Generación completa del proyecto según prompt.md: drivers AHT21B/BH1750, esqueleto HMC5883L, motor Device_t, Makefile con `-DVERSION`, scripts y 26 docs | ✅ Todo el código y la documentación creados |
| _fecha_ | _prueba con sensores conectados (bus I2C)_ | _pendiente: conectar AHT21B/BH1750/OLED a GPIO 2/3 y 3V3/GND_ |
| _fecha_ | _implementación del magnetómetro HMC5883L_ | _pendiente (ROADMAP)_ |

## Convención

- Cada entrada se agrega **arriba** de la tabla (más reciente primero).
- Las tareas en curso se marcan `⏳`; las terminadas, `✅`.
