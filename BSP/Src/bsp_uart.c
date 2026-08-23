#include    <stdint.h>
#include    "bsp_uart.h"
#include    "usart.h"

static UART_HandleTypeDef *const bsp_uart_handle[BSP_UART_COUNT] = {
    &huart1,
    &huart2
};

/* 初始化函数表：默认参数来自 CubeMX，BSP 只负责"认领" */
typedef void (*bsp_uart_init_fn_t)(void);
static const bsp_uart_init_fn_t bsp_uart_init_fn[BSP_UART_COUNT] = {
    MX_USART1_UART_Init,
    MX_USART2_UART_Init,
};



int bsp_uart_init(bsp_uartx_t uartx)
{
    if(uartx >= BSP_UART_COUNT )
    {
        return -1;  //参数越界
    }

    bsp_uart_init_fn[uartx]();   // 自动对接：BSP_UART1 → MX_USART1_UART_Init
    return 0;

}



int bsp_uart_send(bsp_uartx_t uartx, uint8_t *pData, uint16_t Size, uint32_t Timeout, bsp_uart_mode_t mode)
{
    if(uartx >= BSP_UART_COUNT )
    {
        return -1;  //参数越界
    }

    switch(mode)
    {
        case BSP_UART_MODE_POLLING:
            if(HAL_UART_Transmit(bsp_uart_handle[uartx], pData, Size, Timeout) != HAL_OK)
            {
                return -2;  //发送失败
            }
            break;
        default:
                return -3;   // 不支持的模式
    }
    return 0;
}

int bsp_uart_rec_line(bsp_uartx_t uartx, uint8_t *pData,uint16_t Size, uint32_t Timeout, bsp_uart_mode_t mode)
{
    uint16_t i;

    if(uartx >= BSP_UART_COUNT )
    {
        return -1;  //参数越界
    }

    switch(mode)
    {
        case BSP_UART_MODE_POLLING:
            for(i = 0; i < Size - 1; i++)
            {
                if(HAL_UART_Receive(bsp_uart_handle[uartx], &pData[i], 1, Timeout) != HAL_OK)
                {
                    pData[i] = '\0';           // 超时收尾，防止脏数据
                    return -2;  //接收失败
                }
                if(pData[i] == '\n')
                {
                    if(i > 0 && pData[i-1] == '\r')  // 剥掉 \r
                    {
                        i--;
                    }
                    pData[i] = '\0';
                    return 0;
                }
            }
            pData[Size - 1] = '\0';            // 收满还没换行，强制收尾防越界
            return 0;
        default:
            return -3;   // 不支持的模式
    }
}
