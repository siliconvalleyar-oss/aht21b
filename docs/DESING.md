# DESING — Decisiones de diseño

> El nombre del archivo es `DESING.md` (así lo fija la estructura del prompt);
> su contenido trata del diseño ("design") del proyecto.

## 1. bcm2835 como única vía al hardware

La librería bcm2835 (v1.71) da acceso directo al periférico I2C de la BCM2835
vía `/dev/mem`, sin depender del driver del kernel ni de root estricto para
los sensores (el OLED sí necesita `/dev/mem`). Alternativa considerada: el
driver del kernel `/dev/i2c-1` (usado por `i2c-tools`). Se eligió bcm2835 por
coherencia con la familia de proyectos del autor y porque el prompt lo exige
explícitamente.

## 2. Drivers autocontenidos por chip

`AHT21B_t` y `BH1750_t` encapsulan todo el protocolo del chip (comandos,
decodificación, CRC8). El motor no conoce registros: solo llama
`begin()`/`read()`. Esto permite:
- Probar cada driver de forma aislada.
- Migrar el bus (p. ej. a `/dev/i2c-1`) sin tocar la lógica de aplicación.

## 3. Versión en tiempo de compilación

El archivo `VERSION` es la única fuente de la versión; el Makefile la inyecta
con `-DVERSION="x.y.z"`. Consecuencias:
- El binario siempre reporta la versión con la que se compiló (aunque se copie
  a una Pi sin el repo).
- No hay lectura de archivos en runtime → sin dependencias de cwd.
- El fallback `"0.0.0"` solo cubre compilaciones manuales fuera del Makefile.

## 4. Tolerancia a fallos

`bcm2835_init()` falla sin `/dev/mem`; un sensor puede estar mal cableado; el
OLED puede no existir. La app en ninguno de esos casos deja de funcionar:
avisa por stderr, marca el canal como `no data` y sigue su bucle.

## 5. Sin `new`/`delete` en la aplicación

`main.cpp` construye el motor con `std::make_unique`. El único puntero crudo
es el del OLED, propiedad exclusiva del motor y liberado en su destructor.
Esto garantiza limpieza del hardware (bcm2835_close + OLEDPowerDown) aunque
`run()` termine por excepción o Ctrl+C.

## 6. Estructura espejo obj/

`obj/` replica la jerarquía de `src/` (`src/engine/foo.cpp` →
`obj/engine/foo.o`), evitando colisiones de nombres y facilitando el build
cruzado con overrides de variables del Makefile.
