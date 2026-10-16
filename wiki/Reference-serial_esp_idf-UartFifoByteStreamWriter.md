# serial_esp_idf/UartFifoByteStreamWriter.hpp

**PUBLIC concrete ESP-IDF provider**, version `0.1.0`. Target requires `driver/uart.h` from `esp_driver_uart` and the abstract EDP-Serial/E-System source roots.

- `Serial::ESP_IDF::UartFifoByteStreamWriter(uart_port_t port) noexcept`: explicit port chosen by application's Bootstrap; no silent hardware configure/install.
- Noncopyable/nonmovable type deriving EDP-System `Framework::Provider<Serial::Domain,Offers<Offer<Serial::ByteStreamWriter>>>`; `Serial::ByteWriteEndpoint` compile-time contract is asserted.
- `TryWrite(std::span<const std::byte>) noexcept -> ByteWriteResult`: validates native port and `uart_is_driver_installed`; issues at most one `uart_tx_chars` to push into free HW TX FIFO (must have TX ring buffer size zero). Zero available => WouldBlock; successful positive count => Accepted; negative/out-of-range => ProviderFailure. No borrowed pointer retained. No delay/polling/wait and no cross-frame native serialization.
- `Port() const noexcept`: selected `uart_port_t` for Bootstrap's explicit composition validation.

**Only ordinary context.** Firmware wire completion, peer receipt, archive durability, TX-ready wake and power-loss handling are NOT provided. Separate EDP-Serial composed FIFO/arbitration controls writer concurrency; uncoordinated native console writers can still interleave with it. Host tests use a fake `uart.h`; real ESP-IDF 5.5.5 ESP32 firmware ELF link PASSED RPI400 run **20261016T090824-aed310a8**; physical output and on-device timing still require separate validation.

[README](../README.MD) • [Architecture](../docs/ARCHITECTURE.MD) • [Dependency Graph](../DEPENDENCY_GRAPH.MD).

**Additional target builds:** ESP32-S3 ESP-IDF v5.5.5 full ELF link PASS `20261016T091324-d99b12b1`; integrated ESP32 canonical EDLG frame→Serial FIFO→real UART provider ELF link PASS `20261016T091604-f40040e3`. Both compiled without flashing any board; the integrated fixture uses a synthetic (non-production) Threading mutex.
