# BUILD — Compilación

## En la Raspberry Pi (32 o 64 bits)

```bash
# 1. Dependencias (una vez)
sudo bash scripts/install_deps.sh        # instala g++, make, git y bcm2835

# 2. Compilar
make clean && make -j4

# 3. Probar
./bin/App --version
```

El binario queda en `bin/App`. Los objetos se generan en `obj/` reflejando la
jerarquía de `src/` (ej. `src/engine/Device_t.cpp` → `obj/engine/Device_t.o`).

## Targets del Makefile

| Target | Efecto |
|---|---|
| `make` | Compila `bin/App` (default) |
| `make run` | Ejecuta la app (extra args con `ARGS="--version"`) |
| `make install` | Copia `bin/App` a `/usr/local/bin` (`DESTDIR` soportado) |
| `make clean` | Borra `obj/` |
| `make distclean` | Borra `obj/` y `bin/` |
| `make help` | Muestra ayuda y la versión inyectada |

## Versión en tiempo de compilación

El Makefile lee el archivo `VERSION` de la raíz y lo inyecta en **todos** los
objetos con `-DVERSION="x.y.z"`. Para verificar la versión de un binario
concreto: `./bin/App --version`.

## Arquitecturas soportadas

- **Raspberry Pi OS 32-bit (armhf)**: funciona sin flags extra (el toolchain
  del sistema apunta a armv6, compatible con toda la familia Pi).
- **Raspberry Pi OS 64-bit (arm64)**: funciona con los defaults de aarch64.
- Ajustes explícitos opcionales: `make ARCH_FLAGS="-march=armv6zk -mfpu=vfp -mfloat-abi=hard"`.

## Build cruzado (opcional, desde PC x86_64)

El Makefile acepta overrides para toolchains cruzados:

```bash
make CXX=/mnt/disk/gcc10-cross/usr/bin/arm-linux-gnueabihf-g++-10 \
     CXXFLAGS_EXTRA="<sysroot -isystem flags>" \
     BCM2835_INCLUDE=<sysroot>/include \
     BCM2835_LIBDIR=<sysroot>/lib \
     LDFLAGS="<sysroot link flags>" \
     CRT_BEGIN="<crt1.o crti.o crtbegin.o>" CRT_END="<crtend.o crtn.o>"
```

> Requiere un sysroot armhf con la librería bcm2835 compilada (ver el patrón
> del proyecto hermano inmp441_rpi).
