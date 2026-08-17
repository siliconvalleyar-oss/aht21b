// main.cpp - punto de entrada de la aplicación AHT21B_bh1750.
//
// Sigue el formato pedido en el prompt: la lógica vive en Device::Device_t
// y aquí solo se construye con std::make_unique (memoria gestionada
// automáticamente, sin new/delete explícitos) y se ejecuta run().
//
// La única adaptación sobre la plantilla original es pasar argc/argv: el
// motor necesita los argumentos para soportar --version/--help sin cambiar
// la lógica del bucle principal.

#include <memory>

#include "engine/Device_t.hpp"

int main(int argc, char* argv[]) {
    // Construye el motor con gestión automática de memoria; al salir de main
    // el destructor de Device_t cierra bcm2835 y apaga el OLED.
    auto device = std::make_unique<Device::Device_t>(argc, argv);
    device->run();
    return 0;
}
