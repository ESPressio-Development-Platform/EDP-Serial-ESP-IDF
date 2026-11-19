# EDP-Serial-ESP-IDF Changelog

## Consumer README — unreleased 0.1.0

- Replaced stale implementation history with a progressive native ESP-IDF UART usage guide and complete code.
- Documented explicit pin selection, zero TX ring, partial progress, bounded backpressure, local TX completion and guarded driver release.
- Revalidated the full mandatory/optional dependency closure and consumer documentation navigation; runtime and version unchanged.

## Demo dependency portability — unreleased0.1.0

- PlatformIO demos now resolve library dependencies from explicit GitHub URLs.
- Removed symlink and sibling-header dependency bypasses and reconciled consumer build guidance.
- Arduino recipes retain the qualified framework/library bundle; native provider selection stays explicit.

All versions remain **0.1.0**. These changes are untagged.

## Unreleased — 0.1.0

### Added

- Concrete, noncopyable/nonmovable `Serial::ESP_IDF::UartFifoByteStreamWriter` advertising the exact EDP-System Composition `Serial::ByteStreamWriter` offer and conforming to the EDP-Serial `ByteWriteEndpoint` C++20 noexcept operation.
- Explicit borrowed `uart_port_t` selection at application Bootstrap; `TryWrite(span<const byte>)` validates native installed port, rejects invalid/uninstalled states, then performs at most one `uart_tx_chars` call using a zero TX native ring-buffer. Positive accepted prefix, WouldBlock (zero) and provider failure are distinct typed results.
- Native ESP-IDF 5.5.5 ESP32 and ESP32-S3 target compile/link smoke tests and independent GCC/Clang fake UART host tests, previously recorded in October; no physical port was automatically configured.
- Three source-specific V2 demonstrations under `demos/UartFifoByteStreamWriter/` for Arduino IDE/PlatformIO Arduino-ESP32/PlatformIO ESP-IDF, with safe default and explicit opt-in pin configuration; selected ESP32 Arduino-IDF-backed framework compatibility is subject to actual target qualification.
- Repository-local V2 reference Wiki, source/resource/Composition ownership, target build and dependency documentation.

### Release integration

- The package manifest and host workflow select EDP-Serial from `main`.
- The demos and host regression use the SDK's declared `UART_NUM_MAX` invalid-port sentinel. RPI400 passed both host compilers and both native demo builds.

### Deliberate exclusions

- No hardcoded UART pins, baud, installed driver, global logger, console, ownership of the UART hardware lifecycle or free-running worker. The application must install IDF driver with **TX ring buffer exactly zero** and release it on shutdown.
- Accepted means bytes moved to native UART TX FIFO only, not physical line delivery, host receipt, persistence or real-time service. No ISR safety, indefinite throughput or Flash power-cut claim.
- EDP-Serial-Arduino generic concrete, hardware loopback/host capture, USB/CDC output, configured-app multi-producer Threading Mutex and output-drain semantics remain separate; provider source must not be promoted without source/target acceptance.
