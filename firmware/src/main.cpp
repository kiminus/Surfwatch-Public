#include <Arduino.h>
#include "Config.h"
#include "Shared.h"
#include "CameraMod.h"
#include "NetworkMod.h"

// Define the Global Queue
QueueHandle_t networkQueue;

// Task Handles
TaskHandle_t hCamera = NULL;
TaskHandle_t hNetwork = NULL;

void setup() {
    Serial.begin(115200);
    networkQueue = xQueueCreate(5, sizeof(NetworkMessage));

    pinMode(LED_FLASH_GPIO_NUM, OUTPUT);

    xTaskCreatePinnedToCore(
        Task_Network, "Network", 
        8192, NULL,   // Large stack for HTTP/SSL
        1, &hNetwork, 0
    );

    xTaskCreatePinnedToCore(
        Task_Camera, "Camera", 
        4096, NULL, 
        2, &hCamera, 1  // Higher priority than Network to ensure capture timing
    );
    
    // Delete "setup/loop" task to save RAM
    vTaskDelete(NULL);
}
void loop() {}