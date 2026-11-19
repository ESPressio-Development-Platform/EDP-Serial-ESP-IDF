# EDP-Serial-ESP-IDF

Nonblocking ESP32 UART byte output for the [ESPressio Development Platform](https://github.com/ESPressio-Development-Platform).
**C++20 · Version 0.1.0 · Apache-2.0**

`UartFifoByteStreamWriter` connects EDP-Serial's portable byte-writing
contract to the ESP-IDF UART hardware TX FIFO. It works with native ESP-IDF
and qualified IDF-backed Arduino-ESP32 cores.

- Write bytes through the existing `ByteStreamWriter` Composition capability.
- Inspect the exact accepted prefix and distinguish backpressure from failure.
- Use the same endpoint with EDP-Serial's optional EDLG frame pump or bounded queue.

The application selects the UART, pins and baud, installs its driver and owns
its lifetime. The provider performs one immediate FIFO write per `TryWrite`;
it creates no driver, worker or queue. Acceptance means copied into the UART
FIFO. Physical transmission, peer receipt and durable storage are separate
outcomes.

## Getting started

This walkthrough builds a native ESP-IDF application that writes `Hello\n`
to an application-selected UART1. It then waits for local TX completion
before releasing the driver. It deliberately limits service attempts and
retains progress if output cannot finish.

### 1. Create a PlatformIO ESP-IDF project

Save this as `platformio.ini`. All library dependencies resolve from GitHub;
configure Git authentication separately if your account requires it.

```ini
[env:esp32dev]
platform = https://github.com/pioarduino/platform-espressif32.git#d41d8383706dc146fd91a0e1d3c59a915a4d493e
board = esp32dev
framework = espidf

build_unflags =
    -std=gnu++11
    -std=gnu++14
    -std=gnu++17
build_flags =
    -std=gnu++20
    -DTUTORIAL_UART_TX_GPIO=17
    -DTUTORIAL_UART_RX_GPIO=16

lib_deps =
    https://github.com/ESPressio-Development-Platform/EDP-Serial-ESP-IDF.git#main
    https://github.com/ESPressio-Development-Platform/EDP-Serial.git#main
    https://github.com/ESPressio-Development-Platform/EDP-System.git#main
```

The platform revision is the qualified ESP32 toolchain selection. Library
`main` references follow ongoing 0.1.0 development; pin reviewed commit SHAs
when your application needs reproducible inputs.

### 2. Reserve the UART and choose pins

GPIO17 TX and GPIO16 RX are illustrative selections for an ESP32-WROOM-class
board. Check your board's wiring and pin restrictions, then change both build
definitions to unused pins. Reserve UART1 and these pins exclusively for this
application. Keep the console on a different UART.

For a compatible 3.3 V UART receiver, connect the selected TX to the receiver's
RX and share ground. Never connect RS-232 voltage levels directly to ESP32
pins. The example configures 115200 baud, eight data bits, no parity, one stop
bit and no hardware flow control.

Install the driver with **TX software ring-buffer size zero**. This is required
by `uart_tx_chars`, the native operation used by this provider. The RX buffer
below is 256 bytes; it does not enable a TX software queue.

### 3. Configure the driver and write a message

Save the complete program below as `src/main.cpp`. Setup belongs to the
application's ESP-IDF entry point. Every native setup result is checked before
constructing the endpoint.

```cpp
#include <array>
#include <cstddef>
#include <cstdio>
#include <span>

#include <ESPressio_Serial_ESP_IDF.hpp>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef TUTORIAL_UART_TX_GPIO
#error "Select an unused UART TX GPIO in the application build flags."
#endif
#ifndef TUTORIAL_UART_RX_GPIO
#error "Select an unused UART RX GPIO in the application build flags."
#endif

/// Application bootstrap; the ESP-IDF framework owns this entry point.
extern "C" void app_main() {
    namespace Serial = ESPressio::Serial;
    constexpr auto port = UART_NUM_1;

    // Do not take over a UART driver owned by another application component.
    if (uart_is_driver_installed(port)) {
        std::printf("UART1 is already owned; setup stopped.\n");
        return;
    }

    uart_config_t config{};
    config.baud_rate = 115200;
    config.data_bits = UART_DATA_8_BITS;
    config.parity = UART_PARITY_DISABLE;
    config.stop_bits = UART_STOP_BITS_1;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config.source_clk = UART_SCLK_DEFAULT;

    auto error = uart_param_config(
        port,
        &config
    );
    if (error != ESP_OK) {
        std::printf(
            "UART configuration failed: %d\n",
            static_cast<int>(error)
        );
        return;
    }
    error = uart_set_pin(
        port,
        TUTORIAL_UART_TX_GPIO,
        TUTORIAL_UART_RX_GPIO,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );
    if (error != ESP_OK) {
        std::printf(
            "UART pin setup failed: %d\n",
            static_cast<int>(error)
        );
        return;
    }
    error = uart_driver_install(
        port,
        256,
        0,       // Required: no TX software ring buffer.
        0,
        nullptr,
        0
    );
    if (error != ESP_OK) {
        std::printf(
            "UART driver installation failed: %d\n",
            static_cast<int>(error)
        );
        return;
    }

    // Static application state survives an incomplete or failed attempt.
    static Serial::ESP_IDF::UartFifoByteStreamWriter endpoint(port);
    static constexpr std::array<std::byte, 6U> message{
        std::byte{'H'}, std::byte{'e'}, std::byte{'l'},
        std::byte{'l'}, std::byte{'o'}, std::byte{'\n'}
    };
    static std::size_t offset{0U};
    static_assert(Serial::ByteWriteEndpoint<decltype(endpoint)>);

    for (unsigned opportunity = 0U; opportunity < 16U; ++opportunity) {
        const auto result = Serial::TryWriteComplete(
            endpoint,
            std::span<const std::byte>(message).subspan(offset)
        );
        offset += result.BytesAccepted;
        if (result.IsComplete()) {
            break;
        }
        if (result.Status != Serial::WriteAttemptStatus::WouldBlock) {
            std::printf(
                "Output failed: status=%u, accepted=%zu; driver retained.\n",
                static_cast<unsigned>(result.Status),
                offset
            );
            return;
        }
        if (opportunity + 1U < 16U) {
            vTaskDelay(1U); // Yield before a later application service opportunity.
        }
    }

    if (offset != message.size()) {
        std::printf(
            "Output pending after %zu bytes; driver retained.\n",
            offset
        );
        return;
    }

    // Full FIFO acceptance still does not mean the last bit has left the UART.
    error = uart_wait_tx_done(
        port,
        pdMS_TO_TICKS(100U)
    );
    if (error != ESP_OK) {
        std::printf(
            "TX drain unconfirmed: %d; driver retained.\n",
            static_cast<int>(error)
        );
        return;
    }
    error = uart_driver_delete(port);
    if (error != ESP_OK) {
        std::printf(
            "UART driver release failed: %d\n",
            static_cast<int>(error)
        );
        return;
    }
    std::printf("Hello accepted; local UART TX completed; driver released.\n");
}
```

Build with `pio run -e esp32dev`. Connect and configure your chosen receiver
before uploading firmware to your own selected board. This program actively
configures the pins you supplied; the repository's maintained demos instead
default to no UART activity.

### 4. Understand partial writes and backpressure

`TryWriteComplete` may make several immediately progressing provider calls.
It returns as soon as the endpoint reports `WouldBlock`, failure or a contract
violation. `BytesAccepted` describes the prefix accepted during that call;
the application adds it to `offset` and supplies only the remaining suffix on
the next service opportunity. Resending the whole message would duplicate bytes.

The example permits at most 16 service opportunities and yields one FreeRTOS
tick between blocked attempts. This is a finite application policy, not a
hard end-to-end timing guarantee. A production application should integrate
the retained state with its existing service context and recovery policy.

If the budget expires, the example reports pending output and retains the
driver, endpoint, message and offset. It does not automatically resume after
`app_main` returns. A longer-lived application owner must continue servicing
that suffix or explicitly reconcile abandonment. On provider failure or a
contract violation, investigate the endpoint before further output; do not
blindly restart the message.

### 5. Drain and release resources deliberately

Only after every byte is accepted does the example call the SDK's finite
`uart_wait_tx_done`. Successful local TX completion allows this exclusively
owned driver to be deleted. If completion is unconfirmed, the application
keeps the driver installed. Neither this wait nor a successful delete proves
peer receipt or durable logging.

Keep UART configuration and driver lifetime stable while any write is in
flight. The endpoint is ordinary-context only; it supplies no synchronization
for competing writers. For complete framed output, use EDP-Serial's
[progressive frame-pump and queue guide](https://github.com/ESPressio-Development-Platform/EDP-Serial/blob/main/README.MD).
Those optional components add dependencies and application-owned service and
synchronization requirements.

## Dependencies

The basic UART walkthrough activates only the mandatory edges. Optional
Serial headers impose the following additional application dependencies;
constructing this provider alone does not activate them.

```text
EDP-Serial-ESP-IDF
├── ESP-IDF esp_driver_uart [mandatory native SDK]
└── EDP-Serial [mandatory byte endpoint]
    ├── EDP-System [mandatory: default byte endpoint and Composition]
    ├── EDP-Logging [optional: import EdlgFramePump.hpp or BoundedEdlgFrameQueue.hpp]
    │   └── EDP-Memory [imposed by Logging's EdlgFrameWriter.hpp]
    │       ├── EDP-System
    │       ├── EDP-Platform
    │       │   └── EDP-System
    │       └── EDP-BoundedTopology
    ├── EDP-Platform-Portable [optional: import BoundedEdlgFrameQueue.hpp]
    │   ├── EDP-Platform
    │   │   └── EDP-System
    │   └── EDP-Memory
    │       ├── EDP-System
    │       ├── EDP-Platform
    │       │   └── EDP-System
    │       └── EDP-BoundedTopology
    ├── EDP-Platform [optional: import BoundedEdlgFrameQueue.hpp]
    │   └── EDP-System
    └── EDP-Threading [optional: import BoundedEdlgFrameQueue.hpp]
        ├── EDP-System
        ├── EDP-Platform
        │   └── EDP-System
        ├── EDP-Clock
        │   ├── EDP-System
        │   └── EDP-Platform
        │       └── EDP-System
        ├── EDP-BoundedTopology
        └── EDP-Memory
            ├── EDP-System
            ├── EDP-Platform
            │   └── EDP-System
            └── EDP-BoundedTopology
```

The queue's Portable header remains a dependency even with a custom copy
provider. The native ESP-IDF component declares
`REQUIRES esp_driver_uart edp_serial`; a direct ESP-IDF CMake integration must
supply an `edp_serial` component exposing EDP-Serial and EDP-System headers.
The PlatformIO recipes resolve these libraries through their GitHub URLs.
See the [complete dependency graph](./DEPENDENCY_GRAPH.MD).

## Documentation and examples

- [Architecture and UART ownership contract](./docs/ARCHITECTURE.MD)
- [Three-target UART demonstration](./demos/UartFifoByteStreamWriter/README.MD): Arduino IDE, PlatformIO Arduino-ESP32 and PlatformIO ESP-IDF
- [Demo index](./demos/README.MD)
- [Developer Wiki](./wiki/Home.md), [public API](./wiki/Public-API.md) and [build/test qualification](./wiki/Build-Test-and-Source.md)
- [Changelog](./CHANGELOG.md) and [license](./LICENSE)

The provider requires a compatible IDF UART API; it is not a generic Arduino
transport. Host tests and target builds have separate scopes from physical
UART qualification. The linked build/test guide records their evidence and
limits.
