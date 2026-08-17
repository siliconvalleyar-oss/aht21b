# TODO — Checklist de pendientes

Checklist de cumplimiento del proyecto (generación según `prompt.md` +
mejoras). Se actualiza en cada sesión; se marca `[x]` cuando el item está
**implementado, compilado y probado**.

## Generación del proyecto (prompt.md)

- [x] Estructura de carpetas obligatoria (config, docs, include, src, scripts, examples)
- [x] `src/main.cpp` con `std::make_unique<Device::Device_t>` y `run()`
- [x] Clase `Device::Device_t` (namespace Device) con `run()`
- [x] Uso de bcm2835 (I2C) y `#include <memory>`
- [x] Versión en tiempo de compilación (`-DVERSION`) + `--version`
- [x] Makefile: `all/clean/distclean/install`, `obj/` jerárquico, `bin/App`, `-lbcm2835`
- [x] `scripts/install_deps.sh` (bcm2835, 32/64 bits)
- [x] `scripts/setup_git.sh` (repo Git interactivo, gh o manual)
- [x] Drivers AHT21B y BH1750 + esqueleto HMC5883L
- [x] OLED SSD1306 compilable (librería incluida)
- [x] Todos los `.md` de `docs/` completos + README.md
- [x] `VERSION` = 0.1.0, `LICENSE` MIT
- [x] Compilación verificada (build cruzado armhf: `bin/App` ELF 32-bit ARM, requiere GLIBC_2.4)
- [x] Fix v0.1.1: guard `bcm2835_init()` — sin root ya no crashea (modo consola); I2C requiere root (documentado)
- [x] Build remoto en la Pi (`make clean && make -j4`) + `--version` → v0.1.1
- [x] Repo remoto: `siliconvalleyar-oss/aht21b` público, tags v0.1.0..v0.1.4
- [x] Fix 0.1.3: `VERSION` como prerequisito del build (bump fuerza recompilación)
- [x] Fix 0.1.4: buffer del SSD1306 asignado en `OLEDbegin()` (segfault de memset(nullptr) al haber sensor en el bus)
- [x] Fix 0.1.5: `drivers/I2C_bus` con write/read acotados por timeout para sensores (bcm2835 espera S_DONE sin límite → colgaba para siempre si el sensor se caía a mitad de transacción)
- [x] Fix 0.1.6: OLED con writes acotados (`I2C_Write_Byte` → `I2C::write` 50 ms; `OLEDBuffer` aborta en el primer byte fallido). Verificado en la Pi: con 0.1.5 el OLED seguía colgando con el bus trabado
- [x] Fix 0.1.8: AHT21B `begin()` sondea el estado hasta no-busy; CRC tolerante (este módulo no lo calcula; valida rango físico RH 0-100 %, T -40..85 °C); `recover()` resetea el controlador BSC
- [x] Fix 0.1.9: trigger del AHT21B con 500 ms (el sensor estira SCL ~80 ms durante la conversión; 100 ms fallaba intermitente)
- [ ] Pruebas con sensores **conectados de forma estable**: el AHT21B **respondió con datos válidos** (RH≈78 %, T≈20 °C) pero vuelve a desaparecer del bus (contacto intermitente). Reseat del sensor, 3V3 (nunca 5V), capacitor 100 nF VDD-GND, cables cortos

## Mejoras (ROADMAP)

- [ ] Implementar HMC5883L (begin/read de 3 ejes)
- [ ] Ejemplos independientes en `examples/`
- [ ] Unit tests de decodificación sin hardware
- [ ] Salida CSV/JSON e histórico
- [ ] Exposición BLE/MQTT (opcional)
