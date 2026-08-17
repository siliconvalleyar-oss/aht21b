# CONTRIBUTING — Cómo contribuir

## Flujo de trabajo

1. **Editar localmente** (en este repo). Nada se edita directo en la Pi.
2. **Commit** con [Conventional Commits](https://www.conventionalcommits.org/):
   `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:`.
3. **Push con tag** — regla obligatoria del proyecto (ver
   [LEARNINGS.md](LEARNINGS.md)): todo push lleva su tag `vX.Y.Z` y el archivo
   `VERSION` debe coincidir con el tag (sin `v`). Ciclo patch 0-9.
4. Compilar y probar **en la Pi** (o con build cruzado) antes de cerrar.

## Normas de código

- C++17, nombres descriptivos, `const`-correctness, sin `new`/`delete`
  explícitos (usar `std::make_unique`).
- Todo método y clase documentado con comentarios (español o inglés,
  consistente).
- Los drivers de sensores deben ser autocontenidos (solo primitivas bcm2835).
- Sin regresión de versión: cada bump suma un número, nunca retrocede.

## Documentación

- Si un `.md` nuevo es necesario, crearlo en `docs/` y referenciarlo desde el
  índice del [README](../README.md).
- Actualizar `CHANGELOG.md` y `ACTIVITY.md` con cada cambio relevante.
- Completar los checklists de `TODO.md`/`REPORT.md` a medida que se cumplen.

## Tests

- Verificar compilación limpia: `make clean && make -j4`.
- En hardware: seguir el checklist de [TESTING.md](TESTING.md).
