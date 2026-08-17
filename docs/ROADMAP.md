# ROADMAP — Hoja de ruta

Orden de prioridad (P0 = próxima, P2 = ideal a futuro).

## P0 — Validación en hardware real
- [ ] Probar en la Pi con AHT21B + BH1750 + OLED conectados.
- [ ] Verificar lecturas estables (ver TESTING.md) y ajustar la velocidad I2C
      si aparecen `CRC mismatch` / `no data`.
- [ ] Ejecutar como servicio systemd para arranque automático.

## P1 — Completar funcionalidad
- [ ] **HMC5883L**: implementar `begin()`/`read()` (lectura de 3 ejes + modo
      continuo) y mostrar la orientación (heading) en el OLED.
- [ ] **Ejemplos** en `examples/`: `aht21b_demo.cpp`, `bh1750_demo.cpp`
      (drivers usados de forma independiente).
- [ ] **Unit tests** del motor y de la decodificación (20-bit → °C / %)
      sin hardware (patrón `make test` del proyecto hermano).

## P2 — Mejoras
- [ ] Salida a CSV/JSON de las lecturas (histórico).
- [ ] Exposición inalámbrica: BLE (GATT) o MQTT (ver BLUETOOTH.md).
- [ ] Alarma de umbrales (temperatura/humedad/lux fuera de rango).
- [ ] Migración opcional del bus a `/dev/i2c-1` (kernel) para no depender de
      `/dev/mem` en los sensores.

## Criterio de cierre
Cada item se marca `[x]` cuando está implementado, compilado y probado
(hardware o test unitario), y documentado en CHANGELOG.md / ACTIVITY.md.
