#pragma once

#include <cstdint>
#include <string>

#include "drivers/AHT21B.hpp"
#include "drivers/BH1750.hpp"

// Clase SSD1306 declarada por adelantado para no arrastrar las cabeceras de
// bcm2835/SSD1306 a los usuarios de esta cabecera (se incluyen en el .cpp).
class SSD1306;

// Motor principal de la aplicación: inicializa el hardware (bcm2835, sensores
// AHT21B y BH1750 por I2C y el display OLED opcional), carga la configuración
// desde config/*.cfg y ejecuta el bucle de lectura/impresión.
namespace Device {

class Device_t {
public:
    // Crea el motor. `argc`/`argv` se pasan desde main() para soportar
    // --version y --help sin tocar la lógica del bucle.
    explicit Device_t(int argc, char* argv[]);
    ~Device_t();

    // Prohibida la copia: el motor posee recursos de hardware (bcm2835).
    Device_t(const Device_t&) = delete;
    Device_t& operator=(const Device_t&) = delete;

    // Método principal pedido por el prompt: comprueba --version/--help,
    // muestra el banner con la versión de compilación, inicializa el hardware
    // y ejecuta el bucle de lectura de sensores hasta Ctrl+C.
    void run();

private:
    // Procesa --version/--help de argv_. Devuelve true si hay que terminar.
    bool handleCliFlags();

    // Carga config/config.cfg y config/hardware.cfg (JSON); si faltan o son
    // inválidos usa los valores por defecto y avisa por stderr.
    bool loadConfig();

    // bcm2835_init + I2C + sensores + OLED. Tolerante a fallos: un sensor o
    // el display ausente no impiden arrancar la app.
    bool initHardware();

    // Imprime las últimas lecturas en consola con timestamp.
    void printReadings();

    // Vuelca las últimas lecturas al OLED (si está disponible).
    void updateDisplay();

    // Argumentos de la línea de comandos (se copian para no depender de la
    // vida de argv_ de main()).
    int argc_ = 0;
    char** argv_ = nullptr;

    // ---- Configuración (valores por defecto; se sobreescriben con .cfg) ----
    int loopDelayMs_ = 1000;        // pausa entre iteraciones del bucle
    int printIntervalMs_ = 2000;    // cada cuánto imprimir/refrescar
    bool useOled_ = true;           // display SSD1306 opcional
    bool useAht21b_ = true;         // sensor de temperatura/humedad
    bool useBh1750_ = true;         // sensor de luz
    uint8_t aht21bAddr_ = AHT21B::kDefaultAddress;
    uint8_t bh1750Addr_ = BH1750::kDefaultAddress;
    uint8_t oledAddr_ = 0x3C;
    uint32_t i2cBaudrateHz_ = 100000;

    // ---- Drivers (construidos con la dirección por defecto; la dirección
    //      efectiva se aplica con setAddress() al cargar la configuración) ----
    AHT21B::AHT21B_t aht21b_;
    BH1750::BH1750_t bh1750_;

    // ---- Estado de las últimas lecturas ----
    float temperatureC_ = 0.0f;
    float humidityPct_ = 0.0f;
    float lux_ = 0.0f;
    bool aht21bOk_ = false;
    bool bh1750Ok_ = false;

    // Puntero opaco al display (nullptr si no hay OLED o falló su init).
    SSD1306* oled_ = nullptr;

    unsigned long loopCount_ = 0;
};

}  // namespace Device
