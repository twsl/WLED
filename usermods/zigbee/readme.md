# Zigbee Usermod

This usermod enables WLED control via Zigbee protocol using the ESP-Zigbee-SDK.

## Features

- Zigbee 3.0 compatibility
- Control WLED brightness, color, and effects via Zigbee
- Works with Zigbee coordinators and home automation systems
- Supports ESP32-C6 (required for Zigbee functionality)

## Hardware Requirements

- ESP32-C6 development board (ESP32-C6 has native IEEE 802.15.4 radio for Zigbee)
- Compatible Zigbee coordinator (like Zigbee2MQTT, ZHA, etc.)

## Installation

1. Ensure you have an ESP32-C6 board
2. Add `zigbee` to the `custom_usermods` list in your PlatformIO environment
3. Flash the firmware to your ESP32-C6 board using the `esp32c6dev` environment
4. The device will automatically join your Zigbee network when powered on

## Building

To build WLED with Zigbee support:

```bash
# Using PlatformIO
pio run -e esp32c6dev

# Or with custom usermods
pio run -e esp32c6dev --project-option "custom_usermods=zigbee"
```

## Configuration

The usermod can be configured through the WLED web interface under "Usermod Settings".

### Settings

- **Enabled**: Enable/disable Zigbee functionality (default: disabled)
- **Device Name**: Name for the Zigbee device (default: "WLED")
- **Endpoint ID**: Zigbee endpoint identifier (default: 1)

## Usage

Once joined to a Zigbee network, the WLED device will appear as a dimmable light in your Zigbee coordinator. You can then control:

- **On/Off state**: Standard Zigbee on/off cluster
- **Brightness (0-255)**: Level control cluster
- **Color (RGB)**: Color control cluster with CIE XY color space
- **Effects**: WLED effects are preserved and can be controlled through the web interface

## Zigbee Clusters Supported

- **Basic Cluster (0x0000)**: Device identification
- **On/Off Cluster (0x0006)**: Power control
- **Level Control Cluster (0x0008)**: Brightness control
- **Color Control Cluster (0x0300)**: Color control using CIE XY

## Dependencies

This usermod requires the ESP-Zigbee-SDK which is included in ESP-IDF 5.x for ESP32-C6. No additional dependencies are required.

## Compatibility

- **ESP32-C6**: Full support (recommended)
- **Other ESP32 variants**: Not supported (IEEE 802.15.4 radio required)

## Notes

- This usermod is only compatible with ESP32-C6 due to hardware requirements
- The device must be within range of your Zigbee coordinator
- Initial pairing may require putting the coordinator in pairing mode
- Color conversion from CIE XY to RGB is simplified - advanced color accuracy may require calibration

## Troubleshooting

### Device not appearing in Zigbee network
1. Check that your coordinator is in pairing mode
2. Verify the ESP32-C6 is powered and running WLED
3. Check serial output for Zigbee initialization messages

### Zigbee commands not working
1. Ensure the usermod is enabled in settings
2. Check that the device has properly joined the network
3. Verify your coordinator supports the required clusters

### Build errors
1. Ensure you're using the `esp32c6dev` environment
2. Check that the ESP-IDF platform supports ESP32-C6
3. Verify network connectivity for downloading dependencies