/**
 * OTA_Update.cpp - OTA Update implementation for CeilingLamp
 * 
 * Implements all three OTA update methods:
 * 1. ArduinoOTA - Update via Arduino IDE over WiFi
 * 2. Web-based OTA - Upload firmware via web interface
 * 3. HTTP OTA - Automatic update from web server
 * 
 * Author: icebear74
 */

#include "OTA_Update.h"
#include "WiFi.h"

// OTA Configuration
const unsigned int OTA_PORT = 3232;
const unsigned long HTTP_UPDATE_CHECK_INTERVAL_MS = 3600000; // Check every hour
unsigned long lastUpdateCheck = 0;

// HTTP Update Server URLs (configure these to your update server)
const char* UPDATE_SERVER_URL = "http://your-update-server.com/firmware.bin";
const char* UPDATE_VERSION_URL = "http://your-update-server.com/version.txt";

// Web Server for OTA updates
WebServer server(80);

/**
 * HTML page for OTA web interface
 * Provides a user-friendly interface for uploading firmware updates
 */
const char* otaWebPage = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>CeilingLamp OTA Update</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      margin: 0;
      padding: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .container {
      background: white;
      padding: 30px;
      border-radius: 10px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      max-width: 500px;
      width: 100%;
    }
    h1 {
      color: #333;
      text-align: center;
      margin-bottom: 10px;
    }
    .version {
      text-align: center;
      color: #666;
      margin-bottom: 30px;
    }
    .info-box {
      background: #f0f0f0;
      padding: 15px;
      border-radius: 5px;
      margin-bottom: 20px;
    }
    .info-box p {
      margin: 5px 0;
      color: #555;
    }
    .upload-section {
      margin-top: 20px;
    }
    input[type='file'] {
      width: 100%;
      padding: 10px;
      margin: 10px 0;
      border: 2px dashed #667eea;
      border-radius: 5px;
      cursor: pointer;
    }
    input[type='submit'] {
      background: #667eea;
      color: white;
      padding: 12px 30px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      width: 100%;
      font-size: 16px;
      margin-top: 10px;
    }
    input[type='submit']:hover {
      background: #5568d3;
    }
    .progress {
      margin-top: 20px;
      display: none;
    }
    .progress-bar {
      width: 100%;
      height: 30px;
      background: #f0f0f0;
      border-radius: 5px;
      overflow: hidden;
    }
    .progress-fill {
      height: 100%;
      background: #667eea;
      width: 0%;
      transition: width 0.3s;
      display: flex;
      align-items: center;
      justify-content: center;
      color: white;
      font-weight: bold;
    }
    .message {
      margin-top: 20px;
      padding: 10px;
      border-radius: 5px;
      display: none;
    }
    .success {
      background: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
    }
    .error {
      background: #f8d7da;
      color: #721c24;
      border: 1px solid #f5c6cb;
    }
  </style>
</head>
<body>
  <div class='container'>
    <h1>🔆 CeilingLamp OTA Update</h1>
    <div class='version'>Firmware Version: )=====";

const char* otaWebPage2 = R"=====(</div>
    <div class='info-box'>
      <p><strong>Hostname:</strong> <span id='hostname'></span></p>
      <p><strong>IP Address:</strong> <span id='ip'></span></p>
      <p><strong>MAC Address:</strong> <span id='mac'></span></p>
    </div>
    <div class='upload-section'>
      <h3>Upload Firmware</h3>
      <p style='color: #666; font-size: 14px;'>Select a .bin firmware file to update the device</p>
      <form method='POST' action='/update' enctype='multipart/form-data' id='upload-form'>
        <input type='file' name='update' accept='.bin' required>
        <input type='submit' value='Upload & Update'>
      </form>
    </div>
    <div class='progress' id='progress'>
      <div class='progress-bar'>
        <div class='progress-fill' id='progress-fill'>0%</div>
      </div>
    </div>
    <div class='message' id='message'></div>
  </div>
  <script>
    document.getElementById('hostname').textContent = ')=====";

const char* otaWebPage3 = R"=====(';
    document.getElementById('ip').textContent = ')=====";

const char* otaWebPage4 = R"=====(';
    document.getElementById('mac').textContent = ')=====";

const char* otaWebPage5 = R"=====(';
    
    document.getElementById('upload-form').addEventListener('submit', function(e) {
      e.preventDefault();
      var formData = new FormData(this);
      var xhr = new XMLHttpRequest();
      
      document.getElementById('progress').style.display = 'block';
      
      xhr.upload.addEventListener('progress', function(e) {
        if (e.lengthComputable) {
          var percentComplete = Math.round((e.loaded / e.total) * 100);
          document.getElementById('progress-fill').style.width = percentComplete + '%';
          document.getElementById('progress-fill').textContent = percentComplete + '%';
        }
      });
      
      xhr.addEventListener('load', function() {
        if (xhr.status === 200) {
          document.getElementById('message').className = 'message success';
          document.getElementById('message').textContent = 'Update successful! Device is rebooting...';
          document.getElementById('message').style.display = 'block';
        } else {
          document.getElementById('message').className = 'message error';
          document.getElementById('message').textContent = 'Update failed: ' + xhr.responseText;
          document.getElementById('message').style.display = 'block';
        }
      });
      
      xhr.addEventListener('error', function() {
        document.getElementById('message').className = 'message error';
        document.getElementById('message').textContent = 'Upload error occurred';
        document.getElementById('message').style.display = 'block';
      });
      
      xhr.open('POST', '/update');
      xhr.send(formData);
    });
  </script>
</body>
</html>
)=====";

/**
 * Handle root page request - display OTA update interface
 */
void handleRoot() {
  String hostname = WiFi.getHostname();
  String ip = WiFi.localIP().toString();
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  String page = String(otaWebPage) + DECKENLAMPE_VERSION + 
                String(otaWebPage2) + hostname + 
                String(otaWebPage3) + ip + 
                String(otaWebPage4) + String(macStr) + 
                String(otaWebPage5);
  
  server.send(200, "text/html", page);
}

/**
 * Handle firmware upload and update
 */
void handleUpdate() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

/**
 * Handle update completion
 */
void handleUpdateEnd() {
  if (Update.hasError()) {
    server.send(500, "text/plain", "Update Failed");
  } else {
    server.send(200, "text/plain", "Update OK");
    delay(1000);
    ESP.restart();
  }
}

/**
 * Initialize ArduinoOTA for Arduino IDE updates
 */
void setupArduinoOTA() {
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname(WiFi.getHostname());
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("ArduinoOTA initialized");
  Serial.printf("Ready for OTA updates on port %d\n", OTA_PORT);
}

/**
 * Initialize Web Server for manual OTA updates
 */
void setupWebOTA() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdateEnd, handleUpdate);
  server.begin();
  
  Serial.println("Web OTA server started");
  Serial.printf("Access web interface at http://%s/\n", WiFi.localIP().toString().c_str());
}

/**
 * Check for firmware updates from HTTP server
 * Compares version and downloads new firmware if available
 */
void checkHTTPUpdate() {
  Serial.println("Checking for firmware updates...");
  
  WiFiClient client;
  HTTPUpdate httpUpdate;
  
  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  
  httpUpdate.onStart([]() {
    Serial.println("HTTP Update started");
  });
  
  httpUpdate.onEnd([]() {
    Serial.println("HTTP Update finished");
  });
  
  httpUpdate.onProgress([](int cur, int total) {
    Serial.printf("HTTP Update Progress: %d%%\n", (cur * 100) / total);
  });
  
  httpUpdate.onError([&httpUpdate](int err) {
    Serial.printf("HTTP Update Error (%d): %s\n", err, httpUpdate.getLastErrorString().c_str());
  });
  
  // In a real implementation, you would:
  // 1. Check UPDATE_VERSION_URL for new version
  // 2. Compare with FIRMWARE_VERSION
  // 3. If newer, download from UPDATE_SERVER_URL
  // For now, we'll just show the structure
  
  t_httpUpdate_return ret = httpUpdate.update(client, UPDATE_SERVER_URL);
  
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP Update failed: Error (%d): %s\n", 
                    httpUpdate.getLastError(), 
                    httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No updates available");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Update successful");
      break;
  }
}

/**
 * Handle OTA operations in the main loop
 * Call this function from the main loop to keep OTA services active
 */
void handleOTA() {
  ArduinoOTA.handle();
  server.handleClient();
  
  // Periodically check for HTTP updates
  if (millis() - lastUpdateCheck > HTTP_UPDATE_CHECK_INTERVAL_MS) {
    lastUpdateCheck = millis();
    // Uncomment the line below when you have a valid update server
    // checkHTTPUpdate();
  }
}
