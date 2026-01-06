/**
 * Ceiling Lamp (Deckenlampe) - ESP32 WiFi Controller with OTA Updates
 * 
 * This sketch enables WiFi connectivity for an ESP32-based ceiling lamp.
 * Features:
 * - Automatic connection to best WiFi AP (strongest signal)
 * - WPS (WiFi Protected Setup) for easy pairing
 * - Three OTA update methods:
 *   1. ArduinoOTA - Update via Arduino IDE over WiFi
 *   2. Web-based OTA - Upload firmware via web interface
 *   3. HTTP OTA - Automatic update from web server
 * 
 * Hardware: XIAO ESP32S3
 * Author: icebear74
 */

#include "WiFi_Manager.h"
#include "OTA_Update.h"
#include "Version.h"
#include <FastLED.h>

#define NUM_LEDS 40
#define DATA_PIN D3

CRGB leds[NUM_LEDS];


// Serial communication baud rate
const unsigned long SERIAL_BAUD_RATE = 115200;
void fillAndShow(CRGB color) {
    for (int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = color;
    }
    FastLED.show();
}
/**
 * Setup function - Initializes system
 * 
 * Initializes serial communication, WiFi connection, and OTA services
 */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(10);

  Serial.println("\n\n========================================");
  Serial.println("  Deckenlampe - Ceiling Lamp Controller");
  Serial.println("========================================");
  Serial.printf("Firmware: %s\n", DECKENLAMPE_VERSION);
  Serial.printf("Build Date: %s %s\n", DECKENLAMPE_BUILD_DATE, DECKENLAMPE_BUILD_TIME);
  Serial.println("========================================\n");
  
  // Initialize WiFi connection
   FastLED.addLeds<SK6812, DATA_PIN, GRB>(leds, NUM_LEDS).setRgbw(RgbwDefault());
     FastLED.setBrightness(255);
  
  initWiFi();
  
  // Wait for WiFi to be connected before starting OTA
  // WPS might still be in progress, so we check periodically
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 60000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n--- Initializing OTA Services ---");
    
    // Initialize all OTA update methods
    setupArduinoOTA();
    setupWebOTA();
    
    Serial.println("--- All Services Ready ---\n");
  } else {
    Serial.println("WiFi not connected. OTA services not started.");
    Serial.println("Waiting for WPS pairing...");
  }
}

/**
 * Main loop function
 * 
 * Handles OTA operations and keeps services running
 */
void loop() {
  // Handle OTA updates (ArduinoOTA, Web Server, HTTP checks)
  if (WiFi.status() == WL_CONNECTED) {
    handleOTA();
  }
  fillAndShow(CRGB(255,255,255));
  delay(1000);
  fillAndShow(CRGB(255,200,200));
  delay(1000);
}
