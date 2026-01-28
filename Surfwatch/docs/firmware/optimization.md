# Latency optimization

## Baseline Latency Analysis
### [ESP: Baseline measurements](../results.md#baseline-latency-logs-esp)

- if the server is not step, the standard timeout for a network request is around 5 seconds, which conforms the default timeout setting:

```
18:56:57.784 > [HTTP] Error: connection refused
18:56:57.784 > ---- Network Message Timings ----
18:56:57.801 > Payload Created At: 130391 ms
18:56:57.805 > Network Send Time After Payload Creation: 1 ms
18:56:57.806 > Network Receive Time - After Send: 5002 ms
18:56:57.810 > Command End Time - After Receive: 0 ms
18:56:57.815 > Total Time from Payload Creation to Command End: 5003 ms
```

- the baseline lantency for a successful request when the server is idle is around 500 - 1450 ms, but we can see 

```
20:04:00.583 > [HTTP] Raw Commands: NONE
20:04:00.583 > ---- Network Message Timings ----
20:04:00.587 > Payload Created At: 311561 ms
20:04:00.589 > Network Send Time After Payload Creation: 1 ms
20:04:00.596 > Network Receive Time - After Send: 1364 ms
20:04:00.597 > Command End Time - After Receive: 102 ms
20:04:00.601 > Total Time from Payload Creation to Command End: 1467 ms
```


here is the graph of latency over time:

```mermaid
xychart-beta
    title "Network Latency (RTT) per Request"
    x-axis "Request Index" 0 --> 100
    y-axis "Latency (ms)" 0 --> 5200
    line [375, 453, 338, 638, 628, 569, 1092, 298, 388, 378, 283, 605, 348, 522, 473, 679, 580, 628, 615, 465, 559, 865, 360, 987, 945, 636, 822, 591, 433, 1339, 1086, 778, 546, 801, 493, 1185, 648, 386, 416, 574, 2719, 824, 1127, 728, 867, 685, 1301, 1715, 505, 830, 571, 564, 935, 465, 742, 534, 625, 664, 574, 465, 828, 408, 270, 226, 625, 407, 515, 430, 799, 511, 515, 929, 532, 3448, 914, 365, 421, 716, 735, 232, 296, 208, 785, 334, 414, 553, 163, 229, 554, 732, 156, 909, 470, 842, 1418, 1441, 940, 477, 1107, 943, 913, 776, 1824, 5054, 2159, 1652, 307, 546, 406, 581, 769, 426, 1336, 767, 233, 672, 619, 912, 1246, 695, 1150, 948, 823, 2374, 526, 1593, 1206]
```

and here is the histogram of latency distribution:

```mermaid
xychart-beta
    title "Network Latency Histogram (RTT)"
    x-axis "Latency Range (ms)" ["0–499","500–999","1000–1499","1500–1999","2000–2999","3000–3999","4000–5999"]
    y-axis "Request Count" 0 --> 60
    bar [49, 59, 22, 10, 6, 2, 1]
```
Analyzing the data, we have the following statistics:

| Metric         | Value               |
| -------------- | ------------------- |
| Minimum        | 156 ms              |
| Maximum        | 5054 ms             |
| Average (Mean) | 790 ms              |
| Median         | ~622 ms             |
| Mode           | 465 ms, 574 ms      |

- the maximum latency of 5054ms is an outlier, so that indicates a case **when the connection time out/packet loss happens**
- the average latency is **right-skewed** by the outliers, so median is a better representation of typical latency
- The high standard deviation of 680ms indicates extreme jitter, suggesting that there might be either network instability or instable request overhead.
- the minimum latency of 156ms **indicates my system actually functions correctly, and can achieve low latency**, confirming that the current bottlenecks are architectural rather than physical.

### Remove the Outliers

To better analyze the typical latency, we can remove the max 3 data and min 3 data points as outliers:

```mermaid
xychart-beta
    title "Network Latency (RTT) per Request (Trimmed)"
    x-axis "Request Index" 0 --> 100
    y-axis "Latency (ms)" 0 --> 2600
    line [375, 453, 338, 638, 628, 569, 1092, 298, 388, 378, 283, 605, 348, 522, 473, 679, 580, 628, 615, 465, 559, 865, 360, 987, 945, 636, 822, 591, 433, 1339, 1086, 778, 546, 801, 493, 1185, 648, 386, 416, 574, 824, 1127, 728, 867, 685, 1301, 1715, 505, 830, 571, 564, 935, 465, 742, 534, 625, 664, 574, 465, 828, 408, 270, 226, 625, 407, 515, 430, 799, 511, 515, 929, 532, 914, 365, 421, 716, 735, 232, 296, 785, 334, 414, 553, 229, 554, 732, 909, 470, 842, 1418, 1441, 940, 477, 1107, 943, 913, 776, 1824, 2159, 1652, 307, 546, 406, 581, 769, 426, 1336, 767, 233, 672, 619, 912, 1246, 695, 1150, 948, 823, 2374, 526, 1593, 1206]
```

The trimmed data shows:

- Min: 226
- Max: 2374
- Mean: 725.68
- Median: 625
- Standard Deviation: 385.50

### [Backend: Baseline measurements](../results.md#baseline-latency-logs-server)
the typical server processing time is around 40ms:

```
0: 480x640 1 person, 37.5ms
Speed: 1.2ms preprocess, 37.5ms inference, 0.4ms postprocess per image at shape (1, 3, 480, 640)
Results saved to ......
Detection Results: Found 1 objects
[IoT] Processed Image from ESP32_Cam_01, Detections: 1
INFO:     192.168.0.171:63600 - "POST /api/devices/... HTTP/1.1" 200 OK
```

here is the graph of server processing time over time:

```mermaid
xychart-beta
    title "Total Server Processing Time (Pre + Inference + Post)"
    x-axis "Request Index" 0 --> 113
    y-axis "Processing Time (ms)" 30 --> 50
    line [39.0, 41.3, 34.5, 37.4, 40.7, 44.4, 37.5, 39.4, 40.0, 37.1, 38.3, 40.0, 36.6, 38.3, 37.5, 40.4, 39.6, 41.8, 42.9, 43.6, 40.5, 40.6, 39.6, 42.1, 41.1, 40.1, 37.3, 41.0, 39.7, 40.5, 40.0, 44.3, 35.9, 43.9, 38.1, 44.2, 37.7, 44.0, 42.8, 39.8, 40.2, 40.5, 39.8, 42.9, 41.3, 37.4, 40.5, 38.0, 40.7, 40.5, 37.4, 35.8, 40.9, 40.7, 37.1, 41.0, 39.9, 39.4, 40.6, 42.4, 40.2, 37.6, 37.3, 41.2, 40.8, 37.5, 43.4, 42.3, 41.7, 35.1, 41.1, 42.1, 41.3, 34.8, 40.2, 36.5, 35.2, 38.8, 37.2, 38.5, 35.1, 39.8, 41.4, 36.7, 35.7, 33.5, 37.7, 33.2, 41.3, 36.4, 39.4, 37.0, 37.8, 32.5, 37.4, 37.6, 37.4, 39.4, 34.1, 38.4, 35.1, 39.5, 35.6, 43.9, 38.8, 40.2, 43.0, 37.9, 40.7, 41.8, 43.0, 35.2, 45.5]
```

the server processing time is relatively stable, with an average of 39.5ms and a standard deviation of 2.8ms, indicating that the server is not the main source of latency variability.

## Optimizations - `Keep Alive` & `HTTP Persistent Connections`

When I analysize the latency breakdown logs:

```
INFO:     192.xxx.x.xxx:55282 - "POST /api/devices/file/image HTTP/1.1" 200 OK
INFO:     192.xxx.x.xxx:55283 - "POST /api/devices/file/image HTTP/1.1" 200 OK
INFO:     192.xxx.x.xxx:55284 - "POST /api/devices/file/image HTTP/1.1" 200 OK
...
```

In my origial implementation, each HTTP request opens a new TCP connection, send the request, and then close the connection. This TCP connection setup and teardown may introduce significant overhead.

therefore, I only setup the http request once, and reuse the same connection for subsequent requests. Here is what I got:

```mermaid
xychart-beta
    title "Network Latency (RTT) per Request (ms)"
    x-axis "Request Index" 0 --> 100
    y-axis "Time (ms)" 0 --> 4500
    line [595, 607, 661, 703, 1601, 595, 387, 641, 347, 337, 1536, 842, 797, 1278, 1052, 1071, 1910, 3217, 799, 1087, 1174, 688, 571, 581, 937, 844, 853, 573, 845, 819, 1130, 862, 292, 607, 903, 346, 420, 1036, 768, 615, 530, 854, 352, 1318, 803, 659, 478, 605, 675, 1657, 838, 974, 1112, 915, 836, 995, 705, 1149, 593, 615, 1202, 390, 359, 611, 906, 633, 739, 379, 373, 2079, 469, 890, 649, 1427, 587, 915, 652, 699, 933, 1138, 2417, 1104, 4351, 785, 1076, 576, 1171, 684, 680, 652, 683, 794, 614, 555, 418, 1084, 1422, 1272, 1367, 622, 661, 743, 2649, 604, 595, 1209, 1355, 391, 605, 625, 1202, 693, 1395, 1300, 1083, 675, 972, 396, 235, 772, 559, 518, 541, 659, 1304, 848]
```

The "optimized" latency data shows:

- Mean: 893.14 ms
- Max: 4351 ms
- Min: 235 ms
- Median: 755.5 ms
- Standard Deviation (SD): 547.71 ms

which is even worse than before...

After removing the top 3 and bottom 3 outliers, we have:

```mermaid
xychart-beta
    title "Network Latency (RTT) per Request (ms)"
    x-axis "Request Index" 0 --> 120
    y-axis "Time (ms)" 0 --> 2500
    line [595, 607, 661, 703, 1601, 595, 387, 641, 347, 1536, 842, 797, 1278, 1052, 1071, 1910, 799, 1087, 1174, 688, 571, 581, 937, 844, 853, 573, 845, 819, 1130, 862, 607, 903, 346, 420, 1036, 768, 615, 530, 854, 352, 1318, 803, 659, 478, 605, 675, 1657, 838, 974, 1112, 915, 836, 995, 705, 1149, 593, 615, 1202, 390, 359, 611, 906, 633, 739, 379, 373, 2079, 469, 890, 649, 1427, 587, 915, 652, 699, 933, 1138, 2417, 1104, 785, 1076, 576, 1171, 684, 680, 652, 683, 794, 614, 555, 418, 1084, 1422, 1272, 1367, 622, 661, 743, 604, 595, 1209, 1355, 391, 605, 625, 1202, 693, 1395, 1300, 1083, 675, 972, 396, 772, 559, 518, 541, 659, 1304, 848]
```

The trimmed "optimized" latency data shows:

- Mean: 845.46 ms
- Max: 2417 ms
- Min: 346 ms
- Median: 755.5 ms
- Standard Deviation (SD): 364.93 ms

so where is the problem? I look back at the server logs, and found this:

```
INFO:     192.xxx.xxx.xxx:63912 - "POST /api/devices/file/image HTTP/1.1" 200 OK
INFO:     127.0.0.1:56071 - "GET /livereload/118715765/118716218 HTTP/1.1" 404 Not Found
INFO:     127.0.0.1:56071 - "GET /livereload/118715765/118716218 HTTP/1.1" 404 Not Found
...
INFO:     192.xxx.xxx.xxx:63912 - "POST /api/devices/file/image HTTP/1.1" 200 OK
INFO:     127.0.0.1:56071 - "GET /livereload/118715765/118716218 HTTP/1.1" 404 Not Found
```

This indicates there might be other unwanted background traffic on the server, interfering with the processing of my requests.

## Optimizations - Asynchronous Server Task flow

In addition, the current flow completes everything sequentially. So the server will first receive the entire image, process it, and then send back the response. This means the device has to wait for the entire processing to complete before it can proceed.

Therefore, I modified the server to handle the image processing asynchronously and return the response quicker

```python
def receive_image(...):
    raw_bytes = await request.body()
    ...
    background_tasks.add_task(background_image_processing, raw_bytes, x_device_id)
    pending_commands = await get_and_clear_commands(x_device_id)
    ...
    return formatted_pending_commands
```

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