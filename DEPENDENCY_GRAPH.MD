# EDP-Serial-ESP-IDF Dependency Graph

Package version **0.1.0**. The default package resolves EDP-Serial from `main`.

```text
EDP-Serial-ESP-IDF
├── ESP-IDF esp_driver_uart [mandatory native SDK]
└── EDP-Serial [mandatory byte endpoint]
    ├── EDP-System [mandatory: default byte endpoint and Composition]
    ├── EDP-Logging [optional: import EdlgFramePump.hpp or BoundedEdlgFrameQueue.hpp]
    │   └── EDP-Memory [imposed by Logging's EdlgFrameWriter.hpp]
    │       ├── EDP-System
    │       ├── EDP-Platform
    │       │   └── EDP-System
    │       └── EDP-BoundedTopology
    ├── EDP-Platform-Portable [optional: import BoundedEdlgFrameQueue.hpp]
    │   ├── EDP-Platform
    │   │   └── EDP-System
    │   └── EDP-Memory
    │       ├── EDP-System
    │       ├── EDP-Platform
    │       │   └── EDP-System
    │       └── EDP-BoundedTopology
    ├── EDP-Platform [optional: import BoundedEdlgFrameQueue.hpp]
    │   └── EDP-System
    └── EDP-Threading [optional: import BoundedEdlgFrameQueue.hpp]
        ├── EDP-System
        ├── EDP-Platform
        │   └── EDP-System
        ├── EDP-Clock
        │   ├── EDP-System
        │   └── EDP-Platform
        │       └── EDP-System
        ├── EDP-BoundedTopology
        └── EDP-Memory
            ├── EDP-System
            ├── EDP-Platform
            │   └── EDP-System
            └── EDP-BoundedTopology
```

This is a platform-specific **concrete consumer of abstract EDP-Serial**. EDP-Serial and EDP-Logging must NEVER include or directly depend on EDP-Serial-ESP-IDF. No reverse dependency or cycle; no legacy `ESPressio-*` source.

**IDF CMake:** this component declares `REQUIRES esp_driver_uart edp_serial`; the consuming application provides an `edp_serial` component exporting the EDP-Serial public include path and EDP-System provider contracts. The PlatformIO metadata resolves its direct EDP-Serial dependency from `main` and allows **only IDF-backed ESP32 Arduino and native ESP-IDF targets with the real esp_driver_uart**; this is not a generic Arduino backend. The application Bootstrap selects and configures UART port/pins/driver (TX buffer size=0), then binds `UartFifoByteStreamWriter` as the exclusive `Serial::ByteStreamWriter` provider. This library does not import EDP-Platform-ESP-IDF or Platform concrete at the abstraction layer.

**Optional separately activated app graph:** Importing Serial's EdlgFramePump.hpp imposes Logging and its Memory closure. Importing BoundedEdlgFrameQueue.hpp additionally imposes Platform-Portable (including Memory), Platform and Threading with the complete closure above.  `EDP-Serial::BoundedEdlgFrameQueue.hpp` adds EDP-Logging, EDP-Threading and EDP-Platform direct abstract dependencies as described in EDP-Serial DEPENDENCY_GRAPH.MD; they are NOT auto-imported by this provider. Do not assume hidden dependency resolution.

**Target limits:** `esp_driver_uart` is present in ESP-IDF v5.5.5 installed toolchain. Real hardware output, target variant coverage, native driver install and console sharing require separate HIL qualification.

## PlatformIO demo source resolution

The demo manifests express their selected direct and supporting owner
closure through explicit GitHub lib_deps URLs. The library's production
dependency activation above is unchanged. Project-local source, partition
files and framework setup hooks are application assets; dependencies are
downloaded by PlatformIO.
