#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
    #define LOG_INFO(fmt, ...) printf("[INFO] " fmt, ## __VA_ARGS__)
#else
    #define LOG_INFO(...)
#endif

#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR]" fmt, ## __VA_ARGS__)

#endif
