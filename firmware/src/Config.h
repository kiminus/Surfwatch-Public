#ifndef CONFIG_H
#define CONFIG_H

// --- HARDWARE PINS (AI-THINKER) ---
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// --- WIFI & SERVER ---
#define WIFI_SSID       "YourWiFiName"
#define WIFI_PASS       "YourWiFiPass"

// HTTP (For Images)
#define SERVER_UPLOAD_URL "http://192.168.1.100:5000/api/surfwatch/cam"

// MQTT (For Control/Status)
#define MQTT_SERVER     "mqtt.example.com"
#define MQTT_PORT       1883
#define MQTT_TOPIC_CMD  "device/surfwatch/cmd"
#define MQTT_TOPIC_STAT "device/surfwatch/status"

#endif