#pragma once

#include <stdinc.h>

typedef struct timer_t {
    clock_t start, stop;
} timer_t;

void timer_start(timer_t *timer);

void timer_stop(timer_t *timer);

double timer_diff_ms(timer_t *timer);
