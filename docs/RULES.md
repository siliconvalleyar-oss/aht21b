# RULES — Reglas del proyecto

## Git / versionado (resumen de LEARNINGS.md)

- **Todo push lleva su tag** (`vX.Y.Z`). No se pushea sin tag.
- **Tag = VERSION**: el tag lleva `v` (`v0.1.0`) y el archivo `VERSION` lleva
  el número sin `v` (`0.1.0`). Siempre coinciden.
- **Ciclo patch 0-9 obligatorio**: de `v0.1.9` se pasa a `v0.2.0`, nunca a
  `v0.2.1` directamente. Cada minor tiene 10 patches.
- **No se retrocede de versión** ni se eliminan tags publicados; si hay error,
  se crea el siguiente número.
- **Cada commit significativo recibe su tag** (no se salta la secuencia).

## Commits

- Conventional Commits: `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`,
  `test:`, `build:`, `ci:`, `perf:`, `style:`.

## Código

- C++17, sin `new`/`delete` explícitos (usar `std::make_unique`).
- Todo comentado (español o inglés consistente).
- Los drivers son autocontenidos y no dependen de la aplicación.

## Documentación

- Todo cambio relevante actualiza `docs/CHANGELOG.md` y `docs/ACTIVITY.md`.
- Los `.md` nuevos van en `docs/` y se referencian desde el README.
- **Regla REPORT.md**: quien lea `docs/REPORT.md` debe completarlo a medida
  que cumple cada punto (marcar `[x]`, registrar resultados).

## Flujo de trabajo

- Editar y commitear **local**; compilar y probar **en la Pi** (o cross).
- No editar archivos directo en la Pi; solo `git pull` + build.
