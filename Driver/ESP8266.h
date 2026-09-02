#ifndef     ESP8266_H
#define     ESP8266_H

#include    <stdint.h>
#include    "bsp_uart.h"
#include    "app_config.h"     /* 模块总开关：必须先于本文件包含 */

/* =====================================================================
 * ESP8266（ESP-01S）WiFi 模块驱动 —— AT 指令固件
 *
 * 器件卡插槽：CFG_ENABLE_ESP8266 = 1 编译真驱动，= 0 时空桩。
 * 接线：USART3 TX(PB10)→模块 RXD，RX(PB11)←模块 TXD；
 *       VCC=3.3V、CH_PD/RST 拉高、GPIO0 悬空。
 * 波特率：115200 8N1（AT 固件默认，已在 CubeMX(.ioc) 配好 USART3）
 * 固件：安信可 v1.5.4.1-a（老 AT 固件，无 MQTT 指令 → 上云走 TCP）
 * 分层约束：驱动只调 BSP 接口，不触碰 HAL 句柄（无穿透）
 * ===================================================================== */

#define   ESP8266_RX_LINE_MAX   128    /* 单行 AT 响应最大长度（含 \0） */

#if CFG_ENABLE_ESP8266

/* 初始化：认领串口 + AT 自检（AT→OK）；成功返回 0，-1 串口失败，-2 AT 自检失败 */
int  esp8266_init(bsp_uartx_t uartx);

/* 模块探活：AT → OK；成功返回 0 */
int  esp8266_at_test(void);

/* 核心原语：发一条 AT 指令（自动补 \r\n），等响应行里出现 expect 关键字。
 * 成功返回 0；失败为负：
 *   -1 参数错    -2 未初始化    -3 发送失败
 *   -4 模块明确回 ERROR/FAIL    -5 超时未等到 expect      */
int  esp8266_cmd(const char *cmd, const char *expect, uint32_t timeout_ms);

#else
/* ===== 裁剪桩：开关=0 时模块不编译，调用点无需 #if，直接编成失败 ===== */
static inline int esp8266_init(bsp_uartx_t uartx)                     { (void)uartx; return -1; }
static inline int esp8266_at_test(void)                               { return -1; }
static inline int esp8266_cmd(const char *cmd, const char *expect,
                              uint32_t timeout_ms)                    { (void)cmd; (void)expect; (void)timeout_ms; return -1; }
#endif /* CFG_ENABLE_ESP8266 */

#endif
