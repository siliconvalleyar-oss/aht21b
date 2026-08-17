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
- [ ] Pruebas en hardware real (Pi + sensores + OLED)
- [ ] Repo remoto con tag `v0.1.0` (setup_git.sh)

## Mejoras (ROADMAP)

- [ ] Implementar HMC5883L (begin/read de 3 ejes)
- [ ] Ejemplos independientes en `examples/`
- [ ] Unit tests de decodificación sin hardware
- [ ] Salida CSV/JSON e histórico
- [ ] Exposición BLE/MQTT (opcional)
