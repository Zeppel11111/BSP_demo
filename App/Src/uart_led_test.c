#include    <string.h>
#include    "uart_led_test.h"
#include    "bsp_uart.h"
#include    "Board_LED.h"
#include    "cmsis_os.h"
#include    "Debug.h"

const char* TAG = "uart_led_test";
char  rec_data[rec_data_size];

void uart_led_test(void)
{

    
    LOG_D(TAG,"Hello This is uart_led_test\r\n");
    osDelay(1000);  
    LOG_D(TAG,"可以开始交互了\r\n");
    while(1)
    {
        if(bsp_uart_rec_line(BSP_UART1, (uint8_t*)rec_data, rec_data_size, 1000, BSP_UART_MODE_POLLING) == 0)
        {
            LOG_D(TAG,"收到数据： %s\r\n", rec_data);
        
            if (strstr(rec_data, "led_on") != NULL) 
            {
                led_on();
            }   
            else if(strstr(rec_data, "led_off") != NULL)
            {
                led_off();
            }
        }
    }

}