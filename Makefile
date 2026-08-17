# ============================================================================
# AHT21B_bh1750 - Raspberry Pi environment monitor (AHT21B + BH1750 + OLED)
#
# Reads temperature/humidity from an AHT21B and light from a BH1750 over I2C
# using the bcm2835 library, with an optional SSD1306 OLED display. C++17.
# Works on Raspberry Pi OS 32-bit (armhf) and 64-bit (arm64).
#
# Build layout (obj/ mirrors src/):
#   src/engine/foo.cpp  ->  obj/engine/foo.o
#   src/drivers/foo.cpp ->  obj/drivers/foo.o
# Binary: bin/App
#
# Version: the VERSION file at the repo root is the single source of truth;
# it is injected at COMPILE TIME as -DVERSION="x.y.z" (never read at runtime).
#
# Cross-compile overrides (see scripts/cross notes in docs/BUILD.md):
#   make CXX=<cross-g++-10> CXXFLAGS_EXTRA="<sysroot -isystem flags>" \
#        BCM2835_INCLUDE=<sysroot>/include BCM2835_LIBDIR=<sysroot>/lib \
#        LDFLAGS="<sysroot link flags>" CRT_BEGIN="..." CRT_END="..."
# ============================================================================

SHELL      := /bin/bash

PROJECT    := AHT21B_bh1750
BINARY     := App

SRC_DIR    := src
INC_DIR    := include
OBJ_DIR    := obj
BIN_DIR    := bin

CXX        ?= g++
CXXSTD     := c++17

# Application version: baked in at compile time from the VERSION file.
APP_VERSION := $(shell cat VERSION 2>/dev/null || echo 0.0.0)

# Where the bcm2835 library lives. scripts/install_deps.sh installs it into
# /usr/local by default; override for cross builds.
BCM2835_INCLUDE ?= /usr/local/include
BCM2835_LIBDIR  ?= /usr/local/lib

WARNINGS   := -Wall -Wextra -Wpedantic -Wshadow
OPT_FLAGS  := -O2

# Extra flags for cross-builds (e.g. -nostdinc + sysroot -isystem flags).
CXXFLAGS_EXTRA ?=

# No architecture-specific flags are needed by default: Raspberry Pi OS
# 32-bit ships an armv6-compatible toolchain (runs on every Pi) and the
# 64-bit image uses aarch64 defaults. To tune explicitly, e.g. for the Pi
# Zero/1 32-bit:  make ARCH_FLAGS="-march=armv6zk -mfpu=vfp -mfloat-abi=hard"
ARCH_FLAGS ?=

CXXFLAGS   := $(OPT_FLAGS) $(ARCH_FLAGS) -std=$(CXXSTD) $(WARNINGS) \
              -I$(INC_DIR) -I$(INC_DIR)/oled \
              -I$(BCM2835_INCLUDE) $(CXXFLAGS_EXTRA) -MMD -MP \
              -DVERSION=\"$(APP_VERSION)\"
LDLIBS     := -lbcm2835
LDFLAGS    ?= -L$(BCM2835_LIBDIR) $(LDLIBS) -lm -pthread

# Cross-build startup objects (empty on native builds): crt1.o/crti.o/crtn.o
# matched to the target's glibc, plus the toolchain's crtbegin/crtend.
CRT_BEGIN  ?=
CRT_END    ?=

# Discover sources and mirror the tree into obj/.
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp' | sort)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

TARGET     := $(BIN_DIR)/$(BINARY)

.PHONY: all clean distclean install run test help

all: $(TARGET)

# ---- Binary ----------------------------------------------------------------

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CRT_BEGIN) $(OBJS) $(CRT_END) -o $@ $(LDFLAGS)

# VERSION is a real prerequisite (not order-only) so that any version bump
# forces every object to rebuild and the new -DVERSION="x.y.z" gets baked in.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp VERSION
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# ---- Tests (sin hardware) ---------------------------------------------------

# Unit tests de decodificación (AHT21B CRC/20-bit, BH1750 lux): funciones
# puras, se compilan con el compilador del host (TEST_CXX) y no requieren
# bcm2835 ni I2C.
TEST_CXX  ?= g++
TEST_SRC  := tests/test_decode.cpp
TEST_BIN  := $(BIN_DIR)/test_decode

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) | $(BIN_DIR)
	$(TEST_CXX) -std=$(CXXSTD) -Wall -Wextra -I$(INC_DIR) -o $@ $(TEST_SRC)

# ---- Convenience ------------------------------------------------------------

# Run the app (extra args con ARGS="--version"). Requiere root para el I2C
# de bcm2835 (/dev/mem), igual que examples/Makefile usa sudo para /dev/i2c-*.
# Sin sudo la app corre en modo consola (sin lecturas).
run: all
	sudo $(TARGET) $(ARGS)

# Install the binary to /usr/local/bin (DESTDIR supported for packaging).
install: all
	install -d $(DESTDIR)/usr/local/bin
	install -m 0755 $(TARGET) $(DESTDIR)/usr/local/bin/$(BINARY)

clean:
	rm -rf $(OBJ_DIR)

distclean: clean
	rm -rf $(BIN_DIR)

help:
	@echo "Targets:"
	@echo "  all       - build bin/App (default)"
	@echo "  run       - run the app (ARGS='--version' for extra options)"
	@echo "  install   - copy bin/App to /usr/local/bin (DESTDIR supported)"
	@echo "  clean     - remove obj/"
	@echo "  distclean - remove obj/ and bin/"
	@echo ""
	@echo "Version injected at compile time: $(APP_VERSION)"
	@echo "Dependencies: bcm2835 (scripts/install_deps.sh)"

# Pull in auto-generated dependency files.
-include $(DEPS)
