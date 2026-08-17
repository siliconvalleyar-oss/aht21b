// Ejemplo independiente: lee temperatura y humedad del AHT21B por el bus I2C
// del kernel (/dev/i2c-1). No depende de bcm2835; usa ioctl/write/read.
//
// Compilar:  make        (o: g++ -Wall -Wextra -std=c++11 -o aht21b aht21b.cpp)
// Ejecutar:  sudo ./aht21b   (el acceso a /dev/i2c-* normalmente requiere root)
//
// Secuencia según el datasheet AHT21B:
//   1) inicializar (0xBE 0x08 0x00) y esperar a que el sensor quede listo
//   2) disparar la medición (0xAC 0x33 0x00) y esperar ~80-100 ms
//   3) leer 6 bytes: estado + humedad 20-bit + temperatura 20-bit (+CRC)
// Nota: durante la conversión el sensor estira SCL; el driver del kernel
// aguanta el estiramiento con su propio timeout.

#include <cstdint>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Dirección I2C del AHT21B (por defecto 0x38).
constexpr uint8_t kAddress = 0x38;

// Escribe `len` bytes y devuelve true si el bus los aceptó. Con el driver del
// kernel, un NACK (dispositivo ausente o bus trabado) devuelve -1 con
// errno == EREMOTEIO.
bool writeCommand(int file, const uint8_t* cmd, size_t len) {
    if (write(file, cmd, len) == static_cast<ssize_t>(len)) {
        return true;
    }
    std::cerr << "  write() falló: " << std::strerror(errno);
    if (errno == EREMOTEIO) {
        std::cerr << " (NACK: ¿el sensor está conectado y alimentado?)";
    }
    std::cerr << std::endl;
    return false;
}

int main() {
    const char* bus = "/dev/i2c-1";  // Bus I2C de la Raspberry Pi (revisa si es i2c-1)

    // 1. Abrir el bus I2C
    int file = open(bus, O_RDWR);
    if (file < 0) {
        std::cerr << "Error: No se pudo abrir el bus I2C: " << std::strerror(errno)
                  << ". ¿Está habilitado (raspi-config -> I2C)?" << std::endl;
        return 1;
    }

    // 2. Especificar la dirección del sensor
    if (ioctl(file, I2C_SLAVE, kAddress) < 0) {
        std::cerr << "Error: No se pudo configurar la dirección 0x"
                  << std::hex << kAddress << ": " << std::strerror(errno) << std::endl;
        close(file);
        return 1;
    }

    // 3. Inicializar el sensor (datasheet): comando 0xBE 0x08 0x00 y espera.
    //    Sin esto, un sensor recién alimentado puede no aceptar la medición.
    const uint8_t init_cmd[3] = {0xBE, 0x08, 0x00};
    if (!writeCommand(file, init_cmd, sizeof(init_cmd))) {
        std::cerr << "Error: Falló la inicialización del sensor" << std::endl;
        close(file);
        return 1;
    }
    usleep(100000);  // 100 ms: el sensor queda ocupado calibrando

    // 4. Disparar la medición (0xAC 0x33 0x00). El write puede tardar ~80 ms
    //    porque el sensor estira SCL durante la conversión.
    const uint8_t trigger_cmd[3] = {0xAC, 0x33, 0x00};
    if (!writeCommand(file, trigger_cmd, sizeof(trigger_cmd))) {
        std::cerr << "Error: Falló al enviar el comando de medición" << std::endl;
        close(file);
        return 1;
    }

    // 5. Esperar a que el sensor complete la medición (mínimo 80 ms)
    usleep(100000);

    // 6. Leer los 6 bytes de datos (estado + humedad 20-bit + temperatura 20-bit)
    uint8_t data[6] = {0};
    if (read(file, data, sizeof(data)) != static_cast<ssize_t>(sizeof(data))) {
        std::cerr << "Error: No se pudieron leer los 6 bytes de datos: "
                  << std::strerror(errno) << std::endl;
        close(file);
        return 1;
    }

    // 7. Verificar el estado: el bit 7 (busy) debe estar en 0 para datos válidos
    //    (el byte 0 típico con datos listos es 0x18, calibrado y no ocupado).
    if (data[0] & 0x80) {
        std::cerr << "Error: El sensor sigue ocupado (estado 0x"
                  << std::hex << static_cast<int>(data[0]) << ")" << std::endl;
        close(file);
        return 1;
    }

    // 8. Extraer los valores crudos de 20 bits
    //    Humedad: bytes 1, 2 y los 4 bits superiores del byte 3
    uint32_t raw_humidity =
        (static_cast<uint32_t>(data[1]) << 12) |
        (static_cast<uint32_t>(data[2]) << 4) |
        (data[3] >> 4);
    //    Temperatura: 4 bits inferiores del byte 3, bytes 4 y 5
    uint32_t raw_temperature =
        (static_cast<uint32_t>(data[3] & 0x0F) << 16) |
        (static_cast<uint32_t>(data[4]) << 8) |
        data[5];

    // 9. Convertir a valores físicos
    float humidity = (raw_humidity / 1048576.0f) * 100.0f;    // 2^20 = 1048576
    float temperature = (raw_temperature / 1048576.0f) * 200.0f - 50.0f;

    // 10. Mostrar resultados
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Humedad: " << humidity << " %RH" << std::endl;
    std::cout << "Temperatura: " << temperature << " °C" << std::endl;

    close(file);
    return 0;
}
