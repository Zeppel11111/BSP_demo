#ifndef     BSP_UART_H
#define     BSP_UART_H

#include    <stdint.h>

typedef enum
{
    BSP_UART1=0,
    BSP_UART2,
    BSP_UART_COUNT      // 做边界

} bsp_uartx_t;

typedef enum
{
    BSP_UART_MODE_POLLING=0,

} bsp_uart_mode_t;

int bsp_uart_init(bsp_uartx_t uartx);
int bsp_uart_send(bsp_uartx_t uartx, uint8_t *pData, uint16_t Size, uint32_t Timeout, bsp_uart_mode_t mode);
int bsp_uart_rec_line(bsp_uartx_t uartx, uint8_t *pData, uint16_t Size, uint32_t Timeout, bsp_uart_mode_t mode);

#endif