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
    if (httpResponse == 200) {
        String payload = http.getString(); // Example: "BLINK;REBOOT;REPORT"
        
        Serial.println("[HTTP] Raw Commands: " + payload);

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
        
    } else {
        Serial.printf("[HTTP] Error: %s\n", http.errorToString(httpResponse).c_str());
        http.end();
    }
}

void uploadImage(NetworkMessage& msg) {
    if(!checkWifiConnection()) {
        return;
    }
    
    if (!http.connected()) {
        http.end(); // Clean up previous connection
        Serial.println("[HTTP] establishing new connection...");
        http.setReuse(true);
        http.begin(String(SERVER_BASE_URL) + String(SERVER_UPLOAD_IMAGE_URL));
        http.addHeader("Content-Type", "image/jpeg");
        http.addHeader("Connection", "keep-alive");
    }
    
    msg.networkSendTime = millis();
    camera_fb_t* fb = (camera_fb_t*)msg.payload;

    int httpResponse = http.POST(fb->buf, fb->len);
    
    msg.networkReceiveTime = millis();
    handleHTTPResponse(httpResponse, http);
    msg.commandEndTime = millis();
}

void printNetworkMessageTimings(const NetworkMessage& msg) {
    Serial.println("---- Network Message Timings ----");
    Serial.printf("Payload Created At: %lu ms\n", msg.payloadCreateTime);
    Serial.printf("Network Send Time After Payload Creation: %lu ms\n", msg.networkSendTime - msg.payloadCreateTime);
    Serial.printf("Network Receive Time - After Send: %lu ms\n", msg.networkReceiveTime - msg.networkSendTime);
    Serial.printf("Command End Time - After Receive: %lu ms\n", msg.commandEndTime - msg.networkReceiveTime);
    Serial.printf("Total Time from Payload Creation to Command End: %lu ms\n", 
                  msg.commandEndTime - msg.payloadCreateTime);
    Serial.println("---------------------------------");
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
            printNetworkMessageTimings(msg);
        }
    }
}