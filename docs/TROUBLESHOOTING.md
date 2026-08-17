# TROUBLESHOOTING — Problemas comunes y soluciones

| Síntoma | Causa probable | Solución |
|---|---|---|
| `bcm2835_init() failed` | Sin permisos sobre `/dev/mem` | Ejecutar con `sudo`, o añadir el usuario al grupo adecuado |
| `[AHT21B] init command rejected` | Cableado mal, alimentación insuficiente o dirección errónea | Verificar SDA/SCL y VCC 3.3 V; `i2cdetect -y 1` debe ver 0x38 |
| `[AHT21B] CRC mismatch` | Bus inestable (cables largos, ruido) | Acortar cables; añadir capacitor 100 nF en VCC; bajar `i2c_baudrate_hz` |
| `[BH1750] begin/read failed` | ADDR mal conectado o dirección distinta | Probar 0x23 (ADDR a GND) o 0x5C (ADDR a 3.3 V); ajustar `bh1750_address` |
| `AHT21B: no data` en consola | Sensor no inicializado o desconectado | Comprobar `begin()` en el log de arranque; revisar wiring |
| El OLED no muestra nada | Falta root, dirección distinta o hardware ausente | Ejecutar con `sudo`; comprobar 0x3C en `i2cdetect`; `use_oled` en config |
| La app no imprime | `print_interval_ms` alto o bucle bloqueado | Bajar `print_interval_ms`; verificar que no quedó en `--version` |
| Lecturas de temperatura congeladas | Cable SDA/SCL flotante o sensor en sleep | Resetear alimentación; verificar que `begin()` se ejecutó |
| `make` no encuentra bcm2835.h | Dependencias no instaladas | `sudo bash scripts/install_deps.sh` |
| Errores al linkear `-lbcm2835` | Librería no instalada o LIBDIR equivocado | Verificar `/usr/local/lib/libbcm2835.a`; override `BCM2835_LIBDIR` |
| `App` no compila en 64-bit | Flags de arquitectura heredados | Limpiar (`make distclean`) y recompilar sin `ARCH_FLAGS` |

## Diagnóstico rápido

```bash
# 1. ¿Está el I2C del kernel activo?
ls /dev/i2c-1

# 2. ¿Responden los dispositivos?
sudo apt install i2c-tools
i2cdetect -y 1            # esperado: 0x23, 0x38, 0x3C

# 3. ¿La app ve el hardware?
sudo ./bin/App            # los errores salen por stderr con prefijo [hw]/[AHT21B]/[BH1750]
```
