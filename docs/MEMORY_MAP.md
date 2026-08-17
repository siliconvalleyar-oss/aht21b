# MEMORY_MAP — Registros I2C de los sensores

## AHT21B (dirección 0x38)

| Comando | Valor | Descripción |
|---|---|---|
| Inicialización | `0xBE 0x08 0x00` | Reconfigura y calibra el sensor |
| Disparo de medición | `0xAC 0x33 0x00` | Inicia una conversión (~80 ms) |

**Paquete de respuesta (6 bytes):**

| Byte | Contenido |
|---|---|
| 0 | Estado (bit 7 = busy, bit 3 = calibrado) |
| 1-2 | Humedad (20 bits: byte1 << 12 \| byte2 << 4 \| byte3 >> 4) |
| 3-5 | Temperatura (20 bits: (byte3 & 0x0F) << 16 \| byte4 << 8 \| byte5) |
| 5 | CRC-8 (polinomio 0x31, init 0xFF) |

**Escalado:** `HR% = (hum20 / 2^20) × 100`; `T°C = (temp20 / 2^20) × 200 − 50`.

## BH1750 (dirección 0x23 / 0x5C)

| Comando | Valor | Descripción |
|---|---|---|
| Power down | `0x00` | Modo de bajo consumo |
| Power on | `0x01` | Despierta el sensor |
| Reset | `0x07` | Reinicia el registro |
| Medición continua H-res | `0x10` | 1 lx, ~120 ms/muestra |

**Registro de datos (2 bytes):** `valor = (msb << 8) | lsb`; `lux = valor / 1.2`.

## SSD1306 OLED (dirección 0x3C)

Bus de comandos estándar del SSD1306 (control byte 0x00 = comando,
0x40 = dato). La librería `SSD1306_OLED_RPI` (Gavin Lyons) encapsula la
secuencia de init (contraste, charge pump, multiplex, etc.); ver
`include/oled/SSD1306_OLED.hpp`.

## HMC5883L (dirección 0x1E, pendiente de implementar)

| Registro | Dirección | Descripción |
|---|---|---|
| ConfigA | 0x00 | Promedios, rate y modo de medición |
| ConfigB | 0x01 | Ganancia del sensor |
| Mode | 0x02 | Modo continuo / single / idle |
| Data X | 0x03 | Eje X (2 bytes) |
| Data Z | 0x05 | Eje Z (2 bytes) |
| Data Y | 0x07 | Eje Y (2 bytes) |
