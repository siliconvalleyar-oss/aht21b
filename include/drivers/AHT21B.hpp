#pragma once

#include <cstddef>  // size_t
#include <cstdint>

// Driver para el sensor de temperatura y humedad AOSONG AHT21B, leído por I2C
// con la librería bcm2835. El AHT21B entrega medidas de 20 bits (temperatura
// y humedad) en paquetes de 6 bytes con CRC8 opcional (polinomio 0x31).
//
// Conexión típica (bus I2C-1 de la Raspberry Pi):
//   AHT21B VIN  -> 3.3 V      AHT21B GND -> GND
//   AHT21B SDA  -> GPIO 2     AHT21B SCL -> GPIO 3
//
// El chip responde en la dirección 0x38 (56 decimal) por defecto.
namespace AHT21B {

// Dirección I2C por defecto del AHT21B (0x38).
inline constexpr uint8_t kDefaultAddress = 0x38;

// Comandos del sensor (según el datasheet AHT21B).
namespace Command {
inline constexpr uint8_t kInit = 0xBE;        // inicialización / calibración
inline constexpr uint8_t kTrigger = 0xAC;     // dispara una medición
}  // namespace Command

// RAII wrapper sobre el AHT21B. begin() debe llamarse una vez tras
// bcm2835_i2c_begin(); read() devuelve la temperatura en °C y la humedad
// relativa en % (0-100). Todo el acceso al bus se hace con bcm2835_i2c_*.
class AHT21B_t {
public:
    // Construye el driver. `busAddress` permite usar la dirección alternativa
    // si el módulo fue reconfigurado (la mayoría usa 0x38).
    explicit AHT21B_t(uint8_t busAddress = kDefaultAddress);

    // Cambia la dirección I2C (útil cuando la configuración del proyecto
    // especifica una dirección distinta de la default). Debe llamarse antes
    // de begin().
    void setAddress(uint8_t busAddress) { address_ = busAddress; }

    // Inicializa y calibra el sensor (comando 0xBE). Devuelve true si el
    // comando se aceptó en el bus.
    bool begin();

    // Realiza una medición: dispara (0xAC), espera el tiempo de conversión y
    // lee 6 bytes (estado + humedad 20-bit + temperatura 20-bit + CRC).
    // Devuelve true y rellena *temperatureC (grados Celsius) y *humidityPct
    // (0-100 %) si la lectura fue válida; false en caso de error de bus o
    // CRC inválido.
    bool read(float* temperatureC, float* humidityPct);

    // true tras un begin() exitoso.
    bool isReady() const { return ready_; }

private:
    // Envía un comando de 3 bytes por I2C al sensor.
    bool writeCommand(uint8_t c0, uint8_t c1, uint8_t c2);

    // CRC8 (polinomio 0x31, init 0xFF) sobre `len` bytes; usado para validar
    // el paquete de 6 bytes de la medición.
    static uint8_t crc8(const uint8_t* data, size_t len);

    uint8_t address_;  // dirección I2C del sensor
    bool ready_ = false;
};

}  // namespace AHT21B
