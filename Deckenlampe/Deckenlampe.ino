/**
 * Ceiling Lamp (Deckenlampe) - ESP32 WiFi Controller
 * 
 * This sketch enables WiFi connectivity for an ESP32-based ceiling lamp.
 * It attempts to connect to a known WiFi network, and if unsuccessful,
 * initiates WPS (WiFi Protected Setup) for easy pairing.
 * 
 * Hardware: XIAO ESP32S3
 * Author: icebear74
 */

#include "WiFi.h"
#include "esp_wps.h"

// WiFi connection timeout configuration
const unsigned long WIFI_CONNECTION_TIMEOUT_MS = 6000;

// WPS configuration constants
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "XIAO"
#define ESP_MODEL_NUMBER  "ESP32S3"
#define ESP_MODEL_NAME    "SEED STUDIO"
#define ESP_DEVICE_NAME   "CeilingLamp"

// Serial communication baud rate
const unsigned long SERIAL_BAUD_RATE = 115200;

// Delay constants
const unsigned int INITIAL_DELAY_MS = 10;
const unsigned int CONNECTION_CHECK_DELAY_MS = 500;

// Global WPS configuration
static esp_wps_config_t config;

/**
 * Initialize WPS (WiFi Protected Setup) configuration
 * Sets up device information and WPS type for pairing
 */
void wpsInitConfig() {
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
  strcpy(config.pin, "00000000");
}

/**
 * Generate unique hostname by appending last 4 digits of WiFi MAC address
 * This ensures each device has a unique network identifier
 * 
 * @param baseName Base hostname (e.g., "CeilingLamp")
 * @return Unique hostname with MAC suffix (e.g., "CeilingLamp_A1B2")
 */
String generateUniqueHostname(const char* baseName) {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macSuffix[5];
  sprintf(macSuffix, "%02X%02X", mac[4], mac[5]);
  return String(baseName) + "_" + String(macSuffix);
}

/**
 * Convert WPS PIN byte array to string
 * 
 * @param a Byte array containing the 8-digit WPS PIN
 * @return String representation of the WPS PIN
 */
String wpspin2string(uint8_t a[]) {
  char wps_pin[9];
  for (int i = 0; i < 8; i++) {
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}

/**
 * WiFi event handler
 * Handles various WiFi and WPS events including connection, disconnection,
 * and WPS pairing status
 * 
 * @param event The WiFi event type
 * @param info Additional event information
 */
void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("Station Mode Started");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("Connected to: " + String(WiFi.SSID()));
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Disconnected from station, attempting reconnection");
      WiFi.reconnect();
      break;

    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      Serial.println("WPS Successful, stopping WPS and connecting to: " + String(WiFi.SSID()));
      esp_wifi_wps_disable();
      delay(INITIAL_DELAY_MS);
      WiFi.begin();
      break;

    case ARDUINO_EVENT_WPS_ER_FAILED:
      Serial.println("WPS Failed, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
      break;

    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      Serial.println("WPS Timed out, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
      break;

    case ARDUINO_EVENT_WPS_ER_PIN:
      Serial.println("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code));
      break;

    default:
      break;
  }
}

/**
 * Setup function - Initializes WiFi connection
 * 
 * Attempts to connect to a saved WiFi network. If connection fails within
 * the timeout period, initiates WPS pairing mode for easy setup.
 */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(INITIAL_DELAY_MS);

  Serial.println();
  Serial.println("WiFi Connector Initializing");
  
  // Generate unique hostname with MAC address suffix
  String hostname = generateUniqueHostname(ESP_DEVICE_NAME);
  WiFi.setHostname(hostname.c_str());
  Serial.println("Hostname: " + hostname);
  
  WiFi.begin();
  Serial.print("Attempting WiFi connection");

  unsigned long startAttemptTime = millis();
  
  // Wait for connection with timeout
  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < WIFI_CONNECTION_TIMEOUT_MS) {
    Serial.print(".");
    delay(CONNECTION_CHECK_DELAY_MS);
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Connection failed, start WPS
    Serial.println();
    Serial.println("Connection unsuccessful. Starting WPS pairing mode");

    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);

    wpsInitConfig();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
  } else {
    // Connection successful
    Serial.println();
    Serial.println("Connected to: " + String(WiFi.SSID()));
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

/**
 * Main loop function
 * 
 * Currently no active tasks in the main loop as WiFi management
 * is handled through event callbacks
 */
void loop() {
  // No active tasks - WiFi events handled by callbacks
  delay(INITIAL_DELAY_MS);
}