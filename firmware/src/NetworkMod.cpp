#include <WiFi.h>
#include <HTTPClient.h>
#include "Config.h"
#include "Shared.h"
#include "Secrets.h"

WiFiClient espClient;
HTTPClient http;
bool checkWifiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected. Attempting Reconnect...");
        WiFi.disconnect();
        WiFi.reconnect();
        unsigned long startAttemptTime = millis();

        // Wait up to 10 seconds for connection
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_RECONNECT_MAX_TIME_MS) {
            vTaskDelay(pdMS_TO_TICKS(WIFI_RECONNECT_RETRY_INTERVAL_MS));
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nReconnected to WiFi");
            return true;
        } else {
            Serial.println("\nFailed to reconnect to WiFi");
            return false;
        }
    }
    Serial.printf("[WiFi] Status: %d dBm\n", WiFi.RSSI());
    return true;
}

void executeCommand(String cmd) {
    cmd.trim(); // Remove whitespace just in case
    Serial.println("[HTTP] Executing Command: " + cmd);
    if (cmd == "SET_LED_STATE_ON") {
        Serial.println("Action: LED ON");
        digitalWrite(LED_FLASH_GPIO_NUM, HIGH);
    } else if (cmd == "SET_LED_STATE_OFF") {
        Serial.println("Action: LED OFF");
        digitalWrite(LED_FLASH_GPIO_NUM, LOW);
    } else if (cmd == "SET_LED_STATE_BLINK") {
        Serial.println("Action: LED Blinking");
    } else if (cmd == "POWER_ON") {
        Serial.println("Action: System Wake");
    } else if (cmd == "POWER_OFF") {
        Serial.println("Action: System Sleep");
        esp_deep_sleep_start();
    } else if (cmd == "REBOOT") {
        Serial.println("Action: Rebooting...");
        ESP.restart();
    } else if (cmd == "REPORT") {
        Serial.println("Action: Status Report Requested");
    }else {
        Serial.print("Unknown Command: ");
        Serial.println(cmd);
    }
}

void handleHTTPResponse(int httpResponse, HTTPClient& http) {
    String payload = http.getString(); // Example: "BLINK;REBOOT;REPORT"
    
    // Serial.println("[HTTP] Raw Commands: " + payload);

    if (payload == "NONE" || payload.length() == 0) {
        return;
    }
    // --- PARSING LOGIC (Split by ';') ---
    int startIndex = 0;
    int endIndex = payload.indexOf(';');
    
    while (endIndex != -1) {
        String cmd = payload.substring(startIndex, endIndex);
        executeCommand(cmd);
        
        startIndex = endIndex + 1;
        endIndex = payload.indexOf(';', startIndex);
    }
    
    String lastCmd = payload.substring(startIndex);
    if (lastCmd.length() > 0) executeCommand(lastCmd);
}

void uploadImage(NetworkMessage& msg) {
    if(!checkWifiConnection()) {
        msg.state = "FAILED_WIFI";
        Serial.println("[METRIC] State: FAILED_WIFI");
        return;
    }

    uint32_t freeHeapBefore = ESP.getFreeHeap();
    camera_fb_t* fb = (camera_fb_t*)msg.payload;
    size_t dataSize = fb->len;
    msg.dataSize = dataSize;
    msg.freeHeapBefore = freeHeapBefore;
    
    if (!http.connected()) {
        http.end(); // Clean up previous connection
        Serial.println("[HTTP] establishing new connection...");
        http.setReuse(true);
        http.begin(String(SERVER_BASE_URL) + String(SERVER_UPLOAD_IMAGE_URL));
        http.addHeader("Content-Type", "image/jpeg");
        http.addHeader("Connection", "keep-alive");
        http.addHeader("X-Device-ID", DEVICE_NAME);
    }
    
    http.addHeader("X-Free-Heap", String(freeHeapBefore));

    unsigned long startNetworkTime = millis();
    int httpResponse = http.POST(fb->buf, fb->len); //BLOCKING!
    msg.latency = millis() - startNetworkTime;

    if (httpResponse > 0) {
        msg.state= "SUCCESS";
        handleHTTPResponse(httpResponse, http);
    } else {
        msg.state = "FAILED_HTTP";
        Serial.printf("[HTTP] Error: %s\n", http.errorToString(httpResponse).c_str());
        http.end();
    }
    msg.payloadEndTime = millis();
}
void logPerformanceMetrics(const NetworkMessage& msg) {
    Serial.printf("[METRIC] State:%s | Size:%u B | Latency:%lu ms | Heap:%u B | Lifecycle: %lu ms\n", 
                      msg.state.c_str(), msg.dataSize, msg.latency, msg.freeHeapBefore, msg.payloadEndTime - msg.payloadCreateTime);
}
// --- MAIN TASK ---
void Task_Network(void *pvParameters) {
    Serial.println("Connecting to WiFi..., SSID: " + String(WIFI_SSID) + ", PASS: " + String(WIFI_PASS));
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    NetworkMessage msg;
    http.setTimeout(5000);

    while (true) {
        if (xQueueReceive(networkQueue, &msg, 100 / portTICK_PERIOD_MS) == pdTRUE) {
            Serial.println("Dequeued message, queue size: " + String(uxQueueMessagesWaiting(networkQueue)));
            switch (msg.type) {
                case MSG_IMAGE_UPLOAD: {
                    uploadImage(msg);        // Do the heavy HTTP work
                    esp_camera_fb_return((camera_fb_t*)msg.payload);
                    break;
                }

                case MSG_STATUS_UPDATE: {
                    StatusPayload* status = (StatusPayload*)msg.payload;
                    delete status;        
                    break;
                }
            }
            logPerformanceMetrics(msg);
        }
    }
}