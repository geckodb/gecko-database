#include <grid_table.h>

grid_t *gs_grid_create(schema_t *schema, size_t tuplet_capacity, enum frag_impl_type_t type)
{
    require_non_null(schema);
    grid_t *result = require_good_malloc(sizeof(grid_t));
    *result = (grid_t) {
        .frag = gs_fragment_alloc(schema, tuplet_capacity, type),
        .tuple_id_mapping = vector_create(sizeof(tuple_id_t), tuplet_capacity)
    };
    return result;
}

void gs_grid_free(grid_t *grid)
{
    require_non_null(grid);
    vector_free(grid->tuple_id_mapping);
    gs_fragment_free(grid->frag);
    free(grid);
}

tuplet_t *gs_grid_insert(grid_t *grid, const tuple_id_t *tuple_ids, size_t num_tuples)
{
    require_non_null(grid);
    require_non_null(grid->tuple_id_mapping);
    require_non_null(tuple_ids);
    require_non_zero(num_tuples);

    vector_add(grid->tuple_id_mapping, num_tuples, tuple_ids);
    return gs_fragment_insert(grid->frag, num_tuples);
}

const tuple_id_t *gs_grid_covered_tuples(const grid_t *grid)
{
    require_non_null(grid);
    return vector_get(grid->tuple_id_mapping);
}

size_t gs_grid_num_tuplets(const grid_t *grid)
{
    require_non_null(grid);
    return (grid->tuple_id_mapping->num_elements);
}

const schema_t *gs_grid_schema(const grid_t *grid)
{
    require_non_null(grid);
    require_non_null(grid->frag);
    require_non_null(grid->frag->schema);
    return (grid->frag->schema);
}
