# ESP-IDF UART provider source, demos and qualification

**Actual source:** [umbrella](Reference-serial_esp_idf-ESPressio_Serial_ESP_IDF), [UartFifoByteStreamWriter](Reference-serial_esp_idf-UartFifoByteStreamWriter), [Reference Index](Reference-Index). GCC/Clang host `tests/run_host_tests.sh` substitutes a fake IDF `uart.h` while including the *real* EDP-Serial/System headers and tests invalid port, driver absent, full FIFO, one partial accepted count, unexpected native error and exact Composition contract.

Native full ESP-IDF 5.5.5 ESP32 and ESP32-S3 source/ELF builds passed the named RPI400 October fixtures `20261016T090824-aed310a8`, `20261016T091324-d99b12b1` and full Logging→Serial→UART link fixture `20261016T091604-f40040e3`. These were **compile/link only**, no board flashed, no physical host capture, and the last fixture's Threading Mutex was synthetic.

The new [three-layout UartFifoByteStreamWriter demo](../demos/UartFifoByteStreamWriter/README.MD) has Arduino IDE (ESP32 core/IDF driver), PlatformIO Arduino and PlatformIO ESP-IDF source. It defaults to zero native UART activity; real pin setup is opt-in. RPI400 run `20261112T115923-1e73e5a0` passed GCC, Clang, Arduino-ESP32 and ESP-IDF using the SDK's declared `UART_NUM_MAX` invalid-port sentinel. No selected SDK runtime guarantees real TX wire ownership, uninterrupted bytes, sleep latency, host receipt or Flash durability without separate hardware evidence.

## GitHub-resolved PlatformIO demos

Current demo manifests fetch library dependencies through GitHub lib_deps
URLs. The source/profile migration and exact isolated-build receipts are
tracked in the [Policy handoff](https://github.com/ESPressio-Development-Platform/Policy/blob/main/Handoff/2026-11-19/2026-11-19-09-25-Logging-Estate-Demo-GitHub-Dependencies.MD).
These recipe builds are separate from earlier physical UART/archive
qualification. No fresh hardware execution is implied.
