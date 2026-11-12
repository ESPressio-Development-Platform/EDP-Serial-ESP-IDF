# EDP-Serial-ESP-IDF developer Wiki

This repository owns the **concrete ESP-IDF native UART TX FIFO** EDP-Serial ByteStreamWriter provider, for ESP32 Arduino cores exposing the compatible IDF driver and native ESP-IDF target projects. It intentionally does not own the abstract EDP-System Composition domain (EDP-Serial does), UART driver install, console, pins, worker scheduling, frame queue, end-to-end host receipt or flash storage. **Version 0.1.0**.

Start with [Architecture](Architecture), [Dependency Contracts](Dependency-Contracts), [Public API](Public-API), [Internal API](Internal-API), [Private Implementation](Private-Implementation), [Composition and Providers](Composition-and-Providers), [Resources and Lifecycle](Resources-and-Lifecycle), [Compiler Definitions](Compiler-Definitions), [Build/Test/Source](Build-Test-and-Source), [Tooling](Tooling-Reference) or the complete [Reference Index](Reference-Index). The implementation reference is [UartFifoByteStreamWriter](Reference-serial_esp_idf-UartFifoByteStreamWriter); its default [three-target demo](../demos/UartFifoByteStreamWriter/README.MD) does not select hardware or emit UART bytes.

A successful compiler link is **not** physical UART loopback/host capture or durability; native FIFO acceptance never guarantees the last bit reached a peer.
