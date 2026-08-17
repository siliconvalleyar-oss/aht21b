# WORKFLOW — Flujo de Desarrollo

Flujo de trabajo establecido para el proyecto **AHT21B_bh1750**.

## Principios

- **Cambios de codigo:** se hacen exclusivamente a nivel **local**, en `$PWD`
  (el repo local). Nada se edita directo en la maquina remota.
- **Compilacion:** se realiza **solo de forma remota**, en la Raspberry Pi
  (o con build cruzado desde la PC usando el sysroot armhf).
- **Pruebas:** se ejecutan **solo de forma remota**, en la Raspberry Pi.
- **Generacion y modificacion:** todo archivo generado o modificado vive en
  `$PWD` local. No se dejan archivos generados en el remoto.

## Flujo paso a paso

1. **Editar local:** realizar los cambios de codigo/documentacion en `$PWD`.
2. **Commit local:** commitear con mensajes semanticos
   (`feat:`, `fix:`, `chore:`, `docs:`, etc.).
3. **Push:** pushear los commits a `origin/main` **con su tag** (ver abajo).
4. **Actualizar remoto:** en la Pi, `git pull` para traer los cambios.
5. **Compilar remoto:** `make clean && make -j4` en la Pi.
6. **Probar remoto:** `./bin/App --version` y bucle de lecturas con sensores.

## Comandos de referencia

```bash
# Edicion local (en $PWD)
vim src/engine/Device_t.cpp

# Commit y push local
git add <archivos>
git commit -m "fix: ..."
git push origin main

# Bump de version y tag (cuando corresponda)
echo "0.1.1" > VERSION
git add VERSION && git commit -m "chore: bump version to 0.1.1"
git tag v0.1.1
git push origin main && git push origin v0.1.1

# Actualizar, compilar y probar en la Pi (solo remoto)
ssh pi@<ip> "cd AHT21B_bh1750 && git pull --ff-only && make clean && make -j4 && ./bin/App --version"
```

## Version de la app

Las reglas detalladas de versionado estan en [docs/LEARNINGS.md](LEARNINGS.md)
(seccion "Git / Versionado"). Resumen:

- **Todo push debe llevar su tag.** No se pushea sin tag.
- **Tag = VERSION.** El tag lleva `v` (`v0.1.1`) y el archivo `VERSION`
  lleva el mismo numero sin `v` (`0.1.1`). Siempre deben coincidir.
- **Proximo numero:** tag actual + 1 en el ultimo segmento, respetando el
  ciclo patch 0-9 (ej: `v0.1.8` → `0.1.9`; `v0.1.9` → `0.2.0`).
- **Ciclo patch 0-9 obligatorio:** no se pasa de `v0.1.9` a `v0.2.1`; debe ir a
  `v0.2.0`. Cada minor tiene exactamente 10 patches (0 a 9).
- **Cada commit significativo debe tener su tag.** No se salta ningun numero
  de version (no se pierde la secuencia).
- **No eliminar tags publicados** y **no retroceder de version.** Si hay un
  error, se crea un nuevo tag con el siguiente numero de la secuencia.
- El versionado arranco en `0.1.0` (`v0.1.0`).

### Como hacer un bump

1. Obtener el ultimo tag publicado: `git tag --sort=-version:refname | head -1`
   (ej: `v0.1.9`).
2. Verificar que `VERSION` coincida con ese tag (sin `v`).
3. Calcular el siguiente numero segun el ciclo patch 0-9.
4. Actualizar `VERSION` con el nuevo numero.
5. Commit `chore: bump version to X.Y.Z`, tag `vX.Y.Z` y push.

```bash
echo "0.2.0" > VERSION
git add VERSION && git commit -m "chore: bump version to 0.2.0"
git tag v0.2.0
git push origin main && git push origin v0.2.0
```

### Version en la app

- El archivo `VERSION` (raiz del proyecto) es la fuente de la version.
- El Makefile la inyecta en el build con `-DVERSION="$(cat VERSION)"`.
- Se muestra en el banner de presentacion de la app y con `./bin/App --version`.

## Recordatorios

- No compilar ni probar en la maquina local (salvo build cruzado verificado).
- No editar archivos dentro del repo remoto de la Pi; solo `git pull` y build.
- La password de la Pi va en `$SSHPASS`; nunca mostrarla en pantalla.
