#pragma once

#include <stdinc.h>
#include <schema.h>

struct frag_t;

struct frag_t *gs_frag_host_vm_nsm_create(schema_t *schema, size_t tuplet_capacity);

struct frag_t *gs_frag_host_vm_dsm_create(schema_t *schema, size_t tuplet_capacity);
