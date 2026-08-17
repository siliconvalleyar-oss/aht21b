#pragma once

#include <cstddef>  // size_t
#include <cstdint>

// Decodificación pura del AHT21B (CRC-8 y conversión de los valores de 20
// bits) SIN acceso al bus I2C. Funciones inline en el header para que los
// unit tests (tests/test_decode.cpp) puedan usarlas sin enlazar bcm2835 ni
// ningún driver de hardware.
namespace AHT21B {

// Rango físico de los valores medidos; se usa para validar paquetes cuando
// el CRC del módulo no es fiable (algunos AHT21B no lo calculan bien).
inline constexpr float kTempMinC = -40.0f;
inline constexpr float kTempMaxC = 85.0f;
inline constexpr float kRhMinPct = 0.0f;
inline constexpr float kRhMaxPct = 100.0f;

// CRC-8 del AHT21B: polinomio 0x31 (x^8 + x^5 + x^4 + 1), valor inicial
// 0xFF, sin reflect ni XOR final (según datasheet). Se calcula sobre los 5
// primeros bytes del paquete de medición y se compara con el 6º.
inline uint8_t crc8(const uint8_t* data, size_t len) {
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

// Decodifica un paquete de medición de 6 bytes (status + humedad 20-bit +
// temperatura 20-bit + CRC) a unidades físicas:
//   RH% = (humedad20 / 2^20) * 100
//   T°C = (temperatura20 / 2^20) * 200 - 50
//
// Devuelve false (sin tocar las salidas) si los valores quedan fuera del
// rango físico. NO valida el CRC: quien llame puede comparar el 6º byte con
// crc8(packet, 5) cuando el módulo lo implemente bien.
inline bool decodeMeasurement(const uint8_t packet[6], float* temperatureC,
                              float* humidityPct) {
    const uint32_t humidity20 = (static_cast<uint32_t>(packet[1]) << 12) |
                                (static_cast<uint32_t>(packet[2]) << 4) |
                                (static_cast<uint32_t>(packet[3]) >> 4);
    const uint32_t temp20 = (static_cast<uint32_t>(packet[3] & 0x0F) << 16) |
                            (static_cast<uint32_t>(packet[4]) << 8) |
                            static_cast<uint32_t>(packet[5]);

    const float rh = static_cast<float>(humidity20) / 1048576.0f * 100.0f;
    const float tc = static_cast<float>(temp20) / 1048576.0f * 200.0f - 50.0f;

    if (rh < kRhMinPct || rh > kRhMaxPct || tc < kTempMinC || tc > kTempMaxC) {
        return false;
    }
    if (temperatureC != nullptr) {
        *temperatureC = tc;
    }
    if (humidityPct != nullptr) {
        *humidityPct = rh;
    }
    return true;
}

}  // namespace AHT21B
