#include <area.h>
#include <require.h>

struct {

} _global_table_context_register;

mdb_table_space_context *mdb_table_space_context_alloc(const char *context_name)
{
    bool result = mdb_require_non_null(context_name);
}

bool mdb_table_space_context_free(const char *context_name)
{

}

mdb_table_space *mdb_table_space_by_name(const mdb_table_space_context *context, const char *name)
{

}

mdb_table_space *mdb_table_space_all(const mdb_table_space_context *context)
{

}

mdb_table_space *mdb_table_space_alloc(mdb_table_space_context *context, const char *space_name)
{

}

bool mdb_table_space_free(mdb_table_space *space)
{

}