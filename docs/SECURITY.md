# SECURITY — Consideraciones de seguridad

## Superficie de exposición

El proyecto es una aplicación **local** de sensores por I2C: no abre puertos
de red ni acepta entrada externa. El riesgo principal es **físico/eléctrico**
y de **permisos locales**.

## Recomendaciones

1. **Ejecución con root**: `bcm2835_init()` mapea `/dev/mem`; si la app corre
   con `sudo`, hacerlo solo si es necesario (OLED). En producción considerar
   un servicio systemd con usuario restringido (`User=pi`) y capability
   mínima para `/dev/mem`.
2. **Cableado**: alimentar los sensores a 3.3 V (nunca 5 V) y proteger contra
   inversión de polaridad. Los sensores I2C no son tolerantes a 5 V en SDA/SCL.
3. **Configuración**: `config/*.cfg` no contiene secretos; si en el futuro se
   agrega MQTT/BLE (ver BLUETOOTH.md), las credenciales deben ir en variables
   de entorno o un archivo fuera del repo.
4. **Red**: si se expone un servidor REST/MQTT (futuro), limitar el acceso a
   la red local y autenticar.
5. **Repositorio**: nunca commitear tokens ni credenciales (ver LEARNINGS.md
   sobre el manejo de tokens de GitHub).

## Sin dependencias de red en runtime

La app solo usa bcm2835 (local). Las únicas descargas son de
`scripts/install_deps.sh` (bcm2835 desde airspayce.com), que se recomienda
verificar por checksum si se instala en un entorno controlado.
