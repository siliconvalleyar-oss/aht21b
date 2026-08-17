# DIAGRAMS — Diagramas

## Flujo de la aplicación

```
main.cpp
   │  std::make_unique<Device::Device_t>(argc, argv)
   ▼
Device_t::run()
   ├─ handleCliFlags()        --version / --help → salida inmediata
   ├─ banner: "AHT21B_bh1750 v0.1.0"
   ├─ loadConfig()            config/config.cfg + hardware.cfg (JSON)
   ├─ initHardware()          bcm2835_init → i2c_begin → sensores → OLED
   └─ bucle principal
        │
        ├─ AHT21B_t::read()   → Temp °C, RH %
        ├─ BH1750_t::read()   → Lux
        ├─ printReadings()    → consola (cada print_interval_ms)
        ├─ updateDisplay()    → OLED (si está disponible)
        └─ bcm2835_delay(loop_delay_ms)   … hasta Ctrl+C
```

## Comunicación I2C

```
bcm2835 (master) ──SDA/SCL──► 0x38 AHT21B
                          └──► 0x23 BH1750
                          └──► 0x3C SSD1306 OLED

Cada driver hace setSlaveAddress() antes de cada transacción,
así todos comparten el mismo periférico I2C sin conflictos.
```

## Secuencia de una medición AHT21B

```
[write] 0xAC 0x33 0x00   → dispara conversión
[wait]  80 ms
[read]  status | hum[2] | temp[2] | crc
        └─ CRC8 (0x31) validado; si falla → lectura descartada
```

## Secuencia BH1750 (modo continuo)

```
[write] 0x01   → power on
[write] 0x10   → modo continua alta resolución (1 lx, ~120 ms)
[read]  msb | lsb   → lux = valor / 1.2
```
