# Ceiling Lamp (Deckenlampe)

ESP32-based WiFi-enabled ceiling lamp controller with WPS support and Over-The-Air (OTA) updates.

## Overview

This project provides WiFi connectivity for an ESP32-powered ceiling lamp. It automatically connects to the strongest available WiFi network (best RSSI) or enters WPS (WiFi Protected Setup) pairing mode for easy configuration. The firmware includes three different OTA update methods for easy maintenance and upgrades.

## Features

### WiFi & Network
- **Smart WiFi Connection**: Scans for all available access points with your SSID and connects to the strongest signal
- **WPS Support**: Easy pairing via WPS push-button if no credentials are saved
- **Unique Hostname**: Each device gets a unique hostname with the last 4 digits of its MAC address (e.g., `CeilingLamp_A1B2`)
- **Auto-Reconnection**: Automatically reconnects if connection is lost
- **Event-Driven**: Uses WiFi event callbacks for efficient connection management

### Time & Timezone
- **NTP Time Synchronization**: Automatically syncs time from NTP servers with fallback chain:
  1. PTB Germany (`ptbtime1.ptb.de`)
  2. DE Pool NTP (`de.pool.ntp.org`)
  3. Local Gateway (as NTP server)
  4. Google Public NTP (`216.239.35.0`)
- **Timezone Support**: Full timezone and DST (Daylight Saving Time) support
- **Default Timezone**: Berlin, Germany (CET/CEST with automatic DST transitions)
- **Robust Time Conversion**: Custom `GeneralTimeConverter` for reliable timezone calculations

### OTA Updates (Over-The-Air)
The firmware supports three different OTA update methods:

1. **ArduinoOTA** - Update from Arduino IDE
   - Upload new firmware directly from Arduino IDE over WiFi
   - No USB cable required
   - Port: 3232 (default)

2. **Web-Based OTA** - Update via Web Interface
   - Beautiful web interface accessible at `http://[device-ip]/`
   - Upload `.bin` firmware files through your browser
   - Real-time upload progress display
   - Shows device information (hostname, IP, MAC, firmware version)

3. **HTTP OTA** - Automatic Updates from Web Server
   - Configure update server URL in code
   - Periodically checks for new firmware versions
   - Automatic download and installation
   - Perfect for fleet management

### Modular Architecture
- **WiFi_Manager**: Handles WiFi connection, WPS, and NTP synchronization
- **OTA_Update**: Manages all three OTA update methods
- **GeneralTimeConverter**: Robust timezone and DST handling
- **Version**: Firmware version tracking with git commit hash
- **Clean main .ino**: Minimal main file, all functionality in modules

## Hardware Requirements

- XIAO ESP32S3 board (or compatible ESP32 board)
- USB cable for initial programming and power
- WiFi router (WPS support optional, for easy first-time pairing)

## Installation

### Required Libraries

Install these libraries via Arduino Library Manager:
- `WiFi` (included with ESP32 core)
- `ArduinoOTA`
- `WebServer` (included with ESP32 core)
- `HTTPUpdate` (included with ESP32 core)
- `NTPClient` by Fabrice Weinberg

### Upload Firmware

1. Install the Arduino IDE or PlatformIO
2. Install ESP32 board support (via Board Manager)
3. Install required libraries (see above)
4. Open `Deckenlampe/Deckenlampe.ino` in your IDE
5. Select the correct board (XIAO ESP32S3 or your ESP32 variant)
6. Select the correct COM port
7. Upload the sketch to your board

## Usage

### First Time Setup

1. Power on the device
2. Open Serial Monitor at 115200 baud
3. The device will scan for saved WiFi networks
4. If no network is configured or none found:
   - Press the WPS button on your WiFi router
   - The device will automatically pair and save credentials
5. Once connected, time will be automatically synchronized

### Normal Operation

- Device scans for your SSID and connects to the strongest signal automatically
- Time is synchronized via NTP on every boot
- OTA services start automatically after WiFi connection
- Connection status and time info are printed to Serial Monitor

### Accessing the Web Interface

1. Connect to the same WiFi network
2. Find the device IP address (shown in Serial Monitor)
3. Open a web browser and navigate to `http://[device-ip]/`
4. You'll see a web page with:
   - Current firmware version
   - Device information (hostname, IP, MAC)
   - Firmware upload form

### Updating Firmware

#### Method 1: Arduino IDE (ArduinoOTA)
1. Ensure device is connected to WiFi
2. In Arduino IDE, go to Tools → Port
3. Select your device's network port (e.g., `CeilingLamp_A1B2 at 192.168.1.100`)
4. Upload sketch as usual

#### Method 2: Web Interface
1. Build your firmware as a `.bin` file:
   - In Arduino IDE: Sketch → Export Compiled Binary
2. Navigate to `http://[device-ip]/`
3. Click "Choose File" and select your `.bin` file
4. Click "Upload & Update"
5. Wait for upload to complete
6. Device will reboot automatically

#### Method 3: HTTP Server (Automatic)
1. Configure `UPDATE_SERVER_URL` in `OTA_Update.cpp`
2. Place your firmware `.bin` file on the web server
3. Device checks for updates every hour (configurable)
4. Automatic download and installation when new version is found

## Configuration

### WiFi Settings (WiFi_Manager.h)
- `WIFI_CONNECTION_TIMEOUT_MS`: Connection timeout (default: 20000ms)
- `ESP_DEVICE_NAME`: Base hostname (default: "CeilingLamp")

### NTP Settings (WiFi_Manager.h)
- `DEFAULT_NTP_SERVER_PRIMARY`: Primary NTP server
- `DEFAULT_NTP_SERVER_SECONDARY`: Secondary NTP server
- `DEFAULT_NTP_UPDATE_INTERVAL_MIN`: Update interval in minutes (default: 60)
- `DEFAULT_TIMEZONE`: Timezone string (default: Berlin "CET-1CEST,M3.5.0,M10.5.0/3")

### OTA Settings (OTA_Update.cpp)
- `OTA_PORT`: ArduinoOTA port (default: 3232)
- `HTTP_UPDATE_CHECK_INTERVAL_MS`: Auto-update check interval (default: 3600000ms = 1 hour)
- `UPDATE_SERVER_URL`: Your firmware update server URL
- `UPDATE_VERSION_URL`: Version check URL

### Firmware Version
Edit `Deckenlampe/Version.h` to update version number:
```cpp
#define DECKENLAMPE_VERSION "1.0.0-b10642d"
```

Or use the provided script to auto-update with git commit hash:
```bash
./writeversion.sh
```

## Utility Scripts

### writeversion.sh
Updates `Version.h` with current git commit hash:
```bash
./writeversion.sh
```

### cleanup-branches.sh  
Cleans up local git branches that no longer exist on remote:
```bash
./cleanup-branches.sh
```

## Serial Output

The device provides detailed status information via Serial Monitor:
- Firmware version and build date/time
- WiFi scan results (all APs found for your SSID)
- Connection to strongest AP with RSSI values
- IP address, Gateway, DNS information
- NTP synchronization status (with fallback attempts)
- Current time in both UTC and local timezone
- OTA service initialization
- WPS pairing status and PIN (if applicable)

Example output:
```
========================================
  Deckenlampe - Ceiling Lamp Controller
========================================
Firmware: 1.0.0-49f1b04
Build Date: Jan  3 2026 20:11:23
========================================

WiFi Connector Initializing
Hostname: CeilingLamp_A1B2
Scanning for WiFi networks...
Found 5 networks
Found 2 access points for 'MyWiFi':
  AA:BB:CC:DD:EE:FF (-45 dBm, Channel 6)
  11:22:33:44:55:66 (-62 dBm, Channel 11)
Connecting to strongest AP: AA:BB:CC:DD:EE:FF (-45 dBm)...
Connected successfully!
IP Address: 192.168.1.100
Gateway: 192.168.1.1
DNS: 192.168.1.1

--- NTP Time Synchronization ---
Time successfully synchronized!
UTC Time: 2026-01-03 20:15:30
Local Time (Berlin): 2026-01-03 21:15:30 (Standard)

--- Initializing OTA Services ---
ArduinoOTA initialized
Ready for OTA updates on port 3232
Web OTA server started
Access web interface at http://192.168.1.100/
--- All Services Ready ---
```

## Troubleshooting

**Device won't connect:**
- Check that WiFi credentials are saved (use WPS for first-time setup)
- Ensure router is powered on and in range
- Verify correct board settings in Arduino IDE
- Check Serial Monitor for detailed connection logs

**WPS pairing fails:**
- Ensure WPS is enabled on your router
- Press WPS button on router when device prompts
- Some routers have WPS timeout - be quick
- Try power cycling both devices

**OTA update fails:**
- Ensure device is connected to WiFi
- Check that firmware `.bin` file is compatible
- Verify sufficient flash memory space
- For ArduinoOTA: Check firewall settings (port 3232)
- For Web OTA: Try a smaller firmware file first

**Time not syncing:**
- Check internet connectivity
- Verify DNS is working (check Serial Monitor)
- Firewall might block NTP (UDP port 123)
- Try setting gateway as NTP server (automatic fallback)

**Web interface not accessible:**
- Verify device IP address in Serial Monitor
- Ensure you're on the same network
- Try `http://` not `https://`
- Check firewall settings (port 80)

## Architecture

```
Deckenlampe/
├── Deckenlampe.ino              # Main sketch (minimal, uses modules)
├── WiFi_Manager.h/.cpp          # WiFi connection, WPS, NTP sync
├── OTA_Update.h/.cpp            # All three OTA methods + web interface
├── GeneralTimeConverter.h/.cpp  # Timezone and DST handling
└── Version.h                    # Firmware version with git hash
```

## License

Open source - free to use and modify.

## Author

icebear74
