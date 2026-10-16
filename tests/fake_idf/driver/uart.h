#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    UART_NUM_0=0,
    UART_NUM_1=1,
    UART_NUM_2=2,
    UART_NUM_MAX=3
} uart_port_t;

bool uart_is_driver_installed(uart_port_t port);
int uart_tx_chars(uart_port_t port,const char* buffer,uint32_t length);
#ifdef __cplusplus
}
#endif
