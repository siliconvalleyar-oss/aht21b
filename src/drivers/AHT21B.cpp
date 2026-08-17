#include "drivers/AHT21B.hpp"

#include "drivers/I2C_bus.hpp"

#include <bcm2835.h>

#include <cstdio>

namespace AHT21B {

namespace {

// Duración de la conversión de una medición (datasheet: 80 ms típico).
constexpr uint32_t kConversionDelayMs = 80;

// Tamaño del paquete de respuesta de una medición: 1 byte de estado, 2 bytes
// de humedad (20 bits útiles), 2 bytes de temperatura (20 bits útiles) y 1
// byte de CRC.
constexpr size_t kMeasurePacketBytes = 6;

}  // namespace

AHT21B_t::AHT21B_t(uint8_t busAddress) : address_(busAddress) {}

bool AHT21B_t::begin() {
    // El comando de inicialización (0xBE 0x08 0x00) reconfigura el sensor;
    // también sirve como comprobación de que responde en el bus.
    if (!writeCommand(Command::kInit, 0x08, 0x00)) {
        std::fprintf(stderr, "[AHT21B] init command rejected on the bus\n");
        return false;
    }

    // Tras el init el sensor queda ocupado calibrando (estira SCL mientras
    // está ocupado; disparar una medición en ese momento hace timeout). Se
    // espera y se sondea el byte de estado hasta que deje de estar ocupado
    // (bit 7) y esté calibrado (bit 3).
    bcm2835_delay(50);
    uint8_t status = 0x80;  // asumir ocupado para entrar al bucle
    for (int attempt = 0; attempt < 10 && (status & 0x80); ++attempt) {
        if (!I2C::read(address_, &status, 1)) {
            std::fprintf(stderr, "[AHT21B] status read failed during init\n");
            return false;
        }
        if (status & 0x80) {
            bcm2835_delay(20);
        }
    }
    if (status & 0x80) {
        std::fprintf(stderr, "[AHT21B] sensor still busy after init (status 0x%02X)\n",
                     status);
    } else if (!(status & 0x08)) {
        std::fprintf(stderr,
                     "[AHT21B] sensor not calibrated after init (status 0x%02X)\n",
                     status);
    }
    ready_ = true;
    return true;
}

bool AHT21B_t::writeCommand(uint8_t c0, uint8_t c1, uint8_t c2,
                            uint32_t timeoutUs) {
    // Envía el comando de 3 bytes al sensor en una sola transacción (con
    // timeout acotado: ver drivers/I2C_bus.hpp).
    const uint8_t cmd[3] = {c0, c1, c2};
    return I2C::write(address_, cmd, sizeof(cmd), timeoutUs);
}

bool AHT21B_t::read(float* temperatureC, float* humidityPct) {
    if (!ready_ || temperatureC == nullptr || humidityPct == nullptr) {
        return false;
    }

    // 1) Disparar la medición (0xAC 0x33 0x00) y esperar la conversión. El
    //    trigger necesita un timeout generoso: el sensor estira SCL durante
    //    la conversión (~80 ms según datasheet), que arranca dentro del
    //    propio write; con 100 ms quedaba al límite y fallaba intermitente.
    if (!writeCommand(Command::kTrigger, 0x33, 0x00, 500000)) {
        std::fprintf(stderr, "[AHT21B] trigger command rejected\n");
        return false;
    }
    bcm2835_delay(kConversionDelayMs);

    // 2) Leer el paquete de 6 bytes.
    uint8_t raw[kMeasurePacketBytes] = {0};
    if (!I2C::read(address_, raw, sizeof(raw))) {
        std::fprintf(stderr, "[AHT21B] read failed on the bus\n");
        return false;
    }

    // 3) Validar el CRC del paquete (polinomio 0x31, init 0xFF; ver
    //    AHT21B_decode.hpp). Algunos módulos AHT21B no lo calculan
    //    correctamente (verificado en la Pi: el byte de CRC nunca valida
    //    aunque los datos son consistentes). Se avisa pero NO se rechaza la
    //    lectura; la validación real la hace el rango físico del paso 4.
    const uint8_t crc = crc8(raw, sizeof(raw) - 1);
    if (crc != raw[sizeof(raw) - 1]) {
        std::fprintf(stderr,
                     "[AHT21B] CRC mismatch (got 0x%02X, want 0x%02X) - "
                     "accepting if in physical range\n",
                     crc, raw[sizeof(raw) - 1]);
    }

    // 4) Decodificar los valores de 20 bits y validar el rango físico
    //    (atrapa paquetes corruptos aunque el CRC del módulo no funcione).
    if (!decodeMeasurement(raw, temperatureC, humidityPct)) {
        std::fprintf(stderr, "[AHT21B] reading out of physical range\n");
        return false;
    }
    return true;
}

}  // namespace AHT21B
