# Architecture

## Directory layout

```
AHT21B_bh1750/
├── Makefile                 # builds bin/App from src/ via obj/ (mirrored tree)
├── README.md
├── LICENSE                  # MIT
├── VERSION                  # single source of truth for the app version
├── config/
│   ├── config.cfg           # application settings (JSON)
│   └── hardware.cfg         # I2C bus/hardware settings (JSON)
├── include/                 # public headers
│   ├── engine/Device_t.hpp  # application engine (Device::Device_t)
│   ├── drivers/             # sensor drivers (AHT21B, BH1750)
│   ├── HMC5883L.hpp         # magnetometer skeleton (not implemented)
│   ├── oled/                # SSD1306 OLED library (Gavin Lyons)
│   └── nlohmann/json.hpp    # JSON single-header (config parsing)
├── src/                     # implementations, one .cpp per header
│   ├── main.cpp             # entry point (std::make_unique<Device::Device_t>)
│   ├── engine/Device_t.cpp  # engine implementation
│   ├── drivers/             # AHT21B.cpp, BH1750.cpp
│   └── oled/                # SSD1306 library sources
├── obj/                     # object files (mirrors src/, gitignored)
├── bin/App                  # final binary (gitignored)
├── scripts/
│   ├── install_deps.sh      # installs bcm2835 + toolchain
│   └── setup_git.sh         # interactive git repository setup
├── docs/                    # documentation (see README)
└── examples/                # (empty) future examples
```

## Module responsibilities

```
                 src/main.cpp
        std::make_unique<Device::Device_t>(argc, argv); run();
                        │
                        ▼
              Device::Device_t (engine)
        ┌───────────────┼────────────────┐
        ▼               ▼                ▼
  AHT21B_t         BH1750_t          SSD1306 (OLED)
  (temp/RH, I2C)   (lux, I2C)        (optional display)
        └───────────────┼────────────────┘
                        ▼
              bcm2835 I2C (i2c-1, /dev/mem)
                        ▼
           AHT21B + BH1750 (+ OLED) on the I2C bus
```

### Layers

1. **bcm2835** — the library maps `/dev/mem` and drives the BCM2835 I2C
   peripheral directly. `bcm2835_i2c_begin()` opens the bus once; each driver
   calls `bcm2835_i2c_setSlaveAddress()` before every transaction, so all
   devices share the same peripheral safely.
2. **Drivers (`AHT21B_t`, `BH1750_t`)** — thin, self-contained wrappers:
   `begin()` (init command) + `read()` (measurement). They know the chip
   register map but nothing about the application.
3. **Engine (`Device::Device_t`)** — owns the hardware lifecycle: parses CLI
   flags (`--version`), loads `config/*.cfg`, initializes bcm2835 + sensors +
   OLED (tolerant to missing hardware), and runs the sampling loop. It does
   not know chip internals, only the driver APIs.
4. **Entry point (`main.cpp`)** — the minimum possible: build the engine with
   `std::make_unique` and call `run()`.

## Design decisions

- **Compile-time version.** The Makefile reads `VERSION` and injects
  `-DVERSION="x.y.z"`; the binary reports the version it was built from, even
  when copied to the Pi without the repo (see `docs/LEARNINGS.md`).
- **Drivers are bus-agnostic.** They only use bcm2835 I2C primitives, so the
  engine could switch to the kernel I2C (`/dev/i2c-1`) without touching them.
- **Fault tolerance.** Missing sensors, OLED or `bcm2835_init()` failure never
  prevent the app from starting; it logs to stderr and keeps running.
- **No explicit `new`/`delete`** in the application code: `main.cpp` uses
  `std::make_unique` and the OLED pointer is owned by the engine's destructor.
