#include <stdinc.h>

#ifndef NDEBUG
    #define LOG_DEBUG(mondrian_instance, msg, ...)                                                                     \
    ({                                                                                                                 \
        if (mondrian_config(mondrian_instance)->log.debug) {                                                           \
            fprintf(stderr, "# [DEBUG]: ");                                                                            \
            fprintf(stderr, msg, __VA_ARGS__);                                                                         \
            fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                                                         \
        }      else printf("Nope\n");                                                                                                         \
    })
#else
    #define LOG_DEBUG(msg, ...) { }
#endif

#ifndef NWARN
    #define LOG_WARN(mondrian_instance, msg, ...)                                                                      \
    ({                                                                                                                 \
        if (mondrian_config(mondrian_instance)->log.warn) {                                                            \
            fprintf(stderr, "# [WARN]: ");                                                                             \
            fprintf(stderr, msg, __VA_ARGS__);                                                                         \
            fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                                                         \
        }                                                                                                              \
    })
#else
    #define LOG_WARN(msg, ...) { }
#endif

#ifndef NERROR
#define LOG_ERROR(mondrian_instance, msg, ...)                                                                         \
    ({                                                                                                                 \
        if (mondrian_config(mondrian_instance)->log.error) {                                                           \
            fprintf(stderr, "# [ERROR]: ");                                                                            \
            fprintf(stderr, msg, __VA_ARGS__);                                                                         \
            fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                                                         \
        }                                                                                                              \
    })
#else
#define LOG_ERROR(msg, ...) { }
#endif