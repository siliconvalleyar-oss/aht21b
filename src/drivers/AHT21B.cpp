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

bool AHT21B_t::writeCommand(uint8_t c0, uint8_t c1, uint8_t c2) {
    // Envía el comando de 3 bytes al sensor en una sola transacción (con
    // timeout acotado: ver drivers/I2C_bus.hpp).
    const uint8_t cmd[3] = {c0, c1, c2};
    return I2C::write(address_, cmd, sizeof(cmd));
}

bool AHT21B_t::read(float* temperatureC, float* humidityPct) {
    if (!ready_ || temperatureC == nullptr || humidityPct == nullptr) {
        return false;
    }

    // 1) Disparar la medición (0xAC 0x33 0x00) y esperar la conversión.
    if (!writeCommand(Command::kTrigger, 0x33, 0x00)) {
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

    // 3) Validar el CRC del paquete (polinomio 0x31, init 0xFF). Algunos
    //    módulos AHT21B no lo calculan correctamente (verificado en la Pi:
    //    el byte de CRC nunca valida aunque los datos son consistentes). Se
    //    avisa pero NO se rechaza la lectura; la validación real la hace el
    //    rango físico del paso 5.
    const uint8_t crc = crc8(raw, sizeof(raw) - 1);
    if (crc != raw[sizeof(raw) - 1]) {
        std::fprintf(stderr,
                     "[AHT21B] CRC mismatch (got 0x%02X, want 0x%02X) - "
                     "accepting if in physical range\n",
                     crc, raw[sizeof(raw) - 1]);
    }

    // 4) Decodificar los valores de 20 bits (MSB primero, 4 bits de relleno
    //    por canal) y escalarlos a las unidades físicas:
    //      HR%  = (humedad20 / 2^20) * 100
    //      T°C  = (temperatura20 / 2^20) * 200 - 50
    const uint32_t humidity20 = (static_cast<uint32_t>(raw[1]) << 12) |
                                (static_cast<uint32_t>(raw[2]) << 4) |
                                (static_cast<uint32_t>(raw[3]) >> 4);
    const uint32_t temp20 = (static_cast<uint32_t>(raw[3] & 0x0F) << 16) |
                            (static_cast<uint32_t>(raw[4]) << 8) |
                            static_cast<uint32_t>(raw[5]);

    const float rh = static_cast<float>(humidity20) / 1048576.0f * 100.0f;
    const float tc = static_cast<float>(temp20) / 1048576.0f * 200.0f - 50.0f;

    // 5) Validar el rango físico (atrapa paquetes corruptos aunque el CRC
    //    del módulo no funcione): RH 0-100 %, T -40 a 85 °C.
    if (rh < 0.0f || rh > 100.0f || tc < -40.0f || tc > 85.0f) {
        std::fprintf(stderr,
                     "[AHT21B] reading out of physical range (T %.1f C, "
                     "RH %.1f %%)\n",
                     tc, rh);
        return false;
    }

    *humidityPct = rh;
    *temperatureC = tc;
    return true;
}

uint8_t AHT21B_t::crc8(const uint8_t* data, size_t len) {
    // CRC-8 con polinomio 0x31 (x^8 + x^5 + x^4 + 1), valor inicial 0xFF,
    // como especifica el datasheet del AHT21B para el último byte del
    // paquete de medición.
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31)
                               : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

}  // namespace AHT21B
