#include "bsp_uart.h"
#include "debug.h"

#define DEBUG_PORT BSP_UART1

void LOG_init(void)
{
    /* 初始化串口 */
    bsp_uart_init(DEBUG_PORT) ;
}

/* 实现 weak 符号 __io_putchar：把每个字符从 USART1 发出去 */
int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;

    /* 换行自动补 \r，适配 Windows 串口助手 */
    if (ch == '\n')
    {
        uint8_t cr = '\r';
        bsp_uart_send(DEBUG_PORT, &cr, 1, 0xFFFFFFFF, BSP_UART_MODE_POLLING);
    }
    bsp_uart_send(DEBUG_PORT, &c, 1, 0xFFFFFFFF, BSP_UART_MODE_POLLING);
    return ch;
}
