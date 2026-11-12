# EDP-Serial-ESP-IDF Dependency Graph

**Current source branch: feature/logging-v1** until guarded main promotion, package version **0.1.0**.

```text
EDP-Serial-ESP-IDF [ESP-IDF target-specific owner]
├── EDP-Serial 0.1.0 [mandatory abstract capability/ByteWriteResult]
│   └── EDP-System 0.1.0 [mandatory Composition]
└── ESP-IDF esp_driver_uart [target-native SDK mandatory]
```

This is a platform-specific **concrete consumer of abstract EDP-Serial**. EDP-Serial and EDP-Logging must NEVER include or directly depend on EDP-Serial-ESP-IDF. No reverse dependency or cycle; no legacy `ESPressio-*` source.

**IDF CMake:** this component declares `REQUIRES esp_driver_uart edp_serial`; the consuming application provides an `edp_serial` component exporting the EDP-Serial public include path and EDP-System provider contracts. The PlatformIO metadata declares EDP-Serial as a direct dependency and allows **only IDF-backed ESP32 Arduino and native ESP-IDF targets with the real esp_driver_uart**; this is not a generic Arduino backend. The application Bootstrap selects and configures UART port/pins/driver (TX buffer size=0), then binds `UartFifoByteStreamWriter` as the exclusive `Serial::ByteStreamWriter` provider. This library does not import EDP-Platform-ESP-IDF or Platform concrete at the abstraction layer.

**Optional separately activated app graph:** `EDP-Serial::BoundedEdlgFrameQueue.hpp` adds EDP-Logging and EDP-Threading direct abstract dependencies as described in EDP-Serial DEPENDENCY_GRAPH.MD; they are NOT auto-imported by this provider. Do not assume hidden dependency resolution.

**Target limits:** `esp_driver_uart` is present in ESP-IDF v5.5.5 installed toolchain. Real hardware output, target variant coverage, native driver install and console sharing require separate HIL qualification.
