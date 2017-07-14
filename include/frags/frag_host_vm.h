#pragma once

#include <frag.h>

frag_t *gs_frag_host_vm_alloc(schema_t *schema, size_t tuplet_capacity, enum tuplet_format format);