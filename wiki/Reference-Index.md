# ESP-IDF-backed Serial source reference index

| Production header | Source-level Wiki page and classification |
|---|---|
| `src/ESPressio_Serial_ESP_IDF.hpp` | [Public umbrella](Reference-serial_esp_idf-ESPressio_Serial_ESP_IDF) — PUBLIC aggregation only |
| `src/serial_esp_idf/UartFifoByteStreamWriter.hpp` | [Native UART FIFO provider](Reference-serial_esp_idf-UartFifoByteStreamWriter) — PUBLIC concrete and INTERNAL EDP-System/EDP-Serial provider contract |

These are the **only two maintained C++20 production headers**. CMake/PlatformIO native component wiring is described in [Dependency Contracts](Dependency-Contracts), [Build/Test/Source](Build-Test-and-Source), and [Compiler Definitions](Compiler-Definitions), not redefined as a second framework or logger.
