#include    <stdint.h>
#include    <string.h>
#include    "ESP8266.h"
#include    "bsp_uart.h"
#include    "Debug.h"

/* =====================================================================
 * 模块总开关：CFG_ENABLE_ESP8266 = 0 时整个模块不编译，
 * 调用点靠 ESP8266.h 里的裁剪桩编译通过。
 * 分层约束：本文件只允许调用 BSP 接口（bsp_uart_*），
 * 不直接触碰 HAL 句柄——USART3 波特率 115200 在 CubeMX(.ioc) 配置。
 * ===================================================================== */
#if CFG_ENABLE_ESP8266

static const char* TAG = "ESP8266";

/* ---- 器件状态：记录挂在哪路串口 ---- */
static bsp_uartx_t s_uartx = BSP_UART_COUNT;   /* 哨兵值：esp8266_init 前表示未指定 */

/* 接收分片粒度：等一行响应最多阻塞 10ms，不卡任务调度（与 BSP 切片一致） */
#define   ESP8266_POLL_SLICE_MS   10

int esp8266_init(bsp_uartx_t uartx)
{
    s_uartx = uartx;

    if (bsp_uart_init(s_uartx) != 0)
    {
        LOG_E(TAG, "串口初始化失败");
        return -1;
    }
    LOG_I(TAG, "USART3 已初始化（115200 8N1）");

    /* AT 自检：模块必须回 OK，否则查 TX/RX 交叉、CH_PD 拉高、供电 */
    if (esp8266_at_test() != 0)
    {
        LOG_E(TAG, "AT 无响应（检查 TX/RX 交叉、CH_PD/RST 拉高、供电）");
        return -2;
    }
    LOG_I(TAG, "AT 握手成功，模块在线");
    return 0;
}

int esp8266_at_test(void)
{
    return esp8266_cmd("AT", "OK", 1000);
}

int esp8266_cmd(const char *cmd, const char *expect, uint32_t timeout_ms)
{
    char     line[ESP8266_RX_LINE_MAX];
    uint32_t waited = 0;

    if (cmd == NULL || expect == NULL)      return -1;   /* 参数错 */
    if (s_uartx >= BSP_UART_COUNT)          return -2;   /* 未初始化 */

    /* 发指令：cmd + \r\n（AT 指令必须回车结尾，由驱动统一补） */
    if (bsp_uart_send(s_uartx, (uint8_t*)cmd, strlen(cmd), 100,
                      BSP_UART_MODE_POLLING) != 0 ||
        bsp_uart_send(s_uartx, (uint8_t*)"\r\n", 2, 100,
                      BSP_UART_MODE_POLLING) != 0)
    {
        return -3;                                        /* 发送失败 */
    }

    /* 逐行收响应，在行里找关键字；分片等待保证 RTOS 友好 */
    while (waited < timeout_ms)
    {
        if (bsp_uart_rec_line(s_uartx, (uint8_t*)line, sizeof(line),
                              ESP8266_POLL_SLICE_MS, BSP_UART_MODE_POLLING) == 0)
        {
            if (strstr(line, expect) != NULL)  return 0;      /* 等到预期关键字 */
            if (strstr(line, "ERROR") != NULL ||
                strstr(line, "FAIL")  != NULL) return -4;     /* 模块明确报错，提前返回 */
        }
        waited += ESP8266_POLL_SLICE_MS;   /* 没等到：继续收下一行，直到预算耗尽 */
    }
    return -5;                             /* 超时 */
}

#endif /* CFG_ENABLE_ESP8266 */
