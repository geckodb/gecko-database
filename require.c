#include <require.h>
#include <error.h>

bool require_non_null(const void *ptr)
{
    bool is_non_null = (ptr != NULL);
    error_set_last_if(!is_non_null, EC_NULLPOINTER);
    return is_non_null;
}

bool require_less_than(const void *lhs, const void *rhs)
{
    bool is_less_than = (lhs < rhs);
    error_set_last_if(!is_less_than, EC_CORRUPTEDORDER);
    return is_less_than;
}