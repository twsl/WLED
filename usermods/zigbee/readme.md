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
2. Add `zigbee` to the `custom_usermods` list in your platformio environment
3. Flash the firmware to your ESP32-C6 board
4. The device will automatically join your Zigbee network when powered on

## Configuration

The usermod can be configured through the WLED web interface under "Usermod Settings".

### Settings

- **Enabled**: Enable/disable Zigbee functionality
- **Device Name**: Name for the Zigbee device (default: "WLED")
- **Endpoint ID**: Zigbee endpoint identifier (default: 1)

## Usage

Once joined to a Zigbee network, the WLED device will appear as a dimmable light in your Zigbee coordinator. You can then control:

- On/Off state
- Brightness (0-255)
- Color (RGB)
- Effects (via custom attributes)

## Dependencies

This usermod requires the ESP-Zigbee-SDK which is automatically included when building for ESP32-C6.

## Notes

- This usermod is only compatible with ESP32-C6 due to hardware requirements
- The device must be within range of your Zigbee coordinator
- Initial pairing may require putting the coordinator in pairing mode