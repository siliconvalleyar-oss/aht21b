#ifndef DRIVERS_I2C_BUS_HPP
#define DRIVERS_I2C_BUS_HPP

#include <cstdint>

// Transacciones I2C con timeout acotado sobre el controlador BSC1 de
// bcm2835. bcm2835_i2c_write()/bcm2835_i2c_read() de la librería esperan el
// bit S_DONE en un bucle SIN timeout: si un esclavo se cae del bus a mitad de
// una transacción (contacto intermitente, por ejemplo), la app se cuelga para
// siempre y ni SIGTERM la detiene. Estas funciones replican la misma lógica
// pero con un plazo máximo; ante un exceso de tiempo o un error (NACK,
// clock-stretch) devuelven false y dejan el controlador en un estado sano.
namespace I2C {

// Envía `len` bytes a `addr`. Timeout por defecto: 100 ms (una transacción
// normal tarda microsegundos; 100 ms solo se alcanza con el bus trabado).
bool write(uint8_t addr, const uint8_t* data, uint32_t len,
           uint32_t timeoutUs = 100000);

// Lee `len` bytes desde `addr`. Mismo contrato que write().
bool read(uint8_t addr, uint8_t* data, uint32_t len,
          uint32_t timeoutUs = 100000);

}  // namespace I2C

#endif  // DRIVERS_I2C_BUS_HPP
