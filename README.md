# Ceiling Lamp (Deckenlampe)

ESP32-based WiFi-enabled ceiling lamp controller with WPS support.

## Overview

This project provides WiFi connectivity for an ESP32-powered ceiling lamp. It automatically connects to a known WiFi network or enters WPS (WiFi Protected Setup) pairing mode for easy configuration.

## Features

- **Automatic WiFi Connection**: Attempts to connect to previously configured network
- **WPS Support**: Easy pairing via WPS push-button if initial connection fails
- **Unique Hostname**: Each device gets a unique hostname with the last 4 digits of its MAC address (e.g., `CeilingLamp_A1B2`)
- **Auto-Reconnection**: Automatically reconnects if connection is lost
- **Event-Driven**: Uses WiFi event callbacks for efficient connection management

## Hardware Requirements

- XIAO ESP32S3 board (or compatible ESP32 board)
- USB cable for programming and power
- WiFi router with WPS support (optional, for easy pairing)

## Installation

1. Install the Arduino IDE or PlatformIO
2. Install ESP32 board support
3. Open `Deckenlampe.ino` in your IDE
4. Select the correct board (XIAO ESP32S3 or your ESP32 variant)
5. Upload the sketch to your board

## Usage

### First Time Setup

1. Power on the device
2. Open Serial Monitor at 115200 baud
3. The device will attempt to connect to a known network
4. If no network is configured or connection fails:
   - Press the WPS button on your WiFi router
   - The device will automatically pair and save credentials

### Normal Operation

- Device automatically connects to the saved WiFi network on power-up
- Connection status is printed to Serial Monitor
- Unique hostname includes MAC address for easy device identification

## Configuration

Key parameters can be adjusted in the sketch:

- `WIFI_CONNECTION_TIMEOUT_MS`: Time to wait for WiFi connection (default: 6000ms)
- `ESP_DEVICE_NAME`: Base name for the device (default: "CeilingLamp")
- `SERIAL_BAUD_RATE`: Serial communication speed (default: 115200)

## Serial Output

The device provides detailed status information via Serial:
- Connection attempts and status
- Unique hostname (e.g., "CeilingLamp_A1B2")
- IP address when connected
- WPS pairing status and PIN (if applicable)

## Troubleshooting

**Device won't connect:**
- Ensure WiFi credentials are saved (use WPS first time)
- Check that router is powered on and in range
- Verify correct board settings in Arduino IDE

**WPS pairing fails:**
- Ensure WPS is enabled on your router
- Press WPS button on router when device prompts
- Try power cycling both devices

## License

Open source - free to use and modify.

## Author

icebear74
