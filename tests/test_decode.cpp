// Unit tests de decodificación del AHT21B (CRC-8 y valores de 20 bits) y del
// BH1750 (lux) — SIN hardware: solo funciones puras (ver AHT21B_decode.hpp y
// los helpers de BH1750.hpp), sin bcm2835 ni I2C.
//
// Compilar y correr:  make test   (usa el g++ del host)
//
// Los vectores de CRC-8 se verificaron con dos implementaciones independientes
// (bit-loop y tabla precomputada); el valor de "123456789" (0xF7) es el check
// clásico de la variante polinomio 0x31, init 0xFF, sin reflect ni XOR final.

#include <cmath>
#include <cstdint>
#include <cstdio>

#include "drivers/AHT21B_decode.hpp"
#include "drivers/BH1750.hpp"

namespace {

int g_checks = 0;
int g_failures = 0;

void report(const char* expr, int line) {
    ++g_failures;
    std::printf("FAIL %s:%d: %s\n", __FILE__, line, expr);
}

void reportNear(const char* a, const char* b, int line, double got,
                double want) {
    ++g_failures;
    std::printf("FAIL %s:%d: %s ~= %s (got %.5f, want %.5f)\n", __FILE__,
                line, a, b, got, want);
}

}  // namespace

#define CHECK(cond)                                                          \
    do {                                                                     \
        ++g_checks;                                                          \
        if (!(cond)) {                                                       \
            report(#cond, __LINE__);                                         \
        }                                                                    \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                \
    do {                                                                     \
        ++g_checks;                                                          \
        const double got_ = static_cast<double>(a);                          \
        const double want_ = static_cast<double>(b);                         \
        if (std::fabs(got_ - want_) > (eps)) {                               \
            reportNear(#a, #b, __LINE__, got_, want_);                       \
        }                                                                    \
    } while (0)

int main() {
    // ===================== AHT21B: CRC-8 =================================
    {
        const uint8_t check9[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
        CHECK(AHT21B::crc8(check9, sizeof(check9)) == 0xF7);
        CHECK(AHT21B::crc8(check9, 0) == 0xFF);  // vacío = init
        const uint8_t init[] = {0xBE, 0x08, 0x00};
        CHECK(AHT21B::crc8(init, sizeof(init)) == 0x83);
        const uint8_t trigger[] = {0xAC, 0x33, 0x00};
        CHECK(AHT21B::crc8(trigger, sizeof(trigger)) == 0x03);
        const uint8_t status[] = {0x18};
        CHECK(AHT21B::crc8(status, sizeof(status)) == 0x56);
    }

    // ===================== AHT21B: decodificación 20-bit =================
    {
        // Paquete real capturado en la Pi (módulo con CRC no fiable):
        // RH ≈ 77.92 %, T ≈ 19.98 °C.
        const uint8_t real[] = {0x18, 0xC7, 0x76, 0xF5, 0x99, 0x38};
        float tc = 0.0f, rh = 0.0f;
        CHECK(AHT21B::decodeMeasurement(real, &tc, &rh));
        CHECK_NEAR(tc, 19.98, 0.01);
        CHECK_NEAR(rh, 77.92, 0.01);

        // CRC del módulo no coincide con el calculado (0x06): decode no lo
        // valida (comportamiento tolerante de 0.1.8) y acepta si está en rango.
        CHECK(AHT21B::crc8(real, 5) == 0x06);
        CHECK(real[5] != AHT21B::crc8(real, 5));

        // Humedad mínima (0 %RH) con temperatura en rango (t20 = 0x59938 →
        // T ≈ 19.98 °C; el nibble de T vive en el byte 3).
        const uint8_t rhMin[] = {0x18, 0x00, 0x00, 0x05, 0x99, 0x38};
        CHECK(AHT21B::decodeMeasurement(rhMin, &tc, &rh));
        CHECK_NEAR(rh, 0.0, 0.001);
        CHECK_NEAR(tc, 19.98, 0.01);

        // Máximos en rango: RH ≈ 100 % (20-bit máx = 0xFFFFF → 99.9999 %;
        // con 20 bits RH nunca supera 100) y T = 50 °C (t20 = 0x80000, el
        // nibble 0x8 de T va en el byte 3).
        const uint8_t maxIn[] = {0x18, 0xFF, 0xFF, 0xF8, 0x00, 0x00};
        CHECK(AHT21B::decodeMeasurement(maxIn, &tc, &rh));
        CHECK_NEAR(rh, 100.0, 0.001);
        CHECK_NEAR(tc, 50.0, 0.001);

        // Fuera de rango: T = 150 °C (t20 = 0xFFFFF) -> rechazado.
        const uint8_t tooHot[] = {0x18, 0x00, 0x00, 0x0F, 0xFF, 0xFF};
        CHECK(!AHT21B::decodeMeasurement(tooHot, &tc, &rh));

        // Fuera de rango: T = -50 °C (t20 = 0) -> rechazado.
        const uint8_t tooCold[] = {0x18, 0x00, 0x00, 0x00, 0x00, 0x00};
        CHECK(!AHT21B::decodeMeasurement(tooCold, &tc, &rh));

        // Salidas opcionales (nullptr) no deben crashear.
        CHECK(AHT21B::decodeMeasurement(real, nullptr, nullptr));
    }

    // ===================== BH1750: lux = raw / 1.2 =======================
    {
        CHECK_NEAR(BH1750::luxFromRaw(0x0000), 0.0, 0.001);
        CHECK_NEAR(BH1750::luxFromRaw(0x0012), 15.0, 0.001);    // 18 / 1.2
        CHECK_NEAR(BH1750::luxFromRaw(0x0078), 100.0, 0.001);   // 120 / 1.2
        // 65535/1.2 = 54612.5 no es representable en float (da 54612.496),
        // así que la tolerancia es 0.01 (medio lux es irrelevante).
        CHECK_NEAR(BH1750::luxFromRaw(0xFFFF), 54612.5, 0.01);

        const uint8_t lo[] = {0x00, 0x12};
        CHECK_NEAR(BH1750::luxFromBytes(lo), 15.0, 0.001);
        const uint8_t hi[] = {0xFF, 0xFF};
        CHECK_NEAR(BH1750::luxFromBytes(hi), 54612.5, 0.01);
        const uint8_t zero[] = {0x00, 0x00};
        CHECK_NEAR(BH1750::luxFromBytes(zero), 0.0, 0.001);
    }

    std::printf("tests: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
