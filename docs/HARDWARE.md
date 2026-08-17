# HARDWARE — Conexionado y sensores

Todos los dispositivos usan el **bus I2C-1** de la Raspberry Pi (GPIO 2 = SDA,
GPIO 3 = SCL, alimentación 3.3 V).

## Mapa de pines

| Dispositivo | Pin | GPIO Pi | Pin físico |
|---|---|---|---|
| AHT21B | VIN | 3.3 V | 1 o 17 |
| AHT21B | GND | GND | 6, 9, 14… |
| AHT21B | SDA | GPIO 2 | 3 |
| AHT21B | SCL | GPIO 3 | 5 |
| BH1750 | VCC | 3.3 V | 1 o 17 |
| BH1750 | GND | GND | 6, 9, 14… |
| BH1750 | SDA | GPIO 2 | 3 |
| BH1750 | SCL | GPIO 3 | 5 |
| BH1750 | ADDR | GND (0x23) o 3.3 V (0x5C) | — |
| OLED SSD1306 | VCC | 3.3 V | 1 o 17 |
| OLED SSD1306 | GND | GND | 6, 9, 14… |
| OLED SSD1306 | SDA | GPIO 2 | 3 |
| OLED SSD1306 | SCL | GPIO 3 | 5 |

## Diagrama de conexión

```
  Raspberry Pi                 AHT21B               BH1750
  ┌───────────────┐            ┌────────┐           ┌────────┐
  │ 3.3 V ────────┼───────────►│ VIN    │           │ VCC    │◄──┐
  │ GND  ────────┼───────────►│ GND    │           │ GND    │◄──┤
  │ GPIO 2 (SDA) ┼───────────►│ SDA    │           │ SDA    │   │
  │ GPIO 3 (SCL) ┼───────────►│ SCL    │           │ SCL    │   │
  │              │            │        │           │ ADDR ──┘   │
  │              │            └────────┘           └────────┘   │
  │ 3.3 V/GND ───┼──────────────────────────────────────────────┘
  │              │                  OLED SSD1306 (I2C 0x3C)
  └───────────────┘
```

> **Recomendación**: usar cables cortos (< 15 cm) y un capacitor de 100 nF
> entre VCC y GND de cada módulo si hay ruido en las lecturas.

## Direcciones I2C

| Dispositivo | Dirección | Configurable |
|---|---|---|
| AHT21B | 0x38 (56) | No |
| BH1750 | 0x23 (35) / 0x5C con ADDR alto | Pin ADDR |
| OLED SSD1306 | 0x3C (60) | 0x3D alternativo |

Las direcciones se pueden ajustar en `config/config.cfg` (valores decimales).

## Verificación rápida

```bash
# Listar dispositivos del bus (requiere i2c-tools)
sudo apt install i2c-tools
i2cdetect -y 1
# Se esperan: 0x23 (BH1750), 0x38 (AHT21B) y 0x3C (OLED)
```
