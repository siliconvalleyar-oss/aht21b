# Arquitectura

Versión en español de la arquitectura del proyecto (la versión en inglés está
en [ARCHITECTURE.md](ARCHITECTURE.md)).

## Capas

```
┌─────────────────────────────────────────────────────────────┐
│ src/main.cpp                                                │
│   std::make_unique<Device::Device_t>(argc, argv); run();    │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ Device::Device_t (motor)                                    │
│   • CLI: --version / --help (versión en tiempo de compilación)│
│   • Config: config/config.cfg + config/hardware.cfg (JSON)  │
│   • Ciclo: leer sensores → imprimir → refrescar OLED        │
└──────┬─────────────────┬──────────────────┬─────────────────┘
       ▼                 ▼                  ▼
  AHT21B_t          BH1750_t          SSD1306 (OLED)
  (temp/humedad)    (lux)             (display opcional)
       └─────────────────┬──────────────────┘
                         ▼
              bcm2835 I2C (i2c-1)
                         ▼
        Sensores y display en el bus I2C de la Pi
```

## Responsabilidades

| Capa | Archivos | Responsabilidad |
|---|---|---|
| Entrada | `src/main.cpp` | Mínimo posible: construir el motor con `make_unique` y ejecutar `run()`. |
| Motor | `src/engine/Device_t.cpp`, `include/engine/Device_t.hpp` | Ciclo de vida del hardware, configuración, bucle de muestreo, tolerancia a fallos. |
| Drivers | `src/drivers/*.cpp`, `include/drivers/*.hpp` | Protocolo de cada chip (comandos, decodificación, CRC). No saben nada de la aplicación. |
| OLED | `src/oled/*`, `include/oled/*` | Librería SSD1306 de Gavin Lyons (solo presentación). |
| Build | `Makefile` | Versión compilada (`-DVERSION`), árbol `obj/` espejo de `src/`. |

## Decisiones de diseño

- **Versión en tiempo de compilación**: el Makefile inyecta la versión con
  `-DVERSION="$(cat VERSION)"`; nunca se lee el archivo en runtime (regla del
  proyecto, ver `LEARNINGS.md`).
- **Drivers aislados del bus**: usan solo primitivas I2C de bcm2835, así el
  motor podría migrar a `/dev/i2c-1` sin tocar los drivers.
- **Tolerancia a fallos**: si falta un sensor, el OLED o falla
  `bcm2835_init()`, la app avisa por stderr y sigue en modo consola.
- **Sin `new`/`delete` explícitos** en el código de aplicación: `main.cpp`
  usa `std::make_unique` y el OLED se libera en el destructor del motor.
