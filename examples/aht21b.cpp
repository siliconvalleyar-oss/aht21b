#include <iostream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstdint>

// Dirección I2C del AHT21B
#define AHT21B_ADDRESS 0x38

// Comando para iniciar la medición
uint8_t trigger_cmd[3] = {0xAC, 0x33, 0x00};

int main() {
    int file;
    char *bus = "/dev/i2c-1";  // Bus I2C de la Raspberry Pi (revisa si es i2c-1)

    // 1. Abrir el bus I2C
    if ((file = open(bus, O_RDWR)) < 0) {
        std::cerr << "Error: No se pudo abrir el bus I2C. ¿Está habilitado?" << std::endl;
        return 1;
    }

    // 2. Especificar la dirección del sensor
    if (ioctl(file, I2C_SLAVE, AHT21B_ADDRESS) < 0) {
        std::cerr << "Error: No se pudo comunicar con el sensor en la dirección 0x38" << std::endl;
        close(file);
        return 1;
    }

    // 3. Enviar el comando para iniciar la medición
    if (write(file, trigger_cmd, 3) != 3) {
        std::cerr << "Error: Falló al enviar el comando de medición" << std::endl;
        close(file);
        return 1;
    }

    // 4. Esperar a que el sensor complete la medición (mínimo 80ms, recomendado 100ms)
    usleep(100000); // 100 milisegundos

    // 5. Leer los 6 bytes de datos (estado + humedad + temperatura)
    uint8_t data[6];
    if (read(file, data, 6) != 6) {
        std::cerr << "Error: No se pudieron leer los 6 bytes de datos" << std::endl;
        close(file);
        return 1;
    }

    // 6. Verificar el bit de estado (el primer byte debe ser 0x28 o 0x38 para datos válidos)
    if ((data[0] & 0x80) == 0x80) {
        std::cerr << "Error: El sensor está ocupado o el comando no fue aceptado" << std::endl;
        close(file);
        return 1;
    }

    // 7. Extraer los valores crudos de 20 bits
    // Humedad: bytes 1, 2 y los 4 bits superiores del byte 3
    uint32_t raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    // Temperatura: 4 bits inferiores del byte 3, bytes 4 y 5
    uint32_t raw_temperature = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    // 8. Convertir a valores físicos
    float humidity = (raw_humidity / 1048576.0) * 100.0;    // 2^20 = 1048576
    float temperature = (raw_temperature / 1048576.0) * 200.0 - 50.0;

    // 9. Mostrar resultados
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Humedad: " << humidity << " %RH" << std::endl;
    std::cout << "Temperatura: " << temperature << " °C" << std::endl;

    close(file);
    return 0;
}
