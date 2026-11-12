# Private native adapter implementation

The only retained private member is `uart_port_t _port`: one selected native port identifier, initialized in the explicit constructor and immutable for the object's live usage. The class borrows the external native driver; it does not store an IRQ handle, TX buffer pointer, RAM queue, GPIO pin, mutable counters, runtime Clock or persistent resource.

`TryWrite` uses temporary `offered` (the input byte count saturated to uint32 maximum), then calls native `uart_tx_chars` once if port/driver/span checks pass. A negative or out-of-range native count is ProviderFailure; positive count within offered is Accepted with truthful prefix; zero is WouldBlock. No copying loops in ESPressio, no hidden retry, TX flush, timeout or memory beyond the call. Reinterpreting a nonempty `std::byte` span as the native `char` pointer is valid only for the lifetime of that *synchronous* native call; native UART must copy the bytes into hardware FIFO before returning.

Because the IDF driver may have a TX software ring buffer, Bootstrap must install it with **TX ring=0** for `uart_tx_chars` legality. This invariant is **outside** the C++ Type system and requires real target/provider configuration review. A fake UART host test that accepts a scripted count cannot establish it on a board.
