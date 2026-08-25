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
#include "MPU6050.h"
#include "attitude.h"
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
  *         唯一任务：MPU6050 采样 + Mahony 姿态解算 + VOFA+ 角度波形发送
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  MPU6050_data_t raw = {0};
  attitude_euler_t euler;
  float gx, gy, gz;    /* 角速度 rad/s */
  float ax, ay, az;    /* 加速度 g */

  attitude_init();

  if (MPU6050_init(BSP_IIC2) != 0)
  {
    /* 传感器不存在或初始化失败：停住，避免空转刷屏 */
    for(;;) { osDelay(1000); }
  }

  /* Infinite loop */
  for(;;)
  {
    if (MPU6050_read_data(&raw) == 0)
    {
      /* 原始值 → 物理单位（量程 ±250°/s、±2g）：
         陀螺 /131 → °/s，再 ×π/180 → rad/s
         加速度 /16384 → g */
      gx = raw.gyro_x  / 131.0f * 0.01745329f;
      gy = raw.gyro_y  / 131.0f * 0.01745329f;
      gz = raw.gyro_z  / 131.0f * 0.01745329f;
      ax = raw.accel_x / 16384.0f;
      ay = raw.accel_y / 16384.0f;
      az = raw.accel_z / 16384.0f;

      attitude_update(gx, gy, gz, ax, ay, az, 0.02f);   /* 50Hz → dt=0.02s */
      attitude_get_euler(&euler);

      /* VOFA+ FireWater：roll,pitch,yaw（度×10，0.1°分辨率）。
         任务不感知插槽状态：attitude 空槽时，头文件空桩让
         get_euler 返回全 0，任务代码无需任何 #if。
         用整数是因为 newlib-nano 未链 _printf_float，%f 会静默输出空 */
      printf("%d,%d,%d\n",
             (int)(euler.roll  * 10.0f),
             (int)(euler.pitch * 10.0f),
             (int)(euler.yaw   * 10.0f));
    }
    osDelay(20);   /* 采样率 50Hz */
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
