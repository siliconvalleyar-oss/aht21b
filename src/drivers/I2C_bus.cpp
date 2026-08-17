#include "drivers/I2C_bus.hpp"

#include <bcm2835.h>

#include <chrono>
#include <cstdio>

namespace I2C {

namespace {

// Puntero a los registros del controlador BSC1 (el periférico I2C por
// defecto en las Pi modernas). bcm2835 expone la base mapeada en
// bcm2835_peripherals; los offsets de bcm2835.h son desplazamientos de BYTE
// y el puntero es uint32_t*, así que se dividen por 4 (igual que hace la
// propia librería con bcm2835_bsc1).
volatile uint32_t* bsc() {
    return bcm2835_peripherals + BCM2835_BSC1_BASE / 4;
}

uint64_t nowUs() {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
}

// Deja el controlador en un estado limpio tras un error o timeout: vacía el
// FIFO y borra los flags de estado para que la siguiente transacción arranque
// de cero (el bit I2CEN queda activo, así que no hace falta volver a
// inicializar el periférico).
void recover(volatile uint32_t* control, volatile uint32_t* status) {
    bcm2835_peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);
    bcm2835_peri_write_nb(status,
                          BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR |
                          BCM2835_BSC_S_DONE);
}

}  // namespace

bool write(uint8_t addr, const uint8_t* data, uint32_t len,
           uint32_t timeoutUs) {
    volatile uint32_t* control = bsc() + BCM2835_BSC_C / 4;
    volatile uint32_t* status  = bsc() + BCM2835_BSC_S / 4;
    volatile uint32_t* dlen    = bsc() + BCM2835_BSC_DLEN / 4;
    volatile uint32_t* fifo    = bsc() + BCM2835_BSC_FIFO / 4;

    bcm2835_i2c_setSlaveAddress(addr);

    bcm2835_peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);
    bcm2835_peri_write_nb(status,
                          BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR |
                          BCM2835_BSC_S_DONE);
    bcm2835_peri_write_nb(dlen, len);

    uint32_t remaining = len;
    uint32_t i = 0;
    while (remaining && i < BCM2835_BSC_FIFO_SIZE) {
        bcm2835_peri_write_nb(fifo, data[i]);
        ++i;
        --remaining;
    }
    bcm2835_peri_write_nb(control, BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

    const uint64_t deadline = nowUs() + timeoutUs;
    while (!(bcm2835_peri_read(status) & BCM2835_BSC_S_DONE)) {
        while (remaining && (bcm2835_peri_read(status) & BCM2835_BSC_S_TXD)) {
            bcm2835_peri_write(fifo, data[i]);
            ++i;
            --remaining;
        }
        if (nowUs() > deadline) {
            std::fprintf(stderr,
                         "[i2c] write timeout (addr 0x%02X, %u bytes)\n",
                         addr, len);
            recover(control, status);
            return false;
        }
    }

    const uint32_t st = bcm2835_peri_read(status);
    if (st & BCM2835_BSC_S_ERR) {
        std::fprintf(stderr, "[i2c] write NACK (addr 0x%02X)\n", addr);
        recover(control, status);
        return false;
    }
    if (st & BCM2835_BSC_S_CLKT) {
        std::fprintf(stderr,
                     "[i2c] write clock-stretch timeout (addr 0x%02X)\n", addr);
        recover(control, status);
        return false;
    }
    if (remaining) {
        std::fprintf(stderr, "[i2c] write incomplete (addr 0x%02X)\n", addr);
        recover(control, status);
        return false;
    }
    bcm2835_peri_set_bits(control, BCM2835_BSC_S_DONE, BCM2835_BSC_S_DONE);
    return true;
}

bool read(uint8_t addr, uint8_t* data, uint32_t len, uint32_t timeoutUs) {
    volatile uint32_t* control = bsc() + BCM2835_BSC_C / 4;
    volatile uint32_t* status  = bsc() + BCM2835_BSC_S / 4;
    volatile uint32_t* dlen    = bsc() + BCM2835_BSC_DLEN / 4;
    volatile uint32_t* fifo    = bsc() + BCM2835_BSC_FIFO / 4;

    bcm2835_i2c_setSlaveAddress(addr);

    bcm2835_peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);
    bcm2835_peri_write_nb(status,
                          BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR |
                          BCM2835_BSC_S_DONE);
    bcm2835_peri_write_nb(dlen, len);
    bcm2835_peri_write_nb(control,
                          BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST |
                          BCM2835_BSC_C_READ);

    uint32_t remaining = len;
    uint32_t i = 0;
    const uint64_t deadline = nowUs() + timeoutUs;
    while (!(bcm2835_peri_read_nb(status) & BCM2835_BSC_S_DONE)) {
        while (remaining && (bcm2835_peri_read_nb(status) & BCM2835_BSC_S_RXD)) {
            data[i] = static_cast<uint8_t>(bcm2835_peri_read_nb(fifo));
            ++i;
            --remaining;
        }
        if (nowUs() > deadline) {
            std::fprintf(stderr,
                         "[i2c] read timeout (addr 0x%02X, %u bytes)\n",
                         addr, len);
            recover(control, status);
            return false;
        }
    }
    while (remaining && (bcm2835_peri_read_nb(status) & BCM2835_BSC_S_RXD)) {
        data[i] = static_cast<uint8_t>(bcm2835_peri_read_nb(fifo));
        ++i;
        --remaining;
    }

    const uint32_t st = bcm2835_peri_read(status);
    if (st & BCM2835_BSC_S_ERR) {
        std::fprintf(stderr, "[i2c] read NACK (addr 0x%02X)\n", addr);
        recover(control, status);
        return false;
    }
    if (st & BCM2835_BSC_S_CLKT) {
        std::fprintf(stderr,
                     "[i2c] read clock-stretch timeout (addr 0x%02X)\n", addr);
        recover(control, status);
        return false;
    }
    if (remaining) {
        std::fprintf(stderr, "[i2c] read incomplete (addr 0x%02X)\n", addr);
        recover(control, status);
        return false;
    }
    bcm2835_peri_set_bits(control, BCM2835_BSC_S_DONE, BCM2835_BSC_S_DONE);
    return true;
}

}  // namespace I2C
