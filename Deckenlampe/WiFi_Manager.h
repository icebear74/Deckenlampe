/**
 * WiFi_Manager.h - WiFi and WPS management for CeilingLamp
 * 
 * Handles WiFi connection with best AP selection, WPS pairing, and reconnection
 * Includes NTP time synchronization with timezone support
 * Based on Panelclock's ConnectionManager approach
 * 
 * Author: icebear74
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "WiFi.h"
#include "esp_wps.h"
#include <vector>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "GeneralTimeConverter.h"

// Default timezone (Berlin, Germany)
#define DEFAULT_TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// NTP Server configuration (fallback chain)
#define DEFAULT_NTP_SERVER_PRIMARY "ptbtime1.ptb.de"
#define DEFAULT_NTP_SERVER_SECONDARY "de.pool.ntp.org"
#define DEFAULT_NTP_SERVER_TERTIARY_IP "216.239.35.0" // Google Public NTP
#define DEFAULT_NTP_UPDATE_INTERVAL_MIN 60

// WiFi connection timeout configuration
extern const unsigned long WIFI_CONNECTION_TIMEOUT_MS;
extern const unsigned long WIFI_SCAN_TIMEOUT_MS;

// WPS configuration constants
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "XIAO"
#define ESP_MODEL_NUMBER  "ESP32S3"
#define ESP_MODEL_NAME    "SEED STUDIO"
#define ESP_DEVICE_NAME   "CeilingLamp"

// Delay constants
extern const unsigned int INITIAL_DELAY_MS;
extern const unsigned int CONNECTION_CHECK_DELAY_MS;

// WiFi AP structure for storing scan results
struct WiFiAP {
  String ssid;
  String bssid;
  int32_t rssi;
  int32_t channel;
};

// Global time converter instance
extern GeneralTimeConverter timeConverter;

// Function declarations
void initWiFi();
bool connectToBestAP();
bool syncTimeWithNTP();
void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
String generateUniqueHostname(const char* baseName);
String wpspin2string(uint8_t a[]);
void wpsInitConfig();

#endif // WIFI_MANAGER_H
