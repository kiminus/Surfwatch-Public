#include "Config.h"
#include "Shared.h"

void initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Adjust quality based on needs
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12; 
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera Init Failed");
    }
}

void Task_Camera(void *pvParameters) {
    initCamera();
    
    while (true) {
        // 1. Capture
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Capture failed");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        // 2. Wrap Message
        NetworkMessage msg;
        msg.type = MSG_IMAGE_UPLOAD;
        msg.payload = (void*)fb; // Pass ownership to Network Task

        // 3. Send to Queue
        // Wait 50ms for space. If full, DROP frame and release memory immediately.
        if (xQueueSend(networkQueue, &msg, 50 / portTICK_PERIOD_MS) != pdTRUE) {
            Serial.println("NetQueue Full - Dropping Frame");
            esp_camera_fb_return(fb); 
        }

        // 4. Interval (5 seconds)
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}