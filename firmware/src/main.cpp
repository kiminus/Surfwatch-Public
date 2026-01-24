#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "base64.h" // Native ESP32 library for encoding

// --- AI-THINKER CAMERA PINOUT ---
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


// --- Task Handles ---
TaskHandle_t hCamera = NULL;
TaskHandle_t hCore   = NULL;
TaskHandle_t hNet    = NULL;

// --- Function to Init Camera ---
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
  
  // Low Quality for Serial Transfer (Fast)
  config.frame_size = FRAMESIZE_QVGA; // 320x240
  config.jpeg_quality = 12;           // 0-63 (lower is better quality)
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera Init Failed");
    return;
  }
}
// --- Task 1: Camera Capture (High Priority, Core 1) ---
// Why Core 1? Keeps it away from WiFi interrupts on Core 0.
void Task_Camera(void *pvParameters) {
  Serial.println("[CAM] Task Started");
  initCamera(); // Initialize sensor

  while(true) {
    // 1. Capture Frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera Capture Failed");
      vTaskDelay(1000);
      continue;
    }

    // 2. Log Size (Sanity Check)
    Serial.printf("[CAM] Captured: %u bytes\n", fb->len);

    // 3. Convert to Base64 (Text) so we can copy it
    // Only do this once every 5 seconds to avoid spamming the console
    String base64Image = base64::encode(fb->buf, fb->len);
    
    Serial.println("\n--- COPY BELOW THIS LINE ---");
    Serial.print("data:image/jpeg;base64,");
    Serial.print(base64Image); // Prints the massive string
    Serial.println("\n--- COPY ABOVE THIS LINE ---\n");

    // 4. Return buffer to driver
    esp_camera_fb_return(fb);

    // Wait 5 seconds before next photo
    vTaskDelay(pdMS_TO_TICKS(5000)); 
  }
}
// --- Task 2: The Core/Orchestrator (Medium Priority, Core 1) ---
// Processes data. Runs on Core 1 to leave Core 0 for WiFi.
void Task_Core(void *pvParameters) {
  Serial.println("[CORE] Core Logic Task Started on Core " + String(xPortGetCoreID()));

  while(true) {
    // 1. Simulate Processing (e.g., resizing, checking brightness)
    // This connects the Camera to the Network (Logic Bridge)
    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}

// --- Task 3: Network Transmission (Low Priority, Core 0) ---
// Why Core 0? The ESP32 WiFi radio hardware lives here. 
void Task_Network(void *pvParameters) {
  Serial.println("[NET] Network Task Started on Core " + String(xPortGetCoreID()));

  while(true) {
    // 1. Simulate MQTT Publish (Unpredictable latency!)
    // In real code: client.publish(topic, data);
    
    // Simulate a "network lag" of 200ms
    vTaskDelay(pdMS_TO_TICKS(200)); 
    
    // Serial.println("[NET] Data sent to cloud");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- Surfwatch Firmware Booting ---");

  // 1. Create Camera Task
  xTaskCreatePinnedToCore(
    Task_Camera, "Camera", 
    4096, NULL, 
    2,            // Priority 2 (High) - Don't drop frames!
    &hCamera, 
    1             // Pin to Core 1
  );

  // 2. Create Core Logic Task
  xTaskCreatePinnedToCore(
    Task_Core, "Core", 
    3072, NULL, 
    1,            // Priority 1 (Medium)
    &hCore, 
    1             // Pin to Core 1
  );

  // 3. Create Network Task
  xTaskCreatePinnedToCore(
    Task_Network, "Network", 
    4096, NULL,   // Needs big stack for WiFi/MQTT
    1,            // Priority 1 (Medium/Low)
    &hNet, 
    0             // Pin to Core 0 (System Core)
  );
  
  // Note: The "loopTask" (Arduino loop) runs on Core 1 with Priority 1.
  // Since we aren't using it, we can delete it to save RAM.
  vTaskDelete(NULL);
}

void loop() {
  // Empty - Logic is moved to tasks
}