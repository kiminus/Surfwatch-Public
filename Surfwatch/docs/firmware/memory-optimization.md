# Memory optimization

The hardware is a ESP32-CAM

```
00:38:39.540 > depth=13Guru Meditation Error: Core  0 panic'ed (Unhandled debug exception). 
00:38:39.540 > Debug exception reason: Stack canary watchpoint triggered (Processing Task)
```

### overhead of the Core Driver and logging

To optimize the memory usage, I need to first know how much memory it is using by default. Here is the memory usage logging during only core task initialization:

```
00:26:50.797 > Starting processingTask to monitor stack usage...
00:26:50.797 > [free stack: 1752 bytes]
00:26:51.277 > [free stack: 568 bytes]
00:26:52.277 > [free stack: 568 bytes]
```

- initially allocateed 2048 bytes for processingTask
- after initialization, only 1752 bytes left (296 bytes used)
- after logging starts, only 568 bytes left (1184 bytes used)
- so a reasonable estimate is that the core driver and logging uses about 1.2KB of stack memory

---

The Problem: Explain that the ESP32 only has ~520KB of internal SRAM, but a single UXGA (1600x1200) JPEG frame can easily exceed 100KB-200KB, and raw buffers are even larger.

The Solution (PSRAM): Detail how you enabled SPIRAM (Pseudo-Static RAM) in the sdkconfig to expand addressable memory by 4MB.

Key Configuration: Show the specific camera_config_t settings you tuned:

.fb_location = CAMERA_FB_IN_PSRAM: Proves you know how to direct the driver to specific memory regions.

.fb_count = 2: Explain that you used double-buffering so the Capture Task can fill one buffer while the Network Task transmits the other, increasing throughput.
