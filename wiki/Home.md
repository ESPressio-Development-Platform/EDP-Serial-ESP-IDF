# EDP-Serial-ESP-IDF

Initial **ESP-IDF-specific concrete** EDP-Serial ByteStreamWriter implementation, version `0.1.0`, feature branch `feature/logging-v1`. No SDK concrete source enters EDP-Serial's platform-neutral owner.

The only public implementation is [UartFifoByteStreamWriter](Reference-serial_esp_idf-UartFifoByteStreamWriter). `TryWrite` performs a single `uart_tx_chars()` native **nonblocking FIFO push** after validating app-selected `uart_port_t` and installed driver. Return result is EDP-Serial's canonical `ByteWriteResult`: `Accepted` = bytes pushed into hardware FIFO, `WouldBlock` = 0 bytes due FIFO fullness, `ProviderFailure` = invalid port/driver/native error.

**Bootstrap must install/configure the chosen UART with TX ring buffer disabled (tx_buffer_size=0), choose baud/pins, provide the proper EDP-System Composition and prevent other writers from using the same endpoint.** No defaults, hardcoded pins, heap, implicit IRQ, DMA borrowing, auto-console, UART transmit-done wait or true physical acknowledgement.

[Architecture](../docs/ARCHITECTURE.MD) • [Dependency Graph](../DEPENDENCY_GRAPH.MD) • [Host tests](../tests/run_host_tests.sh).

All host tests use a **fake UART**; an actual ESP-IDF v5.5.5 ESP32 compilation/ELF link PASSED on RPI400 detached run **20261016T090824-aed310a8**, with NO board flashing, and no shared development board has been flashed.
