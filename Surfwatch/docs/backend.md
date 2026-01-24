
* **MQTT Protocol Design:**
  * Define your topic structure (e.g., `surfwatch/cam_01/stream` for images, `surfwatch/cam_01/telemetry` for temp/rssi).
  * Explain why you chose MQTT over HTTP (lower overhead, keep-alive session, better for unstable IoT connections).
* **The AI Pipeline:**
  * Describe the image decoding process: `Base64 String -> Numpy Array -> OpenCV Decode`.
  * **YOLO Integration:** Explain that you run inference on the received frame to get bounding boxes (class: "person").
  * **Optimization:** Mention if you used `YOLOv8-nano` or `tiny` to optimize for latency on the server side.
