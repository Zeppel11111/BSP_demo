#include    <stdint.h>
#include    "MPU6050.h"
#include    "bsp_iic.h"
#include    "i2c.h"
#include    "Debug.h"

static const char* TAG = "MPU6050";

/* 初始化成功后确定的 7 位器件地址（AD0 接地 0x68 / 接高 0x69） */
static uint16_t s_dev_addr = MPU6050_ADDR1;

/* 内部：按给定 7 位地址读 WHO_AM_I，器件应答且身份匹配返回 0 */
static int mpu6050_probe(bsp_iic_t iicx, uint16_t dev_addr)
{
    uint8_t who = 0;

    /* 读 1 字节 WHO_AM_I；寄存器地址 1 字节（8 位） */
    if (bsp_iic_mem_read(iicx, dev_addr, MPU6050_WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &who, 1, 1000) != 0)
    {
        return -1;   /* 总线无应答：器件不在这个地址 */
    }
    if (who != MPU6050_WHO_AM_I_VAL)
    {
        return -2;   /* 有应答但身份不对（应读回 0x68） */
    }
    return 0;
}

int MPU6050_init(bsp_iic_t iicx)
{
    uint8_t reg = 0;

    if (bsp_iic_init(iicx) != 0)
    {
        LOG_E(TAG, "I2C 初始化失败");
        return -1;
    }

    /* 0. 总线诊断：打印 BUSY 与 SCL/SDA 电平，判断是否总线卡死/缺上拉 */
    bsp_iic_bus_diag(iicx);

    /* 1. 探测器件地址：先 AD0 接地(0x68)，无应答再试 AD0 接高(0x69) */
    if (mpu6050_probe(iicx, MPU6050_ADDR1) == 0)
    {
        s_dev_addr = MPU6050_ADDR1;
        LOG_I(TAG, "检测到 MPU6050 @ 0x68");
    }
    else if (mpu6050_probe(iicx, MPU6050_ADDR2) == 0)
    {
        s_dev_addr = MPU6050_ADDR2;
        LOG_I(TAG, "检测到 MPU6050 @ 0x69");
    }
    else
    {
        LOG_E(TAG, "未检测到 MPU6050，请检查 SDA/SCL 接线与上拉电阻");
        return -2;
    }

    /* 2. 唤醒：上电默认 SLEEP=1，必须先清掉，否则读到的数据全是 0 */
    reg = 0x00;   /* SLEEP=0，时钟源=内部 8MHz */
    if (bsp_iic_mem_write(iicx, s_dev_addr, MPU6050_PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &reg, 1, 1000) != 0)
    {
        LOG_E(TAG, "唤醒失败");
        return -3;
    }

    /* 3. 配置量程（默认 ±250°/s、±2g，需要改时替换写入值即可） */
    reg = 0x00;   /* GYRO_CONFIG：FS_SEL=0，±250°/s */
    bsp_iic_mem_write(iicx, s_dev_addr, MPU6050_GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, &reg, 1, 1000);

    reg = 0x00;   /* ACCEL_CONFIG：AFS_SEL=0，±2g */
    bsp_iic_mem_write(iicx, s_dev_addr, MPU6050_ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &reg, 1, 1000);

    LOG_I(TAG, "MPU6050 初始化完成");
    return 0;
}
