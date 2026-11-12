#include <Arduino.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <driver/uart.h>
#include <ESPressio_Serial_ESP_IDF.hpp>

namespace Demo {

    namespace Serial = ESPressio::Serial;

    // Safe default: never install a driver, select pins or drive a UART.
    // Physical TX is compiled only if BOTH GPIO macros are explicitly chosen.
    [[nodiscard]] int Run() noexcept {
        using Serial::ESP_IDF::UartFifoByteStreamWriter;
        static_assert(Serial::ByteWriteEndpoint<UartFifoByteStreamWriter>);

        const std::array<std::byte, 2U> bytes{
            std::byte{'O'}, std::byte{'K'}
        };

        UartFifoByteStreamWriter invalid(UART_NUM_MAX);
        if (invalid.TryWrite(bytes).Status !=
            Serial::ByteWriteStatus::ProviderFailure) {
            return 1;
        }

        constexpr uart_port_t port = UART_NUM_1;
        UartFifoByteStreamWriter writer(port);
        if (writer.Port() != port) {
            return 2;
        }

#if defined(EDP_SERIAL_DEMO_TX_GPIO) && defined(EDP_SERIAL_DEMO_RX_GPIO)
        // Opt-in only: caller must ensure UART1 and GPIOs do not belong to the
        // console, another native task, or another Arduino HardwareSerial.
        uart_config_t config{};
        config.baud_rate = 115200;
        config.data_bits = UART_DATA_8_BITS;
        config.parity = UART_PARITY_DISABLE;
        config.stop_bits = UART_STOP_BITS_1;
        config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        config.source_clk = UART_SCLK_DEFAULT;

        if (uart_param_config(port, &config) != ESP_OK) {
            return 3;
        }
        if (uart_set_pin(
            port, EDP_SERIAL_DEMO_TX_GPIO, EDP_SERIAL_DEMO_RX_GPIO,
            UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE
        ) != ESP_OK) {
            return 4;
        }
        // TX ring MUST be zero for uart_tx_chars; RX storage remains native.
        if (uart_driver_install(port, 256, 0, 0, nullptr, 0) != ESP_OK) {
            return 5;
        }
        const auto attempt = writer.TryWrite(bytes);
        const bool validOutcome =
            attempt.Status == Serial::ByteWriteStatus::Accepted ||
            attempt.Status == Serial::ByteWriteStatus::WouldBlock;
        const auto removed = uart_driver_delete(port);
        return validOutcome && removed == ESP_OK ? 0 : 6;
#else
        // A compile-only provider Contract demonstration: NOT wire delivery.
        return 0;
#endif
    }

} // namespace Demo

/// Debug-visible zero-on-success demonstration result.
volatile int EdpSerialIdfUartDemoResult{0};

void setup() {
    EdpSerialIdfUartDemoResult = Demo::Run();
}

void loop() {}
