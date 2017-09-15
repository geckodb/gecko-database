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

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

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