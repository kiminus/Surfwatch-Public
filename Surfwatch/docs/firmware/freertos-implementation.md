# RTOS implementation 

**- The Producer-Consumer Pattern:** explicitly state that you used a producer-consumer architecture to decouple high-speed camera capture from unpredictable network latency.

* **Task Definition:**
  * Describe the **Capture Task** (High Priority): Runs on Core 1. Dedicated to triggering the OV2640 and acquiring the frame buffer. It blocks until a frame is ready.
  * Describe the **Transmission Task** (Medium Priority): Runs on Core 0 (leaving Core 1 free for sensor interrupts). Handles the MQTT publication loop.
* **Inter-Task Communication (The "Proof"):**
  * Explain how you used a **FreeRTOS Queue** (`xQueueSend` / `xQueueReceive`) to pass pointers to the image buffer rather than copying the full image data (which would waste RAM and CPU cycles).
  * Mention using a **Semaphore** or Mutex to protect the WiFi driver if multiple tasks (e.g., telemetry and video) try to access the network simultaneously.
* **Code Snippet to include:** A simplified block showing your `xTaskCreate` calls and the `xQueueReceive` loop.
