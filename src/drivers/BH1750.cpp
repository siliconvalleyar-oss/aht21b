#include "drivers/BH1750.hpp"

#include "drivers/I2C_bus.hpp"

#include <cstdio>

namespace BH1750 {

namespace {

// Factor de conversión del modo de alta resolución: el valor crudo de 16
// bits se divide entre 1.2 para obtener lux (datasheet BH1750FVI).
constexpr float kLuxScale = 1.2f;

// Tiempo máximo de conversión en modo continua de alta resolución.
constexpr uint32_t kMaxConversionMs = 180;

}  // namespace

BH1750_t::BH1750_t(uint8_t busAddress) : address_(busAddress) {}

bool BH1750_t::writeCommand(uint8_t cmd) {
    // Envía el comando de 1 byte al sensor (con timeout acotado: ver
    // drivers/I2C_bus.hpp).
    return I2C::write(address_, &cmd, 1);
}

bool BH1750_t::begin() {
    // Encender el sensor y seleccionar el modo de medición continua de alta
    // resolución (resolución 1 lx, ~120 ms por muestra).
    if (!writeCommand(Command::kPowerOn) || !writeCommand(Command::kContHResMode)) {
        std::fprintf(stderr, "[BH1750] begin: command rejected on the bus\n");
        return false;
    }
    ready_ = true;
    return true;
}

bool BH1750_t::read(float* lux) {
    if (!ready_ || lux == nullptr) {
        return false;
    }

    // En modo continuo el sensor empuja el resultado sin comando previo:
    // basta con leer los 2 bytes del registro de datos.
    uint8_t raw[2] = {0};
    if (!I2C::read(address_, raw, sizeof(raw))) {
        std::fprintf(stderr, "[BH1750] read failed on the bus\n");
        return false;
    }

    // Convertir el valor de 16 bits (MSB primero) a lux dividiendo por 1.2.
    const uint16_t value = static_cast<uint16_t>(
        (static_cast<uint16_t>(raw[0]) << 8) | raw[1]);
    *lux = static_cast<float>(value) / kLuxScale;
    return true;
}

bool BH1750_t::powerDown() {
    return writeCommand(Command::kPowerDown);
}

}  // namespace BH1750
