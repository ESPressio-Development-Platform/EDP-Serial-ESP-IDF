# EDP-Serial-ESP-IDF Changelog

All versions remain **0.1.0**. This is untagged feature-branch work until independently qualified demo and V2 owner review.

## Unreleased — 0.1.0

### Added

- Concrete, noncopyable/nonmovable `Serial::ESP_IDF::UartFifoByteStreamWriter` advertising the exact EDP-System Composition `Serial::ByteStreamWriter` offer and conforming to the EDP-Serial `ByteWriteEndpoint` C++20 noexcept operation.
- Explicit borrowed `uart_port_t` selection at application Bootstrap; `TryWrite(span<const byte>)` validates native installed port, rejects invalid/uninstalled states, then performs at most one `uart_tx_chars` call using a zero TX native ring-buffer. Positive accepted prefix, WouldBlock (zero) and provider failure are distinct typed results.
- Native ESP-IDF 5.5.5 ESP32 and ESP32-S3 target compile/link smoke tests and independent GCC/Clang fake UART host tests, previously recorded in October; no physical port was automatically configured.
- Three source-specific V2 demonstrations under `demos/UartFifoByteStreamWriter/` for Arduino IDE/PlatformIO Arduino-ESP32/PlatformIO ESP-IDF, with safe default and explicit opt-in pin configuration; selected ESP32 Arduino-IDF-backed framework compatibility is subject to actual target qualification.
- Repository-local V2 reference Wiki, source/resource/Composition ownership, target build and dependency documentation.

### Deliberate exclusions

- No hardcoded UART pins, baud, installed driver, global logger, console, ownership of the UART hardware lifecycle or free-running worker. The application must install IDF driver with **TX ring buffer exactly zero** and release it on shutdown.
- Accepted means bytes moved to native UART TX FIFO only, not physical line delivery, host receipt, persistence or real-time service. No ISR safety, indefinite throughput or Flash power-cut claim.
- EDP-Serial-Arduino generic concrete, hardware loopback/host capture, USB/CDC output, configured-app multi-producer Threading Mutex and output-drain semantics remain separate; provider source must not be promoted without source/target acceptance.
