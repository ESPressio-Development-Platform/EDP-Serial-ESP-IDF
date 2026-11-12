#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include <serial_esp_idf/UartFifoByteStreamWriter.hpp>

namespace {
    bool installed=true;
    int nextReport=0;
    int calls=0;
    int installedCalls=0;
    std::array<char,32U> bytes{};
    uart_port_t lastPort=UART_NUM_0;
}

extern "C" bool uart_is_driver_installed(uart_port_t port) {
    ++installedCalls;
    lastPort=port;
    return installed;
}

extern "C" int uart_tx_chars(uart_port_t port,const char* input,uint32_t length) {
    ++calls;
    lastPort=port;
    if (nextReport>0) {
        const auto copy=nextReport<static_cast<int>(length)
            ? nextReport : static_cast<int>(length);
        std::memcpy(bytes.data(),input,static_cast<std::size_t>(copy));
    }
    return nextReport;
}

int main() {
    namespace Serial=ESPressio::Serial;
    namespace IDF=ESPressio::Serial::ESP_IDF;
    IDF::UartFifoByteStreamWriter endpoint(UART_NUM_1);
    const std::array<std::byte,4U> payload{
        std::byte{'E'},std::byte{'D'},std::byte{'L'},std::byte{'G'}
    };
    installed=false;
    assert(endpoint.TryWrite(payload).Status==Serial::ByteWriteStatus::ProviderFailure);
    assert(calls==0 && installedCalls==1);
    installed=true;
    nextReport=2;
    const auto partial=endpoint.TryWrite(payload);
    assert(partial.Status==Serial::ByteWriteStatus::Accepted);
    assert(partial.BytesAccepted==2U);
    assert(calls==1 && lastPort==UART_NUM_1);
    assert(bytes[0]=='E' && bytes[1]=='D');

    nextReport=0;
    const auto blocked=endpoint.TryWrite(payload);
    assert(blocked.Status==Serial::ByteWriteStatus::WouldBlock);
    assert(blocked.BytesAccepted==0U);

    nextReport=-1;
    assert(endpoint.TryWrite(payload).Status==Serial::ByteWriteStatus::ProviderFailure);
    nextReport=9;
    assert(endpoint.TryWrite(payload).Status==Serial::ByteWriteStatus::ProviderFailure);
    const auto priorCalls=calls;
    assert(endpoint.TryWrite(std::span<const std::byte>{}).Status==
        Serial::ByteWriteStatus::Accepted);
    assert(calls==priorCalls);

    IDF::UartFifoByteStreamWriter invalid(UART_NUM_MAX);
    const auto priorChecks=installedCalls;
    assert(invalid.TryWrite(payload).Status==Serial::ByteWriteStatus::ProviderFailure);
    assert(installedCalls==priorChecks);

    nextReport=4;
    const auto accepted=endpoint.TryWrite(payload);
    assert(accepted.Status==Serial::ByteWriteStatus::Accepted);
    assert(accepted.BytesAccepted==4U);
    assert(bytes[2]=='L' && bytes[3]=='G');

    assert(endpoint.Port()==UART_NUM_1);
}
