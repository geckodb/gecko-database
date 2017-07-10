#pragma once

#include <stdinc.h>
#include <frag.h>
#include <pred.h>

struct fragment_t *scan_mediator(struct fragment_t *self, const pred_tree_t *pred, size_t batch_size, size_t nthreads);