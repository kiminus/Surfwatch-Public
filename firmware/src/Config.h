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
#define LED_RED_GPIO_NUM  33
#define LED_FLASH_GPIO_NUM  4

// HTTP
#define NETWORK_QUEUE_MAX_SIZE 5
#define NETWORK_QUEUE_MAX_WAIT_MS  50    // Max wait to enqueue network message
#define WIFI_RECONNECT_MAX_TIME_MS 10000
#define WIFI_RECONNECT_RETRY_INTERVAL_MS 1000
#define SERVER_UPLOAD_IMAGE_URL "/api/devices/file/image"

// --- System Settings ---
#define DEVICE_NAME             "ESP32_Cam_01"
#define CAMERA_CAPTURE_INTERVAL_MS  5000  // Capture every 5 seconds
#define CAMERA_JPEG_QUALITY        12    // 0-63 Lower means better quality
#define CAMERA_FRAME_SIZE         FRAMESIZE_QVGA  // 320x240
#define CAMERA_FB_COUNT           1     // Number of frame buffers

#endif