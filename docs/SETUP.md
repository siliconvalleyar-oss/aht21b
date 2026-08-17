# SETUP — Puesta en marcha

## Paso a paso

1. **Cablear** los sensores y el OLED según [HARDWARE.md](HARDWARE.md).
2. **Instalar dependencias**: `sudo bash scripts/install_deps.sh`.
3. **Habilitar I2C**: `sudo raspi-config` → Interface Options → I2C → Enable
   (o `dtparam=i2c_arm=on` en `/boot/config.txt`) y reiniciar.
4. **Verificar el bus**: `i2cdetect -y 1` debe mostrar 0x23, 0x38 y 0x3C.
5. **Compilar**: `make clean && make -j4`.
6. **Probar**: `./bin/App --version` (imprime la versión compilada).
7. **Ejecutar**: `./bin/App` — muestra el banner y luego lecturas periódicas:
   ```
   AHT21B_bh1750 v0.1.0 - AHT21B temp/humidity + BH1750 light (I2C)
   [2026-08-17 10:00:05] loop=5 | Temp 25.42 C | RH 45.3 % | Lux 320.5
   ...
   ```
8. **Ajustar configuración** en `config/config.cfg` (intervalos, sensores
   habilitados, direcciones) y `config/hardware.cfg` (velocidad I2C).

## Notas

- `bcm2835_init()` mapea `/dev/mem`: normalmente se ejecuta con `sudo` (o con
  el usuario en el grupo adecuado). Si no hay permisos, la app avisa y sigue
  en modo consola sin sensores.
- Los sensores no responden si el cableado está mal: el log mostrará
  `AHT21B: no data` / `BH1750: no data` en cada iteración.
