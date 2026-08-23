#include "bsp_uart.h"
#include "debug.h"

/* 实现 weak 符号 __io_putchar：把每个字符从 USART1 发出去 */
int __io_putchar(int ch)
{
    uint8_t c = (uint8_t)ch;

    /* 换行自动补 \r，适配 Windows 串口助手 */
    if (ch == '\n')
    {
        uint8_t cr = '\r';
        bsp_uart_send(BSP_UART1, &cr, 1, 0xFFFFFFFF, BSP_UART_MODE_POLLING);
    }
    bsp_uart_send(BSP_UART1, &c, 1, 0xFFFFFFFF, BSP_UART_MODE_POLLING);
    return ch;
}
