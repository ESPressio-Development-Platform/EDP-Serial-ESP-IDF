#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <ESPressio_Serial.hpp>
#include <driver/uart.h>

namespace ESPressio::Serial::ESP_IDF {

    /// Concrete Bootstrap-configured ordinary-context TX FIFO provider.
    /// uart_tx_chars must be used only when uart_driver_install's TX buffer
    /// size is ZERO; otherwise the IDF API does not permit this call.
    class UartFifoByteStreamWriter final : public Framework::Provider<
        ESPressio::Serial::Domain,
        Framework::Offers<
            Framework::Offer<ESPressio::Serial::ByteStreamWriter>
        >
    > {
        uart_port_t _port;

    public:
        explicit UartFifoByteStreamWriter(uart_port_t selectedPort) noexcept :
            _port(selectedPort) {}

        UartFifoByteStreamWriter(const UartFifoByteStreamWriter&) = delete;
        UartFifoByteStreamWriter& operator=(const UartFifoByteStreamWriter&) = delete;
        UartFifoByteStreamWriter(UartFifoByteStreamWriter&&) = delete;
        UartFifoByteStreamWriter& operator=(UartFifoByteStreamWriter&&) = delete;

        /// Exactly one nonblocking native FIFO push; never wait for TX drain.
        /// Accepted bytes have entered the FIFO, NOT reached the wire/host.
        [[nodiscard]] ESPressio::Serial::ByteWriteResult TryWrite(
            std::span<const std::byte> bytes
        ) noexcept {
            if (static_cast<int>(_port) < 0 ||
                static_cast<int>(_port) >= static_cast<int>(UART_NUM_MAX)) {
                return {ByteWriteStatus::ProviderFailure,0U};
            }
            if (!uart_is_driver_installed(_port)) {
                return {ByteWriteStatus::ProviderFailure,0U};
            }
            if (bytes.empty()) {
                return {ByteWriteStatus::Accepted,0U};
            }
            if (bytes.data() == nullptr) {
                return {ByteWriteStatus::ProviderFailure,0U};
            }

            const auto offered = bytes.size() >
                static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())
                ? std::numeric_limits<std::uint32_t>::max()
                : static_cast<std::uint32_t>(bytes.size());

            const int reported = uart_tx_chars(
                _port,
                reinterpret_cast<const char*>(bytes.data()),
                offered
            );

            if (reported < 0 ||
                static_cast<std::uint64_t>(reported) >
                    static_cast<std::uint64_t>(offered)) {
                return {ByteWriteStatus::ProviderFailure,0U};
            }
            if (reported == 0) {
                return {ByteWriteStatus::WouldBlock,0U};
            }
            return {ByteWriteStatus::Accepted,static_cast<std::size_t>(reported)};
        }

        [[nodiscard]] uart_port_t Port() const noexcept { return _port; }
    };

    static_assert(
        ESPressio::Serial::ByteWriteEndpoint<UartFifoByteStreamWriter>,
        "ESP-IDF UART provider must fulfil EDP-Serial's abstract byte contract"
    );

} // namespace ESPressio::Serial::ESP_IDF
