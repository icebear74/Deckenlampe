/**
 * WiFi_Manager.cpp - WiFi and WPS management implementation
 * 
 * Handles WiFi connection with best AP selection, WPS pairing, and reconnection
 * Includes NTP time synchronization with timezone support
 * Based on Panelclock's ConnectionManager approach
 * 
 * Author: icebear74
 */

#include "WiFi_Manager.h"
#include "Version.h"
#include <algorithm>

// WiFi connection timeout configuration
const unsigned long WIFI_CONNECTION_TIMEOUT_MS = 20000;  // 20 seconds for connection
const unsigned long WIFI_SCAN_TIMEOUT_MS = 10000;        // 10 seconds for scan

// Delay constants
const unsigned int INITIAL_DELAY_MS = 10;
const unsigned int CONNECTION_CHECK_DELAY_MS = 500;

// Global WPS configuration
static esp_wps_config_t config;

// NTP Client
static WiFiUDP ntpUdp;
static NTPClient ntpClient(ntpUdp, DEFAULT_NTP_SERVER_PRIMARY, 0, 60 * 60 * 24 * 1000UL);

// Global time converter instance (Berlin timezone by default)
GeneralTimeConverter timeConverter(DEFAULT_TIMEZONE);

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
      
      // WiFi.begin() without parameters will use the credentials stored by WPS
      // These are automatically saved to NVS (Non-Volatile Storage) by ESP32
      WiFi.begin();
      
      // Wait for connection
      {
        int retries = 40;
        while (WiFi.status() != WL_CONNECTED && retries > 0) {
          delay(CONNECTION_CHECK_DELAY_MS);
          retries--;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("WPS connection established!");
          Serial.println("WiFi credentials saved to NVS for future use");
          syncTimeWithNTP();
        } else {
          Serial.println("WPS pairing succeeded but connection failed");
        }
      }
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
 * Connect to the best available AP
 * Scans for saved networks and connects to the one with the strongest signal
 * Based on Panelclock's approach
 * 
 * @return true if connected successfully, false otherwise
 */
bool connectToBestAP() {
  Serial.println("Attempting to connect with saved credentials...");
  
  WiFi.mode(WIFI_STA);
  
  // First, try to connect with saved credentials (no parameters)
  // This will use credentials stored in NVS from previous WPS or manual config
  WiFi.begin();
  
  Serial.print("Connecting");
  int retries = 40;  // 40 * 500ms = 20 seconds
  while (WiFi.status() != WL_CONNECTED && retries > 0) {
    Serial.print(".");
    delay(CONNECTION_CHECK_DELAY_MS);
    retries--;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    // Successfully connected with saved credentials
    String connectedSSID = WiFi.SSID();
    Serial.printf("Connected to saved network: %s\n", connectedSSID.c_str());
    
    // Now scan to see if there's a better AP with the same SSID
    Serial.println("Scanning for potentially better access points...");
    WiFi.scanNetworks(true);  // Async scan
    delay(3000);  // Wait for scan to complete
    int n = WiFi.scanComplete();
    
    if (n > 0) {
      Serial.printf("Found %d networks\n", n);
      
      // Find all APs matching the connected SSID
      std::vector<WiFiAP> matchingAPs;
      int32_t currentRSSI = WiFi.RSSI();
      
      for (int i = 0; i < n; ++i) {
        if (WiFi.SSID(i) == connectedSSID) {
          WiFiAP ap;
          ap.ssid = WiFi.SSID(i);
          ap.bssid = WiFi.BSSIDstr(i);
          ap.rssi = WiFi.RSSI(i);
          ap.channel = WiFi.channel(i);
          matchingAPs.push_back(ap);
        }
      }
      
      if (matchingAPs.size() > 1) {
        // Sort by signal strength
        std::sort(matchingAPs.begin(), matchingAPs.end(), 
          [](const WiFiAP& a, const WiFiAP& b) { 
            return a.rssi > b.rssi; 
          });
        
        Serial.printf("Found %d access points for '%s':\n", matchingAPs.size(), connectedSSID.c_str());
        for (const auto& ap : matchingAPs) {
          Serial.printf("  %s (%d dBm, Channel %d)%s\n", 
                        ap.bssid.c_str(), ap.rssi, ap.channel,
                        (ap.rssi == currentRSSI) ? " [CURRENT]" : "");
        }
        
        // Check if we're already on the best AP
        if (matchingAPs[0].rssi > currentRSSI + 10) {  // 10 dBm better
          Serial.printf("Found better AP with %d dBm stronger signal, reconnecting...\n", 
                       matchingAPs[0].rssi - currentRSSI);
          
          // Parse BSSID string to byte array
          uint8_t bssid_arr[6];
          sscanf(matchingAPs[0].bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                 &bssid_arr[0], &bssid_arr[1], &bssid_arr[2], 
                 &bssid_arr[3], &bssid_arr[4], &bssid_arr[5]);
          
          // Reconnect to better AP (credentials already stored, just need BSSID)
          WiFi.disconnect();
          delay(100);
          WiFi.begin(connectedSSID.c_str(), WiFi.psk().c_str(), matchingAPs[0].channel, bssid_arr);
          
          retries = 40;
          while (WiFi.status() != WL_CONNECTED && retries > 0) {
            Serial.print(".");
            delay(CONNECTION_CHECK_DELAY_MS);
            retries--;
          }
          Serial.println();
          
          if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("Reconnected to better AP: %s (%d dBm)\n", 
                         matchingAPs[0].bssid.c_str(), matchingAPs[0].rssi);
          }
        } else {
          Serial.println("Already connected to the best available AP");
        }
      }
      
      WiFi.scanDelete();  // Clean up scan results
    }
    
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP(0).toString().c_str());
    return true;
    
  } else {
    // Could not connect with saved credentials
    Serial.println("No saved credentials or connection failed");
    return false;
  }
}

/**
 * Synchronize time with NTP servers
 * Uses fallback chain: PTB -> de.pool.ntp.org -> Gateway -> Google NTP
 * 
 * @return true if time sync successful, false otherwise
 */
bool syncTimeWithNTP() {
  Serial.println("\n--- NTP Time Synchronization ---");
  
  // Check DNS resolution for primary NTP server
  IPAddress ntpServerIP;
  Serial.printf("Checking DNS resolution for '%s'...\n", DEFAULT_NTP_SERVER_PRIMARY);
  if (WiFi.hostByName(DEFAULT_NTP_SERVER_PRIMARY, ntpServerIP)) {
    Serial.printf("  > DNS resolution SUCCESSFUL. IP: %s\n", ntpServerIP.toString().c_str());
  } else {
    Serial.println("  > DNS resolution FAILED!");
  }
  
  ntpClient.begin();
  
  // Attempt 1: Primary NTP server (PTB Germany)
  Serial.printf("NTP Attempt 1: %s\n", DEFAULT_NTP_SERVER_PRIMARY);
  ntpClient.setPoolServerName(DEFAULT_NTP_SERVER_PRIMARY);
  if (!ntpClient.forceUpdate()) {
    Serial.println("  > Attempt 1 failed.");
    
    // Attempt 2: Secondary NTP server (de.pool.ntp.org)
    Serial.printf("NTP Attempt 2: %s\n", DEFAULT_NTP_SERVER_SECONDARY);
    ntpClient.setPoolServerName(DEFAULT_NTP_SERVER_SECONDARY);
    if (!ntpClient.forceUpdate()) {
      Serial.println("  > Attempt 2 failed.");
      
      // Attempt 3: Use Gateway as NTP server
      String gatewayIpStr = WiFi.gatewayIP().toString();
      Serial.printf("NTP Attempt 3 (Gateway): %s\n", gatewayIpStr.c_str());
      ntpClient.setPoolServerName(gatewayIpStr.c_str());
      if (!ntpClient.forceUpdate()) {
        Serial.println("  > Attempt 3 failed.");
        
        // Attempt 4: Fallback to Google Public NTP IP
        Serial.printf("NTP Attempt 4 (Fallback IP): %s\n", DEFAULT_NTP_SERVER_TERTIARY_IP);
        ntpClient.setPoolServerName(DEFAULT_NTP_SERVER_TERTIARY_IP);
        
        // Keep trying until success
        int retries = 5;
        while (!ntpClient.forceUpdate() && retries > 0) {
          Serial.println("  > Attempt 4 failed. Retrying...");
          delay(2000);
          retries--;
        }
        
        if (retries == 0) {
          Serial.println("NTP synchronization failed after all attempts!");
          return false;
        }
      }
    }
  }
  
  // Set system time
  time_t utc_time = ntpClient.getEpochTime();
  timeval tv = { utc_time, 0 };
  settimeofday(&tv, nullptr);
  
  Serial.println("Time successfully synchronized! System time is UTC.");
  
  // Display current time in both UTC and local time
  time_t now;
  time(&now);
  
  struct tm timeinfo_utc;
  gmtime_r(&now, &timeinfo_utc);
  Serial.printf("UTC Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                timeinfo_utc.tm_year + 1900, timeinfo_utc.tm_mon + 1, timeinfo_utc.tm_mday,
                timeinfo_utc.tm_hour, timeinfo_utc.tm_min, timeinfo_utc.tm_sec);
  
  // Convert to local time using GeneralTimeConverter
  time_t local_time = timeConverter.toLocal(now);
  struct tm timeinfo_local;
  gmtime_r(&local_time, &timeinfo_local);
  Serial.printf("Local Time (Berlin): %04d-%02d-%02d %02d:%02d:%02d %s\n",
                timeinfo_local.tm_year + 1900, timeinfo_local.tm_mon + 1, timeinfo_local.tm_mday,
                timeinfo_local.tm_hour, timeinfo_local.tm_min, timeinfo_local.tm_sec,
                timeConverter.isDST(now) ? "(DST)" : "(Standard)");
  
  Serial.println("--------------------------------\n");
  
  return true;
}

/**
 * Initialize WiFi connection
 * 
 * First tries to connect to saved WiFi with best AP selection.
 * If no saved credentials or connection fails, initiates WPS pairing mode.
 */
void initWiFi() {
  Serial.println();
  Serial.println("WiFi Connector Initializing");
  Serial.printf("Firmware Version: %s\n", DECKENLAMPE_VERSION);
  
  // Generate unique hostname with MAC address suffix
  String hostname = generateUniqueHostname(ESP_DEVICE_NAME);
  WiFi.setHostname(hostname.c_str());
  Serial.println("Hostname: " + hostname);
  
  // Try to connect to best AP with saved credentials
  bool connected = connectToBestAP();
  
  if (!connected) {
    // Connection failed or no saved credentials, start WPS
    Serial.println();
    Serial.println("Starting WPS pairing mode...");
    Serial.println("Please press the WPS button on your router");

    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);

    wpsInitConfig();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
  } else {
    // WiFi connected, now sync time
    syncTimeWithNTP();
  }
}
