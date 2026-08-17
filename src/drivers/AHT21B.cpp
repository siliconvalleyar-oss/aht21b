#include "drivers/AHT21B.hpp"

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
    // El comando de inicialización reconfigura el sensor y lo deja listo para
    // medir; también sirve como comprobación de que responde en el bus.
    if (!writeCommand(Command::kInit, 0x08, 0x00)) {
        std::fprintf(stderr, "[AHT21B] init command rejected on the bus\n");
        return false;
    }
    ready_ = true;
    return true;
}

bool AHT21B_t::writeCommand(uint8_t c0, uint8_t c1, uint8_t c2) {
    // Apunta el bus I2C de bcm2835 a la dirección del sensor y envía el
    // comando de 3 bytes en una sola transacción.
    bcm2835_i2c_setSlaveAddress(address_);
    const uint8_t cmd[3] = {c0, c1, c2};
    // bcm2835_i2c_write() espera const char* (ver bcm2835.h v1.71).
    return bcm2835_i2c_write(reinterpret_cast<const char*>(cmd), sizeof(cmd)) == 0;
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
    bcm2835_i2c_setSlaveAddress(address_);
    uint8_t raw[kMeasurePacketBytes] = {0};
    if (bcm2835_i2c_read(reinterpret_cast<char*>(raw), sizeof(raw)) != 0) {
        std::fprintf(stderr, "[AHT21B] read failed on the bus\n");
        return false;
    }

    // 3) Validar el CRC del paquete (bits 20-27 del datasheet).
    const uint8_t crc = crc8(raw, sizeof(raw) - 1);
    if (crc != raw[sizeof(raw) - 1]) {
        std::fprintf(stderr, "[AHT21B] CRC mismatch (got 0x%02X, want 0x%02X)\n",
                     crc, raw[sizeof(raw) - 1]);
        return false;
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

    *humidityPct = static_cast<float>(humidity20) / 1048576.0f * 100.0f;
    *temperatureC = static_cast<float>(temp20) / 1048576.0f * 200.0f - 50.0f;
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
