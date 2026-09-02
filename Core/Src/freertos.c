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

  /* USART1 = 调试输出（printf，main 里 LOG_init 已初始化），USART2 = 塔石传感器 485 */
  if (TAS_GZ_init(BSP_UART2) != 0)
  {
    /* 传感器不存在或初始化失败：停住，避免空转刷屏 */
    for(;;) { osDelay(1000); }
  }

  /* ESP8266 探活：AT→OK 则驱动内部打印"模块在线"；失败也打印原因，
     但传感器循环继续跑（互不阻塞）。空槽时（CFG=0）桩函数静默返回 -1 */
  esp8266_init(BSP_UART3);

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
    }

    /* 【临时诊断】每轮打印栈剩余字节：hwm 接近 0 说明栈不够，要加大 */
    printf("hwm=%u\n", (unsigned)uxTaskGetStackHighWaterMark(NULL));

    /* vTaskDelayUntil：绝对节拍，每 1000ms 唤醒一次，周期恒定。
       不用 osDelay(1000) 的原因：相对延时会让"执行时间"累积漂移；
       不用计数分频的原因：循环体耗时不可控（Modbus 查询约 46ms），
       次数 ≠ 时间。vTaskDelayUntil 即使某次执行超时，也自动对齐
       到下一个节拍点，不累积误差——这是周期性任务的唯一正解。 */
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
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

