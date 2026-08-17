#include "engine/Device_t.hpp"

#include <bcm2835.h>
#include <SSD1306_OLED.hpp>

// La librería SSD1306 define un macro `swap(a,b)` que rompe std::swap usado
// por nlohmann::json: se anula aquí (solo este TU; la librería conserva su
// macro en sus propios fuentes) antes de incluir la cabecera JSON.
#undef swap

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <unistd.h>

#include <nlohmann/json.hpp>

namespace Device {

namespace {

// Dimensiones del display SSD1306 usado (128x32, 4 filas de 8 píxeles).
constexpr int kOledWidth = 128;
constexpr int kOledHeight = 32;

// Versión de la app: se inyecta en tiempo de compilación por el Makefile con
// -DVERSION="x.y.z" (leída del archivo VERSION del repo). NUNCA se lee el
// archivo en tiempo de ejecución; el fallback cubre compilaciones manuales.
#ifndef VERSION
#define VERSION "0.0.0"
#endif

// Handler de SIGINT/SIGTERM: solo marca el flag cooperativo para que el
// bucle de run() salga limpio (y el destructor cierre bcm2835).
std::atomic<bool> g_stopRequested{false};
void handleSignal(int) {
    g_stopRequested.store(true);
}

// Formatea un timestamp local "YYYY-MM-DD HH:MM:SS" para los logs.
std::string timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);
    char buf[32] = {0};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}

}  // namespace

Device_t::Device_t(int argc, char* argv[]) : argc_(argc), argv_(argv) {}

Device_t::~Device_t() {
    // Apagar el display (libera el bus) y cerrar bcm2835 si se inicializó.
    if (oled_ != nullptr) {
        oled_->OLEDclearBuffer();
        oled_->OLEDupdate();
        oled_->OLEDPowerDown();
        // SSD1306 tiene funciones virtuales y destructor no virtual; el delete
        // es seguro aquí porque oled_ siempre apunta a un SSD1306 concreto
        // (nunca a un tipo derivado).
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
        delete oled_;
#pragma GCC diagnostic pop
        oled_ = nullptr;
    }
    bcm2835_close();
}

bool Device_t::handleCliFlags() {
    for (int i = 1; i < argc_; ++i) {
        const std::string arg = argv_[i];
        if (arg == "--version" || arg == "-v") {
            // La versión es la macro de compilación (nunca el archivo VERSION).
            std::printf("App v%s\n", VERSION);
            return true;
        }
        if (arg == "--help" || arg == "-h") {
            std::printf(
                "Usage: App [options]\n"
                "Reads temperature/humidity (AHT21B) and light (BH1750) over\n"
                "I2C using bcm2835, with an optional SSD1306 OLED display.\n\n"
                "Options:\n"
                "  --version, -v   Show the application version and exit\n"
                "  --help, -h      Show this help and exit\n\n"
                "The version is baked in at compile time from the VERSION file.\n");
            return true;
        }
    }
    return false;
}

bool Device_t::loadConfig() {
    // Configuración de aplicación: config/config.cfg. Cualquier clave
    // ausente conserva el valor por defecto del miembro.
    {
        std::ifstream file("config/config.cfg");
        if (file.is_open()) {
            try {
                nlohmann::json j;
                file >> j;
                loopDelayMs_ = j.value("loop_delay_ms", loopDelayMs_);
                printIntervalMs_ = j.value("print_interval_ms", printIntervalMs_);
                useOled_ = j.value("use_oled", useOled_);
                useAht21b_ = j.value("use_aht21b", useAht21b_);
                useBh1750_ = j.value("use_bh1750", useBh1750_);
                aht21bAddr_ = j.value("aht21b_address", aht21bAddr_);
                bh1750Addr_ = j.value("bh1750_address", bh1750Addr_);
                oledAddr_ = j.value("oled_address", oledAddr_);
                // Aplicar las direcciones a los drivers antes de begin().
                aht21b_.setAddress(aht21bAddr_);
                bh1750_.setAddress(bh1750Addr_);
            } catch (const nlohmann::json::exception& e) {
                std::fprintf(stderr, "[config] config.cfg invalid, using defaults (%s)\n",
                             e.what());
            }
        } else {
            std::fprintf(stderr, "[config] config/config.cfg not found, using defaults\n");
        }
    }
    // Configuración de hardware: config/hardware.cfg (bus y velocidad I2C).
    {
        std::ifstream file("config/hardware.cfg");
        if (file.is_open()) {
            try {
                nlohmann::json j;
                file >> j;
                i2cBaudrateHz_ = j.value("i2c_baudrate_hz", i2cBaudrateHz_);
            } catch (const nlohmann::json::exception& e) {
                std::fprintf(stderr, "[config] hardware.cfg invalid (%s)\n", e.what());
            }
        }
    }
    return true;
}

bool Device_t::initHardware() {
    // bcm2835_init() mapea /dev/mem (requiere root o el usuario en el grupo
    // apropiado); si falla, la app avisa y sigue con solo consola.
    if (!bcm2835_init()) {
        std::fprintf(stderr, "[hw] bcm2835_init() failed; running without hardware\n");
        return false;
    }

    // IMPORTANTE (bcm2835 v1.71): sin root, init() "tiene exito" pero solo
    // mapea /dev/gpiomem (exclusivamente GPIO). El puntero de I2C (bsc1)
    // queda en MAP_FAILED y cualquier acceso I2C (set_baudrate, writes) escribe
    // sobre una direccion invalida -> SIGSEGV. Los sensores y el OLED necesitan
    // I2C real (/dev/mem), asi que sin root se continua en modo consola.
    if (geteuid() != 0) {
        std::fprintf(stderr,
                     "[hw] bcm2835 I2C (sensores/OLED) requiere root: ejecutar "
                     "con 'sudo ./bin/App'; sin root la app corre en modo consola\n");
        return false;
    }

    // Bus I2C del kernel (i2c-1 por defecto en las Pi modernas).
    bcm2835_i2c_begin();
    bcm2835_i2c_set_baudrate(i2cBaudrateHz_);

    // Sensores: cada driver apunta el bus a su propia dirección antes de cada
    // transacción, así que pueden compartir el mismo periférico I2C.
    if (useAht21b_) {
        aht21b_.begin();
    }
    if (useBh1750_) {
        bh1750_.begin();
    }

    // Display OLED opcional: si no responde (falta hardware o root), la app
    // continúa en modo consola. Se usa la clase SSD1306 de Gavin Lyons.
    if (useOled_) {
        oled_ = new SSD1306(kOledWidth, kOledHeight);
        oled_->OLEDbegin(BCM2835_I2C_CLOCK_DIVIDER_626, oledAddr_);
        oled_->OLEDclearBuffer();
        oled_->setFontNum(OLEDFontType_Default);
        oled_->setTextSize(1);
        oled_->setTextColor(WHITE);
    }
    return true;
}

void Device_t::printReadings() {
    std::printf("[%s] loop=%lu", timestamp().c_str(), loopCount_);
    if (aht21bOk_) {
        std::printf(" | Temp %.2f C | RH %.1f %%", temperatureC_, humidityPct_);
    } else {
        std::printf(" | AHT21B: no data");
    }
    if (bh1750Ok_) {
        std::printf(" | Lux %.1f", lux_);
    } else {
        std::printf(" | BH1750: no data");
    }
    std::printf("\n");
    std::fflush(stdout);
}

void Device_t::updateDisplay() {
    if (oled_ == nullptr) {
        return;
    }
    // Pantalla 128x32: 4 filas de 8 píxeles.
    oled_->OLEDclearBuffer();
    oled_->setCursor(0, 0);
    oled_->print("AHT21B_bh1750 v");
    oled_->print(VERSION);

    oled_->setCursor(0, 8);
    if (aht21bOk_) {
        oled_->print("T:");
        oled_->print(temperatureC_, 1);
        oled_->print("C H:");
        oled_->print(humidityPct_, 1);
        oled_->print("%");
    } else {
        oled_->print("AHT21B: NO DATA");
    }

    oled_->setCursor(0, 16);
    if (bh1750Ok_) {
        oled_->print("Lux:");
        oled_->print(lux_, 1);
    } else {
        oled_->print("BH1750: NO DATA");
    }

    oled_->setCursor(0, 24);
    oled_->print("loop ");
    oled_->print(loopCount_);

    oled_->OLEDupdate();
}

void Device_t::run() {
    // 1) Flags de CLI: --version/--help terminan antes de tocar el hardware.
    if (handleCliFlags()) {
        return;
    }

    // 2) Banner con la versión compilada (macro VERSION, no el archivo).
    std::printf("AHT21B_bh1750 v%s - AHT21B temp/humidity + BH1750 light (I2C)\n",
                VERSION);

    // 3) Ctrl+C limpio: el handler solo marca un flag atómico.
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // 4) Configuración y hardware (tolerante a fallos).
    loadConfig();
    initHardware();

    // 5) Bucle principal: lee los sensores, imprime cada printIntervalMs y
    //    refresca el OLED, hasta Ctrl+C.
    while (!g_stopRequested.load()) {
        if (useAht21b_) {
            aht21bOk_ = aht21b_.read(&temperatureC_, &humidityPct_);
        }
        if (useBh1750_) {
            bh1750Ok_ = bh1750_.read(&lux_);
        }

        if (loopCount_ == 0 || loopCount_ % (printIntervalMs_ / loopDelayMs_) == 0) {
            printReadings();
            updateDisplay();
        }

        ++loopCount_;
        bcm2835_delay(static_cast<uint32_t>(loopDelayMs_));
    }

    std::printf("\nBye (App v%s).\n", VERSION);
}

}  // namespace Device
