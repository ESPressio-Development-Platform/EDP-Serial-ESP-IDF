# EDP-Serial-ESP-IDF Architecture — first concrete UART FIFO provider

**Package `0.1.0`, feature `feature/logging-v1`, ESP-IDF v5.5.5 native target validated separately.**

The only concrete type is `Serial::ESP_IDF::UartFifoByteStreamWriter` in `src/serial_esp_idf/UartFifoByteStreamWriter.hpp`, imported via `ESPressio_Serial_ESP_IDF.hpp`. It inherits the **existing** EDP-System CompositionFramework provider offering **EDP-Serial::ByteStreamWriter**, and uses the actual ESP-IDF **`esp_driver_uart`** SDK in this *platform-specific owner*, not in EDP-Serial/EDP-Logging/EDP-System.

## Byte lifetime and scope

`TryWrite(span<const byte>)` validates the app-selected UART number, driver-installed state, and one UINT32-bounded native call. `uart_tx_chars` is specifically documented by ESP-IDF 5.5.5 to **never wait for TX FIFO capacity** and to return a count of bytes pushed to HW FIFO, with -1 on argument failure. This API must ONLY be used when the UART driver was installed with **TX ring-buffer size = 0**. The provider therefore maps 0=>WouldBlock, 1..offered=>Accepted, negative/out-of-range=>ProviderFailure and never retains source pointers.

Bootstrap owns native `uart_param_config`, pin selection `uart_set_pin`, `uart_driver_install`, selected UART port, and physical TX ownership. The provider never configures a board, picks pins, controls a console, flushes, waits for wire completion, allocates DMA, creates a Task, uses an ISR or sends data to another UART on its own. It is exclusively an ordinary-context ByteWriteEndpoint implementation. **FIFO Accepted does not mean UART frame transmitted or host received.**

For complete framed logs the app must also select **EDP-Serial's optional BoundedEdlgFrameQueue** and obey its single-endpoint ownership. EDP-Serial itself remains independent of ESP-IDF concrete code. The ownership graph is DAG.

## Fault, shutdown and lifecycle limits

A caller must not uninstall/reconfigure a UART while a TryWrite is in flight, nor use a TX ring buffer with `uart_tx_chars`. Multiple application writers need the separately composed Serial FIFO/worker; this native provider itself does not serialize competing writers or label log records. If UART driver is absent or port invalid, it returns ProviderFailure without invoking `uart_tx_chars`. When FIFO space is zero, it returns WouldBlock immediately and relies on the owner worker to retry, never spins. If native function reports inconsistent count, fails closed.

No automatic ACK/flush, interruption/power-loss provenance, secure UART confidentiality, complete LogRecord Typed Envelope, physical timing/scope traces, ESP32-S3 target build or Serial-Arduino port is implemented. These must be separately qualified.

## Test and documentation ownership

Host stub `tests/fake_idf/driver/uart.h` is used ONLY by `tests/run_host_tests.sh` and is forbidden in actual target builds. Native C++20 GCC/Clang tests verify typed provider semantics. The actual ESP-IDF component root `CMakeLists.txt` declares `REQUIRES esp_driver_uart edp_serial`, with the app providing the `edp_serial` component to expose EDP-Serial's public header and EDP-System transitive composition. A scratch RPI400 IDF v5.5.5 `esp32` app PASSED genuine firmware compile/ELF link (detached **20261016T090824-aed310a8**, exit 0), without flashing shared boards. See [README](../README.MD) and [Dependency Graph](../DEPENDENCY_GRAPH.MD).

### Additional actual target build evidence (2026-10-16)

- **ESP32-S3, ESP-IDF v5.5.5:** independent full firmware C++20 compile and ELF link **PASS** detached RPI400 run `20261016T091324-d99b12b1`. This is a variant build, NOT an on-device stopwatch/UART test.
- **ESP32-WROOM-class end-to-end header/link smoke:** real C++20 EDP-Logging EDLG frame writer → optional EDP-Serial bounded FIFO → this concrete UART FIFO provider compiled and linked as **one ESP-IDF application** with actual EDP-System/Threading/Clock/Memory/Platform abstraction include components, **PASS** detached run `20261016T091604-f40040e3`. The fixture used a SYNTHETIC Composition mutex and a runtime-disabled write branch; it proves source/target linkage, NOT real FreeRTOS synchronization, delivery or physical UART throughput. Fixture source remains under `/srv/espressio/scratch/gpt6-serial-idf-build-smoke-20261016` and must NEVER be flashed as if it were a fully bootstrapped application.
