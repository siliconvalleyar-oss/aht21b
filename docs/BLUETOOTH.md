# BLUETOOTH — Estado

## Estado actual

**Este proyecto no usa Bluetooth.** AHT21B_bh1750 es una aplicación de
sensores por I2C (temperatura, humedad y luz) con display OLED local; no hay
transmisión inalámbrica de datos.

## Plan futuro (opcional)

Si se desea exponer las lecturas de forma inalámbrica, las opciones naturales
son (ver [ROADMAP.md](ROADMAP.md)):

1. **BLE (GATT server)** con `bluetoothd` + `dbus` — lecturas como
   características (temperatura, humedad, lux).
2. **Wi-Fi / MQTT** — publicar en un broker (`mosquitto_pub`) cada N segundos.
3. **HTTP REST** — mini servidor embebido en la Pi.

> Decisión pendiente: no se implementará hasta que se confirme el caso de uso
> (dashboard local vs. integración con Home Assistant / móvil).
