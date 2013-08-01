#ifdef DEBUG
    #define LOG_INFO(fmt, ...) fprintf(stdout, "[INFO]" fmt, __VA_ARGS__)
#else
    #define LOG_INFO(...)
#endif

#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR]" fmt, __VA_ARGS__)