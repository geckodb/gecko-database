// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
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

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <execinfo.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gs_utils.h>
#include <sys/stat.h>

#include <openssl/md5.h>

#include <gs_platform.h>
#include <gs_global.h>
#include <gs_msg.h>
#include <gs_error.h>
#include <gs_require.h>
#include <gs_hash.h>
#include <zconf.h>

// ---------------------------------------------------------------------------------------------------------------------
// C O N F I G
// ---------------------------------------------------------------------------------------------------------------------

#ifndef ATTR_NAME_MAXLEN
#define ATTR_NAME_MAXLEN     1024
#endif
#define FLAG_REGULAR          0
#define FLAG_PRIMARY     1 << 1
#define FLAG_FOREIGN     1 << 2
#define FLAG_NULLABLE    1 << 3
#define FLAG_AUTOINC     1 << 4
#define FLAG_UNIQUE      1 << 5

// ---------------------------------------------------------------------------------------------------------------------
// D A T A T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

typedef uint64_t gs_attr_id_t;

typedef uint32_t gs_tuplet_id_t;

typedef int (*gs_comp_func_t)(const void *lhs, const void *rhs);

typedef uint8_t     gs_u8_t;
typedef uint16_t   gs_u16_t;
typedef uint32_t   gs_u32_t;
typedef uint64_t   gs_u64_t;
typedef int64_t    gs_s64_t;

enum gs_tuplet_format_e {
    TF_NSM  = 1,
    TF_DSM  = 2,
};


typedef struct {
    uint8_t primary  : 1;
    uint8_t foreign  : 1;
    uint8_t nullable : 1;
    uint8_t autoinc  : 1;
    uint8_t unique   : 1;
} GS_ATTR_FLAGS_t;

typedef MD5_CTX gs_checksum_context_t;

// ---------------------------------------------------------------------------------------------------------------------
// M A C R O S
// ---------------------------------------------------------------------------------------------------------------------

#define min(x,y) (x > y ? y : x)
#define max(x,y) (x > y ? x : y)

#if defined(__cplusplus)
#define	__BEGIN_DECLS	extern "C" {
#define	__END_DECLS	}
#else
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif

#define GS_DECLARE(type) type

#define to_string2(x)   to_string(x)
#define to_string(x)    #x

#define ARRAY_LEN_OF(x) (sizeof(x) / sizeof(x[0]))

#define IS_FLAG_SET(set, flag) ((set & flag) == flag)

#define DELEGATE_CALL(instance, fun)                                                                                   \
    ({                                                                                                                 \
        GS_REQUIRE_NONNULL(instance);                                                                                  \
        REQUIRE_IMPL(instance->fun);                                                                                   \
        instance->fun(instance);                                                                                       \
    })

#define DELEGATE_CALL_WARGS(instance, fun, ...)                                                                        \
    ({                                                                                                                 \
        GS_REQUIRE_NONNULL(instance);                                                                                  \
        REQUIRE_IMPL(instance->fun);                                                                                   \
        instance->fun(instance,__VA_ARGS__);                                                                           \
    })

#define GS_CONNECT(sig, slot)                                                                                          \
    gs_error_if((gs_dispatcher_connect(dispatcher, sig, slot) != GS_SUCCESS), err_connect_failed);

// ---------------------------------------------------------------------------------------------------------------------
// D E F I N E S
// ---------------------------------------------------------------------------------------------------------------------

#ifndef BOOL
#define BOOL      bool
#endif
#ifndef INT8
#define INT8      int8_t
#endif
#ifndef INT16
#define INT16     int16_t
#endif
#ifndef INT32
#define INT32     int32_t
#endif
#ifndef INT64
#define INT64     int64_t
#endif
#ifndef UINT8
#define UINT8     uint8_t
#endif
#ifndef UINT16
#define UINT16    uint16_t
#endif
#ifndef UINT32
#define UINT32    uint32_t
#endif
#ifndef UINT64
#define UINT64    uint64_t
#endif
#ifndef FLOAT32
#define FLOAT32   float
#endif
#ifndef FLOAT64
#define FLOAT64   double
#endif
#ifndef CHAR
#define CHAR      char
#endif
#ifndef STRPTR
#define STRPTR    char*
#endif
#ifndef ATTRID
#define ATTRID    gs_attr_id_t
#endif
#ifndef GRIDID
#define GRIDID    gs_grid_id_t
#endif
#ifndef TUPLEID
#define TUPLEID   gs_tuple_id_t
#endif
#ifndef FRAGTYPE
#define FRAGTYPE  enum frag_impl_type_t
#endif
#ifndef SIZE
#define SIZE      size_t
#endif
#ifndef TFORMAT
#define TFORMAT   enum gs_tuplet_format_e
#endif