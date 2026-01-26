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
