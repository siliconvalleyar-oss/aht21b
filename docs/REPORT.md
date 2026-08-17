# REPORT — Informe de estado del proyecto

## ⚠️ REGLA DE USO (obligatoria)

> **Quien lea este reporte (humano o IA) debe completarlo a medida que lo
> cumple**: marcar `[x]` cada punto del checklist cuando se verifique/ejecute,
> anotar el resultado en la tabla de registro y actualizar la sección Estado.
> **No se cierra el reporte con items del checklist sin marcar.**

## 1. Estado

| Campo | Valor |
|---|---|
| Proyecto | AHT21B_bh1750 (temp/humedad + luz por I2C) |
| Versión | 0.1.1 (v0.1.1) |
| Fecha | 2026-08-17 |
| Estado global | ✅ Generación completa + compilación verificada + build remoto OK; **hardware no conectado** (bus I2C vacío) |

## 2. Resumen

Se generó el proyecto completo según `prompt.md`: drivers AHT21B y BH1750
(bcm2835 I2C), esqueleto HMC5883L, motor `Device::Device_t` con configuración
JSON y OLED opcional, versión en tiempo de compilación, Makefile, scripts y
26 documentos en `docs/`. Falta la **validación en hardware real** y la
**implementación del magnetómetro** (ver ROADMAP.md).

## 3. Checklist de cumplimiento

- [x] Estructura de carpetas obligatoria creada
- [x] `src/main.cpp` con `std::make_unique<Device::Device_t>` y `run()`
- [x] `Device::Device_t` implementado (CLI, config, bucle, OLED)
- [x] Drivers AHT21B y BH1750 implementados (I2C + decodificación + CRC)
- [x] `HMC5883L.hpp` esqueleto declarado (pendiente de implementar)
- [x] Makefile con `-DVERSION="$(cat VERSION)"` y objetivos `all/clean/distclean/install`
- [x] Scripts `install_deps.sh` y `setup_git.sh`
- [x] Docs completos (26 archivos) y README.md
- [x] `VERSION` = 0.1.1 y `LICENSE` MIT
- [x] Compilación verificada (build cruzado armhf GCC 10 + sysroot; `bin/App` ELF 32-bit ARM, requiere GLIBC_2.4/GLIBCXX_3.4.21)
- [x] Build remoto en la Pi (`make clean && make -j4`) exitoso; `./bin/App --version` → v0.1.1
- [x] Fix root/I2C: sin root (joy) → modo consola sin crash; con sudo → I2C inicializa y el bucle corre sin crash
- [x] Repo remoto configurado: `siliconvalleyar-oss/aht21b` público, tags v0.1.0 y v0.1.1
- [ ] Pruebas con sensores **conectados** — pendiente: `i2cdetect` no detecta ningún dispositivo (bus vacío; revisar wiring/3V3)

## 4. Registro de pruebas (completar)

| Fecha | Prueba | Resultado | Acción |
|---|---|---|---|
| 2026-08-17 | Generación del código y la documentación | ✅ Completado | — |
| 2026-08-17 | Build cruzado armhf (GCC 10 + sysroot) de todo el proyecto | ✅ `bin/App` ELF 32-bit ARM (GLIBC_2.4, GLIBCXX_3.4.21); solo 2 warnings de la librería SSD1306 | — |
| 2026-08-17 | Fix v0.1.1: guard de `bcm2835_init()` (sin root usa /dev/gpiomem y deja `bcm2835_bsc1` en MAP_FAILED → SIGSEGV en I2C) | ✅ Sin root: modo consola sin crash; con sudo: I2C operativo | root requerido para I2C (documentado en USAGE.md) |
| 2026-08-17 | `make clean && make -j4` en la Pi (remoto) | ✅ exit 0, todos los .o + link `-lbcm2835` | — |
| 2026-08-17 | `./bin/App --version` en la Pi | ✅ `App v0.1.1` | — |
| 2026-08-17 | `./bin/App` con sudo (6 s, bucle completo) | ✅ exit 124 (timeout esperado), sin crash; NACK correctos | — |
| 2026-08-17 | `i2cdetect -y 1` y `-y 0` | ❌ **Ningún dispositivo en el bus** (0x23, 0x38, 0x3C ausentes) | Conectar sensores + OLED a GPIO 2/3 (SDA/SCL) y 3V3/GND |
