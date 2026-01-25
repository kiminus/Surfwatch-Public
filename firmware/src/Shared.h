#ifndef SHARED_H
#define SHARED_H

#include <Arduino.h>
#include "esp_camera.h"

enum NetMsgType {
    MSG_IMAGE_UPLOAD,   // Payload is camera_fb_t*
    MSG_STATUS_UPDATE,  // Payload is StatusPayload*
    MSG_ALERT           // Payload is generic text or error code
};

struct StatusPayload {
    int batteryLevel;
    int wifiSignal;
    unsigned long uptime;
};

struct NetworkMessage {
    NetMsgType type;
    void* payload;
};

extern QueueHandle_t networkQueue;

#endif