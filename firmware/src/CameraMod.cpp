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
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    // Adjust quality based on needs
    config.frame_size = CAMERA_FRAME_SIZE;
    config.jpeg_quality = CAMERA_JPEG_QUALITY; 
    config.fb_count = CAMERA_FB_COUNT;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera Init Failed");
    }
}

NetworkMessage createImageUploadMessage(camera_fb_t* fb) {
    NetworkMessage msg;
    msg.type = MSG_IMAGE_UPLOAD;
    msg.payload = (void*)fb; // Pass ownership to Network Task
    msg.payloadCreateTime = millis();
    return msg;
}

void Task_Camera(void *pvParameters) {
    initCamera();
    
    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Capture failed");
            vTaskDelay(pdMS_TO_TICKS(CAMERA_CAPTURE_INTERVAL_MS));
            continue;
        }

        NetworkMessage msg = createImageUploadMessage(fb);

        if (xQueueSend(networkQueue, &msg, pdMS_TO_TICKS(NETWORK_QUEUE_MAX_WAIT_MS)) != pdTRUE) {
            Serial.println("NetQueue Full - Dropping Frame");
            esp_camera_fb_return(fb); 
        }

        Serial.println("Enqueue image, queue size: " + String(uxQueueMessagesWaiting(networkQueue)));

        vTaskDelay(pdMS_TO_TICKS(CAMERA_CAPTURE_INTERVAL_MS));
    }
}