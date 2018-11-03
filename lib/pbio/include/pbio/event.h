
#ifndef _PBIO_EVENT_H_
#define _PBIO_EVENT_H_

#include <stdint.h>

#include "pbio/port.h"
#include "sys/process.h"

/**
 * Contiki process events.
 */
typedef enum {
    PBIO_EVENT_UART_RX,         /**< Character was received on a UART port. *data* is ::pbio_event_uart_rx_data_t. */
} pbio_event_t;

/**
 * Data for ::PBIO_EVENT_UART_RX.
 */
typedef union {
    struct {
        pbio_port_t port;       /**< The port the UART is associated with. */
        uint8_t byte;           /**< The byte received. */
    };
    process_data_t data;        /**< For casting ::pbio_event_uart_rx_data_t to/from ::process_data_t */
} pbio_event_uart_rx_data_t;

#endif // _PBIO_EVENT_H_