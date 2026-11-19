# ESP-IDF native UART FIFO ByteStreamWriter — explicitly selected target demo

This example **does not auto-open UART0 or the system console**. It constructs the real `ESPressio::Serial::ESP_IDF::UartFifoByteStreamWriter`, validates EDP-System Composition's exact abstract capability, checks that an invalid port reports ProviderFailure, and exposes a debugger-visible `EdpSerialIdfUartDemoResult` (zero means the selected compile/runtime checks passed). With no macros configured it does **not** install any driver or transmit bytes. This default is safe to compile/run on a shared ESP32 test appliance.

## Three V2 build targets

- `Arduino_IDE/Arduino_IDE.ino` — C++20-capable Arduino-ESP32 **on ESP32 IDF 5.x** with the EDP-Serial and EDP-Serial-ESP-IDF source packages installed; uses the embedded ESP-IDF UART driver, **not** a separate EDP-Serial-Arduino provider.
- `PlatformIO_Arduino` — `pio run -d demos/UartFifoByteStreamWriter/PlatformIO_Arduino`; the Arduino core must export ESP-IDF `driver/uart.h` and `uart_tx_chars`.
- `PlatformIO_ESP-IDF` — `pio run -d demos/UartFifoByteStreamWriter/PlatformIO_ESP-IDF`, using native IDF 5.5.x and esp_driver_uart.

The source is identical except for `setup()/loop()` vs `app_main()`. The package advertises both Arduino-ESP32 on a compatible IDF-backed toolchain and native ESP-IDF; **unsupported Arduino toolchains are not automatically qualified**.

## Explicit optional hardware TX (not enabled by default)

Define **both** `EDP_SERIAL_DEMO_TX_GPIO` and `EDP_SERIAL_DEMO_RX_GPIO` at build time **after** verifying selected physical pins are available and UART1 belongs exclusively to this application. The sample then chooses 115200 8N1, calls the real IDF `uart_param_config`, `uart_set_pin`, `uart_driver_install` with a **zero TX ring** and a bounded 256-byte RX ring, attempts **one** immediate `TryWrite` of the two-byte illustrative payload, and deletes its native driver. It never spins while TX FIFO is full. The exact native result can be `Accepted` or `WouldBlock`; even Accepted means **copied to hardware TX FIFO**, not observed on wire, captured remotely, flushed or durably persisted.

**No pins are supplied or inferred by ESPressio.** Any future genuine loopback/host capture qualification requires an authorised independent pin map/wire setup, as well as timing and ownership tests. Do not flash with this opt-in mode enabled on a shared device until its port/pin ownership and buffering are verified. Package versions remain 0.1.0.

## PlatformIO dependency setup

The two PlatformIO manifests declare GitHub URLs in lib_deps.
Keep this demo's repository-owned source and project assets together,
then run either command from the repository root:

```sh
pio run -d demos/UartFifoByteStreamWriter/PlatformIO_Arduino
pio run -d demos/UartFifoByteStreamWriter/PlatformIO_ESP-IDF
```

PlatformIO downloads the libraries; sibling library checkouts and local
dependency path overrides are unnecessary. Arduino-ESP32 selects the
qualified3.3.12 framework and ESP-IDF5.5 library bundle. Project-owned
partition files and build hooks are resolved within the demo itself.
