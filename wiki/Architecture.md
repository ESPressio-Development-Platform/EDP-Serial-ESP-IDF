# ESP32 native UART TX architecture

This EDP-Serial-ESP-IDF repository contains one **concrete** C++20 `UartFifoByteStreamWriter` adapter over the ESP-IDF `esp_driver_uart` subsystem, paired with the existing platform-neutral EDP-Serial/E-System ByteStreamWriter provider offer. The provider is a borrowed, stateless-in-output native port identity; the application owns all UART driver install/configuration, TX/ RX pins, baud, hardware lifecycle, console exclusion and scheduling.

The adapter validates that `uart_port_t` is within [0,`UART_NUM_MAX`) and `uart_is_driver_installed`; then `TryWrite` uses exactly one `uart_tx_chars` call bounded to uint32_t offered bytes. Native success means a prefix was accepted into the **hardware TX FIFO**, not that the last byte left the UART wire. Native zero means WouldBlock. No retry, waiting or buffer lifetime beyond call is introduced. The ESP-IDF requirement **TX driver ring buffer = 0** is a concrete invariant for `uart_tx_chars`.

No Logging source imports this platform concrete. Optional EDLG framing and FIFO come from EDP-Serial's separately activated headers and its EDP-Threading keyed Mutex; they do not create a real Threading Mutex in this provider. See [normative architecture](../docs/ARCHITECTURE.MD) and [source reference](Reference-serial_esp_idf-UartFifoByteStreamWriter).
