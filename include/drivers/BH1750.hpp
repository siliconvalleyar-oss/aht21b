#pragma once

#include <cstdint>

// Driver para el sensor de luz ambiente ROHM BH1750, leído por I2C con la
// librería bcm2835. El BH1750 entrega la iluminancia en lux con resolución de
// 1 lx (modo H-resolución continua, ~120 ms por muestra).
//
// Conexión típica (bus I2C-1 de la Raspberry Pi):
//   BH1750 VCC -> 3.3 V       BH1750 GND -> GND
//   BH1750 SDA -> GPIO 2      BH1750 SCL -> GPIO 3
//   (ADDR a GND => dirección 0x23; ADDR a 3.3 V => 0x5C)
//
// El chip responde en la dirección 0x23 (35 decimal) por defecto.
namespace BH1750 {

// Dirección I2C por defecto del BH1750 (0x23; 0x5C con el pin ADDR alto).
inline constexpr uint8_t kDefaultAddress = 0x23;

// Comandos del sensor (según el datasheet BH1750FVI).
namespace Command {
inline constexpr uint8_t kPowerDown = 0x00;        // modo bajo consumo
inline constexpr uint8_t kPowerOn = 0x01;          // despierta el sensor
inline constexpr uint8_t kReset = 0x07;            // reinicia el registro
inline constexpr uint8_t kContHResMode = 0x10;     // medición continua 1 lx
}  // namespace Command

// RAII wrapper sobre el BH1750. begin() lo enciende en modo continua de alta
// resolución; read() devuelve la iluminancia en lux. Todo el acceso al bus
// se hace con bcm2835_i2c_*.
class BH1750_t {
public:
    // Construye el driver. `busAddress` permite elegir la dirección según el
    // estado del pin ADDR (0x23 o 0x5C).
    explicit BH1750_t(uint8_t busAddress = kDefaultAddress);

    // Cambia la dirección I2C (útil cuando la configuración del proyecto
    // especifica una dirección distinta de la default). Debe llamarse antes
    // de begin().
    void setAddress(uint8_t busAddress) { address_ = busAddress; }

    // Enciende el sensor (0x01) y lo pone en modo de medición continua de
    // alta resolución (0x10). Devuelve true si el bus aceptó los comandos.
    bool begin();

    // Lee la iluminancia actual en lux (2 bytes). Devuelve true y rellena
    // *lux si la lectura fue válida.
    bool read(float* lux);

    // Vuelve al modo de bajo consumo (0x00). Opcional; el sensor se apaga
    // solo al cortar la alimentación.
    bool powerDown();

    // true tras un begin() exitoso.
    bool isReady() const { return ready_; }

private:
    // Envía un comando de 1 byte por I2C al sensor.
    bool writeCommand(uint8_t cmd);

    uint8_t address_;  // dirección I2C del sensor
    bool ready_ = false;
};

}  // namespace BH1750
