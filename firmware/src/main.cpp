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
    
    // 1. Initialize the Unified Queue
    // Holds 5 items max. If camera is faster than network, it fills up.
    networkQueue = xQueueCreate(5, sizeof(NetworkMessage));

    // 2. Create Network Task (Core 0 - Radio Core)
    xTaskCreatePinnedToCore(
        Task_Network, "Network", 
        8192, NULL,   // Large stack for HTTP/SSL
        1, &hNetwork, 0
    );

    // 3. Create Camera Task (Core 1 - App Core)
    xTaskCreatePinnedToCore(
        Task_Camera, "Camera", 
        4096, NULL, 
        2, &hCamera, 1  // Higher priority than Network to ensure capture timing
    );
    
    // Delete "setup/loop" task to save RAM
    vTaskDelete(NULL);
}

void loop() {
    // Empty
}