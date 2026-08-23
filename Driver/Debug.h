#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

/* 日志级别：值越大越详细 */
typedef enum {
    DEBUG_LEVEL_OFF   = 0,   /* 关闭所有日志 */
    DEBUG_LEVEL_ERROR = 1,
    DEBUG_LEVEL_WARN  = 2,
    DEBUG_LEVEL_INFO  = 3,
    DEBUG_LEVEL_DEBUG = 4,
} debug_level_t;


/* 全局日志开关：改这一行即可控制整个工程的日志输出量 */
#define DEBUG_GLOBAL_LEVEL   DEBUG_LEVEL_DEBUG

#define DEBUG_ACTIVE(level)  ((level) <= DEBUG_GLOBAL_LEVEL)

/* 日志宏，用法同 ESP_LOG：LOG_I("TAG", "fmt", ...) */
#define LOG_E(tag, fmt, ...) do { if (DEBUG_ACTIVE(DEBUG_LEVEL_ERROR)) printf("[E][%s] " fmt "\r\n", tag, ##__VA_ARGS__); } while(0)
#define LOG_W(tag, fmt, ...) do { if (DEBUG_ACTIVE(DEBUG_LEVEL_WARN))  printf("[W][%s] " fmt "\r\n", tag, ##__VA_ARGS__); } while(0)
#define LOG_I(tag, fmt, ...) do { if (DEBUG_ACTIVE(DEBUG_LEVEL_INFO))  printf("[I][%s] " fmt "\r\n", tag, ##__VA_ARGS__); } while(0)
#define LOG_D(tag, fmt, ...) do { if (DEBUG_ACTIVE(DEBUG_LEVEL_DEBUG)) printf("[D][%s] " fmt "\r\n", tag, ##__VA_ARGS__); } while(0)



void LOG_init(void);

#endif /* DEBUG_H */
