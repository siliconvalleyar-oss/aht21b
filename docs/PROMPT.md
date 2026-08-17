# PROMPT — Instrucciones del proyecto

Este archivo es una copia de referencia de las instrucciones originales del
proyecto (el original vive en `prompt.md`, en la raíz, y no se versiona).

## Resumen ejecutivo

Generar un proyecto **C++ completo para Raspberry Pi** (32/64 bits) con la
librería **bcm2835** y el sensor **AHT21B**, con la estructura de carpetas
obligatoria, Makefile funcional, documentación completa y configuración de
repositorio Git:

- `main.cpp` con `std::make_unique<Device::Device_t>()` y `run()`.
- Clase `Device::Device_t` con toda la lógica (bucle, sensores, banner).
- **Versión en tiempo de compilación**: `-DVERSION="$(cat VERSION)"`,
  mostrada al inicio y con `--version`.
- Scripts: `install_deps.sh` (bcm2835), `setup_git.sh` (repo Git
  interactivo con `gh` o manual).
- `docs/`: completar TODOS los `.md` (LEARNINGS.md y WORKFLOW.md
  obligatorios de leer y completar).
- Código 100 % comentado (propósito, parámetros, decisiones).

## Estado de cumplimiento

| Requisito | Estado |
|---|---|
| Estructura de carpetas | ✅ |
| Código (drivers, motor, main) | ✅ |
| Makefile con `-DVERSION` | ✅ |
| Scripts | ✅ |
| Documentación completa | ✅ |
| Compilación verificada | ⏳ pendiente de ejecutar |

Ver [TODO.md](TODO.md) para el checklist detallado.
