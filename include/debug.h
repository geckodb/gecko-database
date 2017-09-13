// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.

#pragma once

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdinc.h>

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#ifndef NDEBUG
    #define LOG_DEBUG(mondrian_instance, msg, ...)                                                                     \
    ({                                                                                                                 \
        if (mondrian_config(mondrian_instance)->log.debug) {                                                           \
            fprintf(stderr, "# [DEBUG]: ");                                                                            \
            fprintf(stderr, msg, __VA_ARGS__);                                                                         \
            fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__);                                                         \
        }                                                                                                              \
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