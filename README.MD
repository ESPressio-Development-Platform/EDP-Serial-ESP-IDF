# EDP-Serial-ESP-IDF

**Application-selected IDF-backed UART TX FIFO ByteStreamWriter concrete**, package `0.1.0`, maintained on `feature/logging-v1` until its independently qualified `main` promotion. The native UART API is available under supported ESP-IDF and IDF-backed Arduino-ESP32 framework configurations; build and physical acceptance remain separate. The upstream **EDP-Serial** abstraction remains platform-neutral; no EDP-Logging or Serial-ESP-IDF code was imported into foundation or other abstract EDP owners.

## Purpose and public composition contract

`#include <ESPressio_Serial_ESP_IDF.hpp>` provides `ESPressio::Serial::ESP_IDF::UartFifoByteStreamWriter`, a concrete EDP-System Composition `Provider<Serial::Domain,Offers<Offer<Serial::ByteStreamWriter>>>`, with typed `ByteWriteResult TryWrite(std::span<const std::byte>) noexcept`. The class is non-copyable, non-movable, and constructed explicitly with one **application-selected `uart_port_t`** (e.g. `UART_NUM_1`). It exposes `Port()` for inspection. No UART peripheral, pins, baud, driver, native Queue, IRQ, global console, DMA buffer or worker is silently allocated here.

**Bootstrap must configure the ESP-IDF UART** via native IDF API (`uart_param_config`, `uart_set_pin`, `uart_driver_install` with **TX ring-buffer size exactly ZERO**); own its lifecycle and the selected port; and bind this concrete instance to the EDP-Serial **exclusive ByteStreamWriter** capability using EDP-System Composition. It must prevent any other user (`printf`, IDF console, another sink/queue, app diagnostics) writing to the same UART outside the composed Serial frame queue. The application chooses ALL hardware pins; there are no hardcoded board pins and no automatic peripheral installation.

`TryWrite` rejects an invalid UART port or uninstalled driver with typed `ProviderFailure`, and only for a correctly configured port issues exactly one **`uart_tx_chars(port,bytes,UINT32-safeCount)`**. ESP-IDF v5.5.5 documents this as immediate nonblocking transfer to currently available **hardware TX FIFO** capacity (never waits for enough space). A native count 0 maps to `WouldBlock`, positive count bounded by offered bytes to `Accepted`, negative/error/out-of-range count to `ProviderFailure`. The provided source memory is fully copied into native UART FIFO *before* returning Accepted, so it never retains a borrowed `span`; it is suitable for the EDP-Serial ByteStreamWriter ownership contract.

**Accepted bytes are NOT proof of transmitted-on-wire, host-received, flushed or durable LogRecord delivery.** A UART FIFO can retain bytes after TryWrite returns and later lose them on power loss. No native `uart_wait_tx_done` or implicit blocking spin/sleep is performed. EDP-Serial's higher-level EDLG pump and bounded FIFO are still opt-in application components, not automatically activated by this provider. This class is strictly **ordinary-context**, not ISR-safe.

## Dependencies and build

- **Mandatory direct EDP library:** `EDP-Serial` `0.1.0` (which consumes `EDP-System` Composition).
- **Platform-only SDK dependency:** ESP-IDF **`esp_driver_uart`** component. The owner `CMakeLists.txt` declares `REQUIRES esp_driver_uart edp_serial`, where the application must provide a CMake component named `edp_serial` exposing the actual EDP-Serial abstract headers (and its dependent EDP-System headers). No legacy `ESPressio-*` library is permitted.
- PlatformIO metadata `library.json` declares package `0.1.0`, `frameworks: [arduino, espidf]`, `platforms: [espressif32]`, and **EDP-Serial** dependency. A C++20-compatible **ESP32 Arduino core exposing the ESP-IDF 5.x UART driver** may also instantiate this exact native provider. This is not an Arduino-generic transport and is unrelated to the empty `EDP-Serial-Arduino` placeholder; incompatible Arduino platforms must be rejected.
- **Optional application dependency:** if using `serial/BoundedEdlgFrameQueue.hpp`, the application additionally explicitly declares `EDP-Logging` and `EDP-Threading` as specified by EDP-Serial's conditional dependency graph.

## Tests and outstanding qualification

`tests/run_host_tests.sh` supplies a **fake IDF uart.h** and scripted uart_tx_chars stub, but imports REAL `EDP-Serial`/`EDP-System` abstractions. GCC and Clang HOST suites PASSED on RPI400 run **20261016T090647-02bb7051** and test invalid ports, uninstalled driver, FIFO full, partial accepted count, negative/error/invalid provider count, empty input, and exact provider Composition contract. This fake-host test alone **cannot** establish real IDF compatibility or hardware output.

**RPI400 ESP-IDF v5.5.5 full ESP32 C++20 component compilation AND ELF link PASSED**, detached run **20261016T090824-aed310a8**, with the actual provider and native esp_driver_uart API. A scratch application was used, with actual owner `UartFifoByteStreamWriter` and `esp_driver_uart`; fixture lives at `/srv/espressio/scratch/gpt6-serial-idf-build-smoke-20261016`. This fixture is compile/link only, and must **NOT** be flashed onto the shared development boards; no UART pins/USB port were configured on any physical device.

**Still open:** ESP32 UART RX/loopback/scope tests and host capture of complete EDLG frames, baud/peripheral and console ownership, FreeRTOS scheduling/jitter, end-to-end accepted→wire→received guarantees, serial flush/power-loss semantics, full app Composition Bootstrap, Serial-Arduino concrete implementation, and target ESP32-S3 variant qualification. The native UART FIFO may be capacity-limited and callers must use the existing bounded retry/FIFO worker; there is no producer-blocking API or hidden Queue here.

[Architecture](docs/ARCHITECTURE.MD) • [Dependency Graph](DEPENDENCY_GRAPH.MD) • [Wiki](wiki/Home.md).

### Additional actual target build evidence (2026-10-16)

- **ESP32-S3, ESP-IDF v5.5.5:** independent full firmware C++20 compile and ELF link **PASS** detached RPI400 run `20261016T091324-d99b12b1`. This is a variant build, NOT an on-device stopwatch/UART test.
- **ESP32-WROOM-class end-to-end header/link smoke:** real C++20 EDP-Logging EDLG frame writer → optional EDP-Serial bounded FIFO → this concrete UART FIFO provider compiled and linked as **one ESP-IDF application** with actual EDP-System/Threading/Clock/Memory/Platform abstraction include components, **PASS** detached run `20261016T091604-f40040e3`. The fixture used a SYNTHETIC Composition mutex and a runtime-disabled write branch; it proves source/target linkage, NOT real FreeRTOS synchronization, delivery or physical UART throughput. Fixture source remains under `/srv/espressio/scratch/gpt6-serial-idf-build-smoke-20261016` and must NEVER be flashed as if it were a fully bootstrapped application.

## V2 demonstrations and public-release readiness — 12 November 2026

[UartFifoByteStreamWriter](demos/UartFifoByteStreamWriter/README.MD) provides all **three V2 example layouts**: Arduino IDE using the ESP32 Arduino core's underlying IDF UART API, PlatformIO Arduino-ESP32, and native PlatformIO ESP-IDF. The **safe default does not install a UART driver, select TX/RX pins, write to the system console or flash a device**. Opting into actual native TX requires the application to provide both `EDP_SERIAL_DEMO_TX_GPIO` and `EDP_SERIAL_DEMO_RX_GPIO`, reserve UART1/pins exclusively, and accept that a nonblocking `uart_tx_chars` result confirms only native FIFO acceptance, **not** delivery to a host. Full native target compile/link against real current framework libraries, physical UART loopback and scope/host capture are separate qualification gates; a committed sketch alone proves none of these.

The [0.1.0 changelog](CHANGELOG.md), [V2 developer Wiki](wiki/Home.md) and [source dependency graph](DEPENDENCY_GRAPH.MD) document the exact provider ownership. No version bump or release tag is implied. Downstream full EDLG Serial/Logging application assembly must also acquire a real keyed Threading Mutex; the bundled Serial queue demo's simulated single-context Mutex is never a production Threading concrete.
