/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "app_config.h"   /* 模块总开关：必须先于各模块头文件 */
#include "Debug.h"
#include "TAS_GZ.h"
#include "ESP8266.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 【部署配置】TCP 接收服务器：电脑上跑 server.py 监听 9000，板子连这里。
   服务器 IP 随网络环境变化，两种常用场景：
     电脑开"移动热点"给板子连 → 电脑固定是 192.168.137.1（查 ipconfig 里
     网段 192.168.137.1 那张"无线网络连接* N"网卡）
     iPhone 开热点            → 电脑是 172.20.10.2
   换环境改这里一行即可，端口不变。 */
#define NET_SERVER_IP     "192.168.137.1"
#define NET_SERVER_PORT   9000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  /* 栈 2048B：TAS_GZ_read 单帧 ~300B + printf ~250B 峰值已用 ~900B，
     实测 1024B 只剩 107B 余量；后续加 ESP8266 AT 缓冲/JSON 拼串还要 ~400B，
     留足余量。改这里要同步改 .ioc 的 Tasks01 栈参数 */
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  *         唯一任务：塔石传感器（温湿度光照）定时采集 + 串口输出
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  TAS_GZ_data_t tas = {0};
  TickType_t xLastWakeTime;          /* 绝对节拍基准 */
  int tcp_ok = 0;                    /* TCP 连接状态：0=未连/已断，1=在线 */

  /* USART1 = 调试输出（printf，main 里 LOG_init 已初始化），USART2 = 塔石传感器 485 */
  if (TAS_GZ_init(BSP_UART2) != 0)
  {
    /* 传感器不存在或初始化失败：停住，避免空转刷屏 */
    for(;;) { osDelay(1000); }
  }

  /* ESP8266 探活：AT→OK 则驱动内部打印"模块在线"；失败也打印原因，
     但传感器循环继续跑（互不阻塞）。空槽时（CFG=0）桩函数静默返回 -1。
     模块与板子同时上电时需 ~1s 引导，首次 AT 可能撞上引导期 →
     失败则每秒重试探活，最多 3 次，避免偶发"无响应"误报 */
  {
    int esp_ok = (esp8266_init(BSP_UART3) == 0);
    for (int i = 0; !esp_ok && i < 3; i++)
    {
      osDelay(1000);
      esp_ok = (esp8266_at_test() == 0);
      if (esp_ok)
      {
        printf("[I][ESP8266] AT 握手成功（引导重试后）\n");
      }
    }
    if (!esp_ok)
    {
      printf("[E][ESP8266] 模块持续无响应，检查 TX/RX 交叉/CH_PD/供电\n");
    }
  }

  /* 连热点（模式②：扫描凭据表自动连）。启动时最多阻塞约几秒（扫描+连接），
     只连一次；断线重连逻辑后续再加。连上后查 IP 打印——看到 IP 即铁证 */
  {
    char ip[32] = "?";
    if (esp8266_auto_join() != 0)
    {
      printf("[E][ESP8266] 热点连接失败（查凭据表/热点范围）\n");
    }
    else if (esp8266_get_ip(ip, sizeof(ip)) != 0)
    {
      printf("[I][ESP8266] WiFi 已连接（IP 查询失败）\n");
    }
    else
    {
      printf("[I][ESP8266] WiFi 已连接 IP=%s\n", ip);
    }
  }

  /* TCP 建连（电脑 TcpCom 需已监听 9000）。失败不阻塞：循环里每轮自动重试 */
  tcp_ok = (esp8266_tcp_connect(NET_SERVER_IP, NET_SERVER_PORT) == 0);
  if (tcp_ok)
  {
    printf("[I][ESP8266] TCP 已连接 %s:%d\n", NET_SERVER_IP, NET_SERVER_PORT);
  }
  else
  {
    printf("[E][ESP8266] TCP 连接失败（先开 TcpCom 监听 9000，将每轮重试）\n");
  }

  /* 绝对节拍初始化：以当前 tick 为基准点 */
  xLastWakeTime = xTaskGetTickCount();

  /* Infinite loop */
  for(;;)
  {
    if (TAS_GZ_read(&tas) == 0)
    {
      /* VOFA+ 三通道：温度(0.1℃), 湿度(0.1%), 光照(LUX)。
         任务不感知插槽状态：TAS_GZ 空槽时，头文件空桩让
         read 返回 -1，任务代码无需任何 #if。
         用整数是因为 newlib-nano 未链 _printf_float，%f 会静默输出空 */
      printf("%d,%d,%d\n",
             (int)tas.temp_x10,
             (int)tas.humi_x10,
             (int)tas.lux);

      /* 上行：拼一行数据经 WiFi 发给服务器（行格式 = 数据契约，服务器按 \n 分行：
         温度x10,湿度x10,光照\n ；末尾 \n 是行结束符，服务器靠它切行）。
         断线自动重连：发送失败置 tcp_ok=0，下一轮先重连再发 */
      {
        char uplink[32];
        int  n = snprintf(uplink, sizeof(uplink), "%d,%d,%d\n",
                          (int)tas.temp_x10, (int)tas.humi_x10, (int)tas.lux);
        if (!tcp_ok)
        {
          tcp_ok = (esp8266_tcp_connect(NET_SERVER_IP, NET_SERVER_PORT) == 0);
        }
        if (tcp_ok && esp8266_send_data((uint8_t*)uplink, (uint16_t)n) != 0)
        {
          tcp_ok = 0;   /* 连接断了，下轮重连 */
          printf("[E][ESP8266] 发送失败，准备重连\n");
        }
      }
    }

    /* vTaskDelayUntil：绝对节拍，上报周期 = 30 秒（想改间隔只改这里）。
       每 30 秒：读一次传感器 → 串口打印 → WiFi 上行一行。
       原为 1000ms（调试期看实时曲线），植物监测 30s 已足够：
       数据量从 8.6 万条/天 降到 2880 条/天，cpolar 免费流量也省。
       不用 osDelay 的原因：相对延时会让"执行时间"累积漂移；
       vTaskDelayUntil 即使某次执行超时，也自动对齐到下一个节拍点，
       不累积误差——这是周期性任务的唯一正解。 */
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30000));
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* 栈溢出钩子：configCHECK_FOR_STACK_OVERFLOW=2 时，内核检测到任务栈
 * 被踩穿会调到这里。打印任务名后停住，便于立刻发现（正常不应触发） */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  taskDISABLE_INTERRUPTS();
  printf("[FATAL] 栈溢出: %s\r\n", pcTaskName ? pcTaskName : "?");
  for(;;);
}

/* USER CODE END Application */

