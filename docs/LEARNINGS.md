# LEARNINGS — Reglas y Aprendizajes del Proyecto AHT21B_bh1750

## Reglas de versión de la aplicación

- **La versión se define en tiempo de compilación**, NO en runtime: el
  Makefile lee el archivo `VERSION` de la raíz y la inyecta con
  `-DVERSION="x.y.z"` en todos los objetos.
- El binario muestra la versión al iniciar ("AHT21B_bh1750 v0.1.0") y con
  `./bin/App --version`.
- Al copiar el binario a otra máquina (sin el archivo `VERSION`), reporta la
  versión con la que se compiló.
- El fallback `"0.0.0"` en el código solo cubre compilaciones manuales fuera
  del Makefile.

## Git / Versionado

**Todo push debe llevar su tag.** No se pushea sin tag. El tag identifica la versión.

### Flujo de versionado

1. Obtener el último tag publicado (ej: `v0.1.0`).
2. El archivo `VERSION` debe coincidir con ese tag (sin `v`).
3. Calcular la siguiente versión: `tag + 0.0.1` (ej: `v0.1.0` → `0.1.1`).
4. Actualizar `VERSION` a la nueva versión.
5. Confirmar cambios y pushear con el nuevo tag.

### Reglas

- **Los mensajes de commit deben seguir conventional commits**: `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:`.
- **No eliminar tags publicados.** Si hay error, crear nuevo tag.
- **El tag y `VERSION` siempre deben coincidir** (tag con `v`, `VERSION` sin `v`).
- **VERSION siempre refleja el último tag.** El archivo `VERSION` en la raíz del proyecto debe contener exactamente el número del último tag publicado (sin prefijo `v`). Ejemplo: si el último tag es `v0.1.5`, el archivo `VERSION` contiene `0.1.5`.
- **Cada commit significativo debe tener su tag.** No se salta ningún número de versión. Si se salta un número, se pierde la secuencia.
- **Tag = VERSION.** Cada vez que se hace push de un tag, el archivo `VERSION` debe actualizarse al mismo número (sin `v`). La relación es: `git tag v0.1.5` → `VERSION` = `0.1.5`.
- **El ciclo patch 0-9 es obligatorio.** No se puede pasar de `v0.1.9` a `v0.2.1`. Debe ir a `v0.2.0`. Esto asegura que cada minor tenga exactamente 10 patches.
- **No se puede retroceder de versión.** Una vez publicado un tag, no se puede reemplazar. Si hay un error, se crea un nuevo tag con el siguiente número en la secuencia.
- **El archivo VERSION empieza en 0.1.0** que corresponde al tag `v0.1.0`.

### Automatización

`scripts/tag_and_push.sh` ejecuta el ciclo completo de forma mecánica:

```bash
./scripts/tag_and_push.sh                    # sin cambios pendientes
./scripts/tag_and_push.sh "feat: mensaje"    # commitea lo pendiente y sigue
```

Calcula la siguiente versión desde el último tag (con el ciclo patch 0-9),
actualiza `VERSION`, commitea el bump, crea el tag y pushea `main` + tag.
Todo push debe pasar por este script (o repetir exactamente sus pasos) para
que ningún push quede sin tag.

### ¿Para qué sirve este esquema?

- Cada versión es única e inmutable
- Se puede saber exactamente cuántas versiones han existido
- No hay ambigüedad sobre qué versión sigue
- Compatible con semver estricto

---

## Git Push — Credenciales y Token

### Problema

El push falla con `403 Permission denied` cuando se usa HTTPS con el remote.

### Causa

El credential helper del sistema almacena credenciales que no tienen permisos de escritura en el repositorio.

### Solución — Token en config global

El token de GitHub (`ghp_...`) está configurado en la **config global de git**:

```bash
git config --global --list | grep -i "user.password"
```

### Proceso de Push

```bash
# 1. Temporalmente configurar remote con token
git remote set-url origin https://USUARIO:<TOKEN>@github.com/USUARIO/REPOSITORIO.git

# 2. Push
git push origin main --tags

# 3. Limpiar remote (quitar token de la URL)
git remote set-url origin https://github.com/USUARIO/REPOSITORIO.git
```

**IMPORTANTE:** Siempre limpiar la URL después del push para no exponer el token.

### Verificación de credenciales

```bash
git remote -v
git config user.name
git config user.email
git ls-remote origin
git status
git log --oneline -5
```

---

## Errores Comunes

### 403 Permission denied
- **Causa:** usuario sin permisos de escritura
- **Solución:** usar token con permisos de push

### SSH Permission denied
- **Causa:** SSH key no configurada en la cuenta
- **Solución:** agregar `~/.ssh/id_ed25519.pub` en GitHub → Settings → SSH Keys

---

## Licencia

- Proyecto AHT21B_bh1750: **MIT** (ver `LICENSE`).
- Librería OLED SSD1306 (Gavin Lyons): licencia de su repositorio
  `SSD1306_OLED_RPI` (incluida en los encabezados de `include/oled/`).
- `nlohmann/json.hpp`: MIT (single header de Niels Lohmann).
