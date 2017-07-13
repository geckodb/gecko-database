#pragma once

#include <stdinc.h>
#include <frag.h>
#include <pred.h>

struct frag_t *scan_mediator(struct frag_t *self, const pred_tree_t *pred, size_t batch_size, size_t nthreads);