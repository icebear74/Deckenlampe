/**
 * OTA_Update.h - OTA Update functionality for CeilingLamp
 * 
 * This file contains all three OTA update methods:
 * 1. ArduinoOTA - Update via Arduino IDE over WiFi
 * 2. Web-based OTA - Upload firmware via web interface
 * 3. HTTP OTA - Automatic update from web server
 * 
 * Author: icebear74
 */

#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <ArduinoOTA.h>
#include <WebServer.h>
#include <HTTPUpdate.h>
#include <Update.h>
#include "Version.h"

// OTA Configuration
extern const unsigned int OTA_PORT;
extern const unsigned long HTTP_UPDATE_CHECK_INTERVAL_MS;
extern unsigned long lastUpdateCheck;

// HTTP Update Server URLs
extern const char* UPDATE_SERVER_URL;
extern const char* UPDATE_VERSION_URL;

// Web Server instance
extern WebServer server;

// Function declarations
void setupArduinoOTA();
void setupWebOTA();
void checkHTTPUpdate();
void handleOTA();

// Web handler functions
void handleRoot();
void handleUpdate();
void handleUpdateEnd();

#endif // OTA_UPDATE_H
