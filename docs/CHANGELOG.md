# CHANGELOG

Formato basado en [Keep a Changelog](https://keepachangelog.com/es/1.0.0/).
La versión coincide siempre con el archivo `VERSION` y con el último tag
(`vX.Y.Z`, ver `LEARNINGS.md`).

## [0.2.2] - 2026-08-17

### Añadido
- **Auto-detección de dispositivos al arrancar**: cada dispositivo (AHT21B,
  BH1750, OLED) se sondea en el bus y solo se usan los que responden. Un
  sensor ausente ya no aparece en la salida (`X: no data`) ni gasta tiempo ni
  reintentos en el bus: se deshabilita y se avisa una vez por stderr
  (`[hw] BH1750 NO detectado (0x23) - deshabilitado`).
- El OLED se sondea con un write barato antes de inicializarlo: sin display,
  el arranque ya no quema cientos de reintentos (~7 s) ni cada vuelta del
  bucle pierde tiempo en `OLEDupdate()`.

## [0.2.1] - 2026-08-17

### Añadido
- **Unit tests de decodificación sin hardware** (`make test`): CRC-8 del
  AHT21B (vectores verificados con dos implementaciones independientes,
  incluido el check clásico de "123456789" = 0xF7), decodificación de los
  valores de 20 bits (RH/T) con casos de borde y fuera de rango, y conversión
  de lux del BH1750 (÷1.2).

### Refactor
- La decodificación pura del AHT21B (CRC-8 y 20-bit) se extrajo a
  `include/drivers/AHT21B_decode.hpp` (funciones inline, sin bcm2835) para
  poder testearla; el driver la usa. El BH1750 expone `luxFromRaw`/
  `luxFromBytes` con el mismo fin.

## [0.1.9] - 2026-08-17

### Corregido
- **Timeout del trigger del AHT21B**: el sensor estira SCL durante toda la
  conversión (~80 ms según datasheet), que arranca dentro del propio write
  de 0xAC; con el límite de 100 ms quedaba justo al borde y fallaba
  intermitente (el driver del kernel, con 1 s de timeout, leía bien). El
  trigger ahora permite 500 ms; init y status mantienen el default.

## [0.1.8] - 2026-08-17

### Corregido
- **El AHT21B nunca entregaba lecturas** aunque respondía en 0x38:
  - `begin()` enviaba el init y volvía al instante; el sensor queda ocupado
    calibrando justo después (estira SCL) y el primer trigger hacía timeout.
    Ahora se sondea el byte de estado hasta que deja de estar ocupado y se
    reporta el estado de calibración.
  - **El byte de CRC nunca valida en este módulo** (verificado en la Pi con
    el driver del kernel: valores consistentes y en rango, CRC siempre
    falla). El CRC ahora es un aviso; la validación real la hace el rango
    físico (RH 0-100 %, T -40..85 °C).
  - `recover()` del helper I2C ahora resetea el controlador BSC completo
    (disable → clear → enable) para que un timeout de software no lo deje
    trabado.

## [0.1.6] - 2026-08-17

### Corregido
- **Completa el fix 0.1.5 en el OLED**: aunque los sensores ya usaban
  transacciones acotadas, la librería SSD1306 seguía escribiendo con
  `bcm2835_i2c_write()` (sin timeout). Verificado en la Pi: con el bus
  trabado por el sensor flaky, `OLEDinit()` quedaba colgado para siempre.
  Ahora `I2C_Write_Byte()` usa el helper acotado (50 ms por intento) y
  `OLEDBuffer()` aborta la transferencia en el primer byte fallido (el OLED
  es opcional; la app sigue en modo consola).

## [0.1.5] - 2026-08-17

### Corregido
- **Sensores AHT21B/BH1750: la app ya no se cuelga si un esclavo se cae a
  mitad de una transacción I2C**: `bcm2835_i2c_write()`/`read()` esperan el
  bit S_DONE en un bucle sin timeout; si el esclavo desaparece del bus
  (contacto intermitente) el controlador nunca llega a DONE y la app queda
  bloqueada para siempre (ni SIGTERM la detiene). Nuevo módulo
  `drivers/I2C_bus` que replica la lógica con un plazo máximo de 100 ms y
  deja el controlador en estado limpio ante NACK, clock-stretch o timeout.
  *(Nota: el camino del OLED quedó pendiente y se completó en 0.1.6.)*

## [0.1.4] - 2026-08-17

### Corregido
- **Segfault en el display SSD1306**: la librería vendorizada nunca asignaba
  el framebuffer; `OLEDclearBuffer()` hacía `memset` sobre un puntero nulo.
  Solo apareció al conectar un sensor (con el bus vacío, los reintentos I2C
  de `OLEDbegin()` alargaban el arranque y el timeout mataba la app antes de
  llegar al memset). Ahora `OLEDbegin()` reserva el buffer con `malloc` y el
  destructor lo libera.

## [0.1.3] - 2026-08-17

### Corregido
- **`make run` ejecutaba un binario viejo**: `VERSION` no era dependencia del
  build, así que al bumpear la versión `make` no recompilaba y corría el
  binario anterior (el que crasheaba sin root). Ahora `VERSION` es
  prerequisito de cada objeto y cualquier bump fuerza la recompilación con el
  nuevo `-DVERSION`.

## [0.1.2] - 2026-08-17

### Documentado
- Verificación remota en la Pi (build, `--version`, runtime sin/con root,
  `i2cdetect` con el bus vacío) registrada en REPORT/TODO/ACTIVITY.

## [0.1.1] - 2026-08-17

### Corregido
- **Segfault al arrancar sin root**: bcm2835 v1.71, sin root, mapea solo
  /dev/gpiomem (GPIO) y deja el puntero I2C (bsc1) en MAP_FAILED; cualquier
  acceso I2C escribía sobre una dirección inválida y la app moría con SIGSEGV
  (verificado en Pi 4). Ahora `initHardware()` exige root para usar I2C y,
  sin root, la app corre en modo consola sin crashear.

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
