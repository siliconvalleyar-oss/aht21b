# TESTING — Pruebas

## Pruebas de compilación (sin hardware)

```bash
make clean && make -j4        # compila sin errores en 32 y 64 bits
./bin/App --version           # imprime la versión compilada (macro -DVERSION)
```

## Pruebas en hardware (Raspberry Pi)

1. **Bus I2C**: `i2cdetect -y 1` → se esperan 0x23 (BH1750), 0x38 (AHT21B),
   0x3C (OLED).
2. **AHT21B**: `./bin/App` y comprobar que `Temp` y `RH` cambian al soplar o
   tocar el sensor; los valores deben ser físicamente plausibles
   (15-35 °C, 30-80 %).
3. **BH1750**: tapar el sensor → `Lux` baja cerca de 0; iluminar con linterna
   → sube (cientos de lux).
4. **CRC**: si el cableado es inestable, el log muestra
   `[AHT21B] CRC mismatch` (el driver descarta la lectura).
5. **OLED**: con `sudo`, las 4 filas muestran versión, temperatura/humedad,
   lux y el contador de loops.
6. **Tolerancia a fallos**: desconectar un sensor y reiniciar — la app sigue
   corriendo y muestra `no data` para ese canal.

## Valores de referencia (AHT21B)

| Entrada | Esperado |
|---|---|
| Temperatura ambiente | 20-30 °C |
| Humedad ambiente | 40-70 % |
| Soplar el sensor | RH sube varios puntos |
| Tocar el sensor | Temp sube 1-3 °C |

## Checklist

- [ ] Compila limpio con `make clean && make -j4`
- [ ] `--version` muestra `v0.1.0`
- [ ] `i2cdetect` encuentra los 3 dispositivos
- [ ] Lecturas estables y plausibles de los 2 sensores
- [ ] OLED muestra las 4 filas (con sudo)
- [ ] Ctrl+C sale limpio ("Bye (App v0.1.0).")
