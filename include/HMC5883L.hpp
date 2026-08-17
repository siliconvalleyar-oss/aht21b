#pragma once

#include <cstdint>

// Esqueleto de la clase del magnetómetro Honeywell HMC5883L (bus I2C,
// dirección 0x1E). Este proyecto (AHT21B_bh1750) no usa el magnetómetro
// todavía: la clase queda declarada con su API prevista para que la
// integración futura (brújula / orientación) solo tenga que rellenar la
// implementación.
//
// La interfaz sigue el mismo patrón que los drivers AHT21B_t y BH1750_t:
// begin() inicializa el dispositivo y read() devuelve los tres ejes.
namespace Device {

// Skeleton del magnetómetro de 3 ejes HMC5883L.
class HMC5883L_t {
public:
    // Dirección I2C fija del HMC5883L (0x1E); el chip no tiene pin de
    // selección de dirección.
    static constexpr uint8_t kAddress = 0x1E;

    // Registros principales del HMC5883L (según el datasheet).
    static constexpr uint8_t kRegConfigA = 0x00;    // configuración de medición
    static constexpr uint8_t kRegConfigB = 0x01;    // ganancia del sensor
    static constexpr uint8_t kRegMode = 0x02;       // modo de operación
    static constexpr uint8_t kRegDataX = 0x03;      // datos del eje X (2 bytes)
    static constexpr uint8_t kRegDataZ = 0x05;      // datos del eje Z (2 bytes)
    static constexpr uint8_t kRegDataY = 0x07;      // datos del eje Y (2 bytes)

    // Inicializa el sensor: modo continuo (0x00), medición normal (0x70 en
    // ConfigA) y ganancia por defecto (0x20 en ConfigB).
    // TODO(integracion): implementar con bcm2835_i2c_*.
    bool begin() {
        // bcm2835_i2c_setSlaveAddress(kAddress);
        // bcm2835_i2c_write(...);  // ConfigA/B y modo continuo
        return false;  // pendiente de implementar
    }

    // Lee los tres ejes del campo magnético en cuentas crudas (int16).
    // TODO(integracion): implementar la lectura de 6 bytes y el escalado.
    bool read(int16_t* x, int16_t* y, int16_t* z) {
        (void)x;
        (void)y;
        (void)z;
        return false;  // pendiente de implementar
    }

    // true tras un begin() exitoso.
    bool isReady() const { return ready_; }

private:
    bool ready_ = false;
};

}  // namespace Device
