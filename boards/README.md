# Custom Board Definitions

This directory contains custom board definitions for boards that are not yet fully supported by PlatformIO or require special configuration for WLED.

## ESP32-C6 Support

### Current Status: **Experimental / In Progress**

The ESP32-C6 is a next-generation chip from Espressif featuring:
- **RISC-V 32-bit processor** @ 160 MHz
- **WiFi 6** (802.11ax) support
- **Bluetooth 5.0** (LE)
- **IEEE 802.15.4** radio for **Zigbee** and **Thread**
- 320 KB SRAM, up to 16 MB Flash
- USB Serial/JTAG controller

### Why is ESP32-C6 Experimental?

ESP32-C6 support in WLED is currently experimental because:

1. **Arduino Framework Limitation**: ESP32-C6 requires arduino-esp32 3.x (based on ESP-IDF 5.x), which is still being integrated into PlatformIO. The standard espressif32 platform doesn't yet include stable Arduino support for C6.

2. **Build Environment**: While the hardware is capable, the software ecosystem is still maturing. PlatformIO's espressif32 platform 6.x includes ESP-IDF 5.x support but Arduino framework for C6 is not yet complete.

### How to Use ESP32-C6

To build WLED for ESP32-C6:

```bash
# Build for ESP32-C6 (experimental)
pio run -e esp32c6dev
```

**Note**: This build may fail until arduino-esp32 3.x is fully integrated into the espressif32 platform. Monitor these resources for updates:
- [PlatformIO ESP32 Platform](https://github.com/platformio/platform-espressif32)
- [Arduino-ESP32 3.x releases](https://github.com/espressif/arduino-esp32/releases)

### Zigbee Support

The ESP32-C6 includes hardware support for Zigbee (IEEE 802.15.4). To enable Zigbee features:

1. **Use ESP-IDF Framework**: Zigbee SDK is natively supported in ESP-IDF 5.x
2. **Enable in Configuration**: Uncomment Zigbee flags in `platformio.ini`
3. **Add Zigbee Libraries**: The esp-zigbee-sdk is part of ESP-IDF 5.x

Example configuration for Zigbee (in `platformio_override.ini`):

```ini
[env:esp32c6dev_zigbee]
extends = esp32c6dev
framework = espidf  ; Use ESP-IDF for full Zigbee support
build_flags = ${env:esp32c6dev.build_flags}
  -D CONFIG_ZB_ENABLED=1
  -D CONFIG_IEEE802154_ENABLED=1
  -D CONFIG_ZB_RADIO_NATIVE=1
```

### Contributing

If you're working with ESP32-C6 and arduino-esp32 3.x:
1. Test the build environment as arduino-esp32 3.x matures
2. Report issues specific to C6 support
3. Update documentation as the ecosystem stabilizes

### References

- [ESP32-C6 Product Page](https://www.espressif.com/en/products/socs/esp32-c6)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP Zigbee SDK](https://github.com/espressif/esp-zigbee-sdk)
- [Arduino-ESP32 Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
