#ifndef SHARED_H
#define SHARED_H

#include <Arduino.h>
#include "esp_camera.h"

// Types of messages the Network Task can handle
enum NetMsgType {
    MSG_IMAGE_UPLOAD,   // Payload is camera_fb_t*
    MSG_STATUS_UPDATE,  // Payload is StatusPayload*
    MSG_ALERT           // Payload is generic text or error code
};

// Data structure for Status Updates
struct StatusPayload {
    int batteryLevel;
    int wifiSignal;
    unsigned long uptime;
};

// The Unified Wrapper
struct NetworkMessage {
    NetMsgType type;
    void* payload; // Generic pointer (cast based on type)
};

// Global Queue Handle (Defined in main.cpp, used everywhere)
extern QueueHandle_t networkQueue;

#endif