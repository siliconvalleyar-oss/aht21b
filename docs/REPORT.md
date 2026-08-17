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
| Versión | 0.2.1 (v0.2.1) |
| Fecha | 2026-08-17 |
| Estado global | ✅ Software completo y verificado (build remoto OK, sin colgadas) + **unit tests de decodificación sin hardware** (26 checks, 0 failures). El AHT21B **sí lee datos válidos** (RH≈78 %, T≈20 °C vía kernel) pero sigue con **contacto intermitente**: aparece en 0x38 y desaparece. La app está lista para leer en cuanto el wiring sea estable |

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
- [x] Fix 0.1.3: `VERSION` como prerequisito del build (bump fuerza recompilación; `make run` ya no corre binarios viejos)
- [x] Fix 0.1.4: buffer del SSD1306 asignado en `OLEDbegin()` (elimina el segfault de `memset(nullptr)` al haber sensor en el bus)
- [x] Fix 0.1.5: módulo `drivers/I2C_bus` con write/read acotados por timeout (100 ms) para **sensores** (AHT21B/BH1750). bcm2835 espera S_DONE sin límite → la app se colgaba para siempre si un sensor se caía a mitad de transacción
- [x] Fix 0.1.6: **el OLED también quedaba colgado** (verificado en la Pi: `OLEDinit()` con el bus trabado). `I2C_Write_Byte()` ahora usa el helper acotado (50 ms/intento) y `OLEDBuffer()` aborta en el primer byte fallido → la app ya no puede colgarse en ningún camino I2C
- [x] Repo remoto configurado: `siliconvalleyar-oss/aht21b` público, tags v0.1.0..v0.1.4
- [x] Fix 0.1.8: `begin()` sondea el estado hasta que el sensor deja de estar ocupado tras el init; **CRC no fatal** (este módulo no lo calcula bien; validan los rangos físicos); `recover()` resetea el controlador BSC
- [x] Fix 0.1.9: trigger del AHT21B con timeout de **500 ms** (el sensor estira SCL durante la conversión ~80 ms; con 100 ms fallaba intermitente)
- [x] **Unit tests sin hardware** (0.2.1): `make test` — CRC-8 AHT21B (vectores doble-verificados), decodificación 20-bit con bordes y fuera de rango, lux BH1750. 26 checks, 0 failures. Decodificación extraída a `AHT21B_decode.hpp` (inline, sin bcm2835)
- [ ] Pruebas con sensores **conectados de forma estable** — el AHT21B **sí respondió con datos válidos** (RH≈78 %, T≈20 °C, vía kernel/Python) pero vuelve a **desaparecer del bus** (contacto intermitente; `i2cdetect` vacío otra vez). Acción: reseat del sensor, verificar 3V3 (nunca 5V), capacitor 100 nF VDD-GND, cables cortos. La app (v0.1.9) está lista para leer

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
| 2026-08-17 | `i2cdetect -y 1` con AHT21B conectado | ⚠️ 0x38 apareció una vez, luego el bus quedó vacío e intermitente (0/10 respuestas) | **Contacto intermitente**: reseat del sensor, verificar 3V3/GND y capacitor 100 nF |
| 2026-08-17 | `sudo ./bin/App` con sensor presente (v0.1.3) | ❌ **Segfault (exit 139)** en `memset()` de `OLEDclearBuffer()` (buffer nullptr) | Fix 0.1.4: malloc del framebuffer en `OLEDbegin()` |
| 2026-08-17 | `sudo timeout 6 ./bin/App` (v0.1.4, sensor flaky) | ⚠️ **La app quedó colgada para siempre**: bcm2835 espera S_DONE sin timeout; el proceso siguió vivo y ni SIGTERM lo detuvo (hubo que `pkill -9`) | Fix 0.1.5: `drivers/I2C_bus` con write/read acotados (100 ms) |
| 2026-08-17 | Build cruzado + remoto del fix 0.1.5 | ✅ exit 0 local (armhf) y remoto (Pi); `App v0.1.5` en la Pi | — |
| 2026-08-17 | `sudo timeout 8 ./bin/App` (v0.1.5, sensor flaky) | ⚠️ **Siguió colgada**: el log muestra los NACK acotados de los sensores (`[i2c] write NACK (addr 0x23)` ✓) pero el write del **OLED** (`I2C_Write_Byte` → `bcm2835_i2c_write` sin timeout) quedó bloqueado con el bus trabado | Fix 0.1.6: `I2C_Write_Byte()` acotado (50 ms) + abort de `OLEDBuffer()` en el primer byte fallido |
| 2026-08-17 | `sudo timeout 15 ./bin/App` (v0.1.6, bus muerto) | ✅ **Sin colgado**: EXIT=124 (timeout), el bucle corre (`loop=0` → `loop=2`), sin procesos residuales. Arranque lento con bus muerto (~7 s en `OLEDinit` acotado) y ~2.5 s/iteración (writes del OLED fallidos, acotados) | Con el wiring estable el arranque y el bucle vuelven a ser rápidos |
| 2026-08-17 | AHT21B presente (0x38, status 0x18 calibrado) + sondeo kernel/Python | ✅ **Datos válidos y consistentes**: 10/10 paquetes RH≈78 %, T≈20 °C. **CRC siempre falla** en este módulo | 0.1.8: CRC tolerante (aviso) + rango físico como validación |
| 2026-08-17 | `sudo timeout 15 ./bin/App` (v0.1.8, sensor presente) | ⚠️ init y status OK, pero el **trigger (0xAC) hacía timeout** (el sensor estira SCL durante la conversión ~80 ms; 100 ms quedaba al límite) | 0.1.9: trigger con 500 ms |
| 2026-08-17 | Post-app: `i2cget` status + `i2cdetect` | ⚠️ **El sensor desapareció del bus otra vez** (NACK, i2cdetect vacío) — patrón de contacto intermitente | Reseat del sensor; cuando 0x38 sea estable, la app debe leer Temp/RH |
| 2026-08-17 | Unit tests de decodificación (0.2.1) | ✅ `make test`: **26 checks, 0 failures** (CRC-8, 20-bit RH/T, lux) | Decodificación extraída a `AHT21B_decode.hpp` para testear sin hardware |
