#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h> // Requires PubSubClient Library
#include "Config.h"
#include "Shared.h"
#include "Secrets.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void executeCommand(String cmd) {
    cmd.trim(); // Remove whitespace just in case
    Serial.println("[HTTP] Executing Command: " + cmd);
    // --- LED States ---
    if (cmd == "SET_LED_STATE_ON") {
        Serial.println("Action: LED ON");
        digitalWrite(LED_FLASH_GPIO_NUM, HIGH);

    } else if (cmd == "SET_LED_STATE_OFF") {
        Serial.println("Action: LED OFF");
        digitalWrite(LED_FLASH_GPIO_NUM, LOW);

    } else if (cmd == "SET_LED_STATE_BLINK") {
        Serial.println("Action: LED Blinking");
        // enableBlinkMode();

    // --- Power & System (Grouped Synonyms) ---
    } else if (cmd == "POWER_ON") {
        Serial.println("Action: System Wake");
        
    } else if (cmd == "POWER_OFF") {
        Serial.println("Action: System Sleep");
        esp_deep_sleep_start();

    } else if (cmd == "REBOOT") {
        Serial.println("Action: Rebooting...");
        ESP.restart();
    // --- Default / Unknown ---
    } else {
        Serial.print("Unknown Command: ");
        Serial.println(cmd);
    }
}

void uploadImage(camera_fb_t* fb) {
    if(WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(String(SERVER_BASE_URL) + String(SERVER_UPLOAD_IMAGE_URL));
    http.addHeader("Content-Type", "image/jpeg");

    // --- ADD CUSTOM HEADERS HERE ---
    http.addHeader("X-Device-ID", DEVICE_NAME);
    http.addHeader("X-Timestamp", String(millis()));

    int httpResponse = http.POST(fb->buf, fb->len);
    if (httpResponse == 200) {
        String payload = http.getString(); // Example: "BLINK;REBOOT;OPEN_VALVE"
        
        Serial.println("[HTTP] Raw Commands: " + payload);

        if (payload == "NONE" || payload.length() == 0) {
            http.end(); 
            return;
        }

        // --- PARSING LOGIC (Split by ';') ---
        int startIndex = 0;
        int endIndex = payload.indexOf(';');
        
        while (endIndex != -1) {
            // Extract single command
            String cmd = payload.substring(startIndex, endIndex);
            executeCommand(cmd);
            
            // Move to next
            startIndex = endIndex + 1;
            endIndex = payload.indexOf(';', startIndex);
        }
        
        // Catch the last command (after the last ';')
        String lastCmd = payload.substring(startIndex);
        if (lastCmd.length() > 0) executeCommand(lastCmd);
        
    } else {
        Serial.printf("[HTTP] Error: %s\n", http.errorToString(httpResponse).c_str());
    }
    http.end();
}

// --- MAIN TASK ---
void Task_Network(void *pvParameters) {
    // 1. Setup WiFi
    Serial.println("Connecting to WiFi..., SSID: " + String(WIFI_SSID) + ", PASS: " + String(WIFI_PASS));
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    NetworkMessage msg;

    while (true) {
        // --- Process Queue (The Consumer) ---
        if (xQueueReceive(networkQueue, &msg, 100 / portTICK_PERIOD_MS) == pdTRUE) {
            
            switch (msg.type) {
                case MSG_IMAGE_UPLOAD: {
                    camera_fb_t* fb = (camera_fb_t*)msg.payload;
                    uploadImage(fb);        // Do the heavy HTTP work
                    esp_camera_fb_return(fb); // FREE MEMORY (Crucial!)
                    break;
                }

                case MSG_STATUS_UPDATE: {
                    StatusPayload* status = (StatusPayload*)msg.payload;
                    delete status;          // FREE MEMORY (Crucial!)
                    break;
                }
            }
        }
    }
}