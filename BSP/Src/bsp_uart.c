#include    <stdint.h>
#include    "bsp_uart.h"
#include    "usart.h"

static UART_HandleTypeDef *const bsp_uart_handle[BSP_UART_COUNT] = {
    &huart1,
    &huart2
};



int bsp_uart_init(bsp_uartx_t uartx)
{
    if(uartx >= BSP_UART_COUNT )
    {
        return -1;  //参数越界
    }
    else if (HAL_UART_Init(bsp_uart_handle[uartx]) != HAL_OK)
    {
        return -2;  //初始化失败
    }

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
    if(uartx >= BSP_UART_COUNT )
    {
        return -1;  //参数越界
    }

    switch(mode)
    {
        case BSP_UART_MODE_POLLING:
            for(int i=0;i<Size-1;i++)
            {
                if(HAL_UART_Receive(bsp_uart_handle[uartx], &pData[i], 1, Timeout) != HAL_OK)
                {
                    return -2;  //接收失败
                }
                if(pData[i] == '\n')
                {
                    pData[i+1] = '\0';
                    break;
                }
            }
            
            break;
        default:
            return -3;   // 不支持的模式
    }
    return 0;
}