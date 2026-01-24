#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h> // Requires PubSubClient Library
#include "Config.h"
#include "Shared.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- HELPER: Upload Image (HTTP) ---
void uploadImage(camera_fb_t* fb) {
    if(WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    http.begin(SERVER_UPLOAD_URL);
    http.addHeader("Content-Type", "image/jpeg");
    
    int httpResponse = http.POST(fb->buf, fb->len);
    
    if (httpResponse > 0) {
        Serial.printf("[HTTP] Upload success: %d\n", httpResponse);
    } else {
        Serial.printf("[HTTP] Error: %s\n", http.errorToString(httpResponse).c_str());
    }
    http.end();
}

// --- HELPER: Publish Status (MQTT) ---
void sendStatus(StatusPayload* status) {
    if (!mqttClient.connected()) return;
    
    char jsonBuffer[128];
    snprintf(jsonBuffer, sizeof(jsonBuffer), 
             "{\"battery\": %d, \"wifi\": %d, \"uptime\": %lu}", 
             status->batteryLevel, status->wifiSignal, status->uptime);
             
    mqttClient.publish(MQTT_TOPIC_STAT, jsonBuffer);
    Serial.println("[MQTT] Status Published");
}

// --- MAIN TASK ---
void Task_Network(void *pvParameters) {
    // 1. Setup WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    // 2. Setup MQTT
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    // mqttClient.setCallback(callback); // Add callback for incoming commands

    NetworkMessage msg;

    while (true) {
        // --- A. MQTT Maintenance ---
        // Ensure we are connected for control/status messages
        if (!mqttClient.connected()) {
            if (mqttClient.connect("ESP32CamClient")) {
                mqttClient.subscribe(MQTT_TOPIC_CMD);
            }
        }
        mqttClient.loop(); // Process incoming MQTT messages

        // --- B. Process Queue (The Consumer) ---
        // Wait up to 100ms for a message. If nothing, loop back to check MQTT.
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
                    sendStatus(status);     // Do the light MQTT work
                    delete status;          // FREE MEMORY (Crucial!)
                    break;
                }
            }
        }
    }
}