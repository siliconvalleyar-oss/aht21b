# ACTIVITY — Registro de actividades

Bitácora cronológica de las actividades del proyecto (generación, cambios,
pruebas y despliegues). Se actualiza en cada sesión de trabajo.

| Fecha | Actividad | Resultado |
|---|---|---|
| 2026-08-17 | Build cruzado armhf (GCC 10 + sysroot con bcm2835) de todo el proyecto | ✅ `bin/App` ELF 32-bit ARM (GLIBC_2.4, GLIBCXX_3.4.21); sin errores (2 warnings de la librería SSD1306) |
| 2026-08-17 | Generación completa del proyecto según prompt.md: drivers AHT21B/BH1750, esqueleto HMC5883L, motor Device_t, Makefile con `-DVERSION`, scripts y 26 docs | ✅ Todo el código y la documentación creados |
| _fecha_ | _prueba en hardware real (Pi)_ | _pendiente_ |
| _fecha_ | _implementación del magnetómetro HMC5883L_ | _pendiente (ROADMAP)_ |

## Convención

- Cada entrada se agrega **arriba** de la tabla (más reciente primero).
- Las tareas en curso se marcan `⏳`; las terminadas, `✅`.
