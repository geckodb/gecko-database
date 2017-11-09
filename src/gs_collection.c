#include <gs_collection.h>
#include <gs_grid.h>

typedef struct gs_collection_t {
    char *name;
    gs_table_t *documents;
    gs_table_t *relations;
} gs_collection_t;

void create_core_schema(gs_schema_t *schema);

void create_internal_attr(gs_schema_t *schema, enum gs_field_type_e type);

bool is_internal_attr(const gs_attr_t *attr);

GS_DECLARE(gs_status_t) gs_collection_create(gs_collection_t **collection, const char *name)
{
    gs_collection_t *result = GS_REQUIRE_MALLOC(sizeof(gs_collection_t));

    gs_schema_t *documents_schema = gs_schema_new("collection_records");
    gs_schema_t *relations_schema = gs_schema_new("collection_relations");

    create_core_schema(documents_schema);
    create_core_schema(relations_schema);
    create_internal_attr(relations_schema, FT_RELTYPE);

    result->documents = gs_table_new(documents_schema, 1);
    result->relations = gs_table_new(relations_schema, 1);
    result->name = strdup(name);

    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_collection_dispose(gs_collection_t *collection)
{
    GS_REQUIRE_NONNULL(collection);
    gs_table_delete(collection->relations);
    gs_table_delete(collection->documents);
    free(collection->name);
    free(collection);

    return GS_SUCCESS;
}

GS_DECLARE(const char*) gs_collection_get_name(gs_collection_t *collection)
{
    GS_REQUIRE_NONNULL(collection);
    return (collection->name);
}

GS_DECLARE(gs_status_t) gs_collection_print(FILE *file, gs_collection_t *collection)
{
    GS_REQUIRE_NONNULL(file);
    GS_REQUIRE_NONNULL(collection);
    // TODO Print as JSON
    fprintf(file, "%s\n", collection->name);
    return GS_SUCCESS;
}

void create_core_schema(gs_schema_t *schema)
{
    create_internal_attr(schema, FT_BOOL);
    create_internal_attr(schema, FT_UINT8);
    create_internal_attr(schema, FT_UINT16);
    create_internal_attr(schema, FT_UINT32);
    create_internal_attr(schema, FT_UINT64);
    create_internal_attr(schema, FT_INT8);
    create_internal_attr(schema, FT_INT16);
    create_internal_attr(schema, FT_INT32);
    create_internal_attr(schema, FT_INT64);
    create_internal_attr(schema, FT_FLOAT32);
    create_internal_attr(schema, FT_FLOAT64);
    create_internal_attr(schema, FT_STRPTR);
}

void create_internal_attr(gs_schema_t *schema, enum gs_field_type_e type)
{
    assert(schema);

    char buffer[256];
    sprintf(buffer, ":$%s", gs_field_type_str(type));
    gs_attr_create(buffer, type, 1, schema);
}

bool is_internal_attr(const gs_attr_t *attr)
{
    assert(attr);
    return (strlen(attr->name) > 2 ? (attr->name[0] == ':' && attr->name[1] == '$') : false);
}