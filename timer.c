#include <timer.h>

void timer_start(timer_t *timer)
{
    assert (timer);
    timer->start = clock();
}

void timer_stop(timer_t *timer)
{
    assert (timer);
    timer->stop = clock();
}

double timer_diff_ms(timer_t *timer)
{
    assert (timer);
    return (double)(timer->stop - timer->start) / CLOCKS_PER_SEC;
}