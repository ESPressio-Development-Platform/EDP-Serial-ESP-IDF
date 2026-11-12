# Internal and cross-repository provider seam

This concrete provider subclasses the **EDP-System framework provider** for EDP-Serial's unique `Serial::Domain` and `ByteStreamWriter` capability. `Serial::ByteWriteEndpoint<UartFifoByteStreamWriter>` is an internal cross-repository compatibility assertion: if the class changes its CompositionOffers, noexcept signature or return Type, native IDF application integrations must fail compilation, not silently select another provider.

The underlying native `uart_is_driver_installed` and `uart_tx_chars` calls are **internal SDK integrations** with platform-specific preconditions, not new ESPressio APIs. No shared mutable state is used to invent multiple serial domains; other native IDF and Arduino writers to the same port remain outside this provider's coordination and can interleave unless Bootstrap independently excludes them.
