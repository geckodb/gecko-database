// Copyright (C) 2017 Marcus Pinnecke
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either user_port 3 of the License, or
// (at your option) any later user_port.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.
// If not, see .

// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <tableimg.h>


// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R  I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------


size_t tableimg_rawsize(
        timg_header_t *header,
        timg_var_header_t *var_header)
{
    /*
    size_t total = 0;
    for (size_t i = 0; i < header->num_attributes_len; i++) {
        total += get_field_size(var_header->attributes, i);
    }
    return (total * header->num_tuples);
     */
    panic(NOTIMPLEMENTED, to_string(tableimg_rawsize))
    return 0;
}



void timg_table_free(void *data)
{
    if (data != NULL)
        free (data);
}



void write_header(
    FILE *file,
    timg_version_t version,
    const char *database_name,
    const char *table_name,
    const char *table_spec_ref,
    const char *comment,
    const void *tuple_data,
    size_t num_tuples,
    enum tuplet_format format_out,
    size_t num_attr,
    size_t tuple_size)
{
    timg_header_t header = {
        .magic                  = TIMG_MAGIC_WORD,
        .format_version         = version,
        .database_name_len      = strlen(database_name),
        .table_name_len         = strlen(table_name),
        .table_spec_ref_len     = strlen(table_spec_ref),
        .comment_len            = strlen(comment),
        .num_attributes_len     = num_attr,

        .flags = {
                .serial_format_type = (format_out == TF_NSM ? TIMG_FORMAT_NSM : TIMG_FORMAT_DSM)
        },

        .num_tuples        = num_tuples,
        .timestamp_created = (uint64_t) time(NULL),
    };

    checksum_context_t raw_data_checksum;
    gs_checksum_begin(&raw_data_checksum);
    gs_checksum_update(&raw_data_checksum, tuple_data, tuple_data + tuple_size * num_tuples);
    gs_checksum_end(header.raw_table_data_checksum, &raw_data_checksum);

    fwrite(&header, sizeof(timg_header_t), 1, file);
}

void write_var_header(
    FILE *file,
    const char *database_name,
    const char *table_name,
    const char *table_spec_ref,
    const char *comment,
    const attr_t *attr,
    size_t num_attr
)
{
    fwrite(database_name, strlen(database_name), 1, file);
    fwrite(table_name, strlen(table_name), 1, file);
    fwrite(table_spec_ref, strlen(table_spec_ref), 1, file);
    fwrite(comment, strlen(comment), 1, file);

    fwrite(attr, sizeof(attr_t), num_attr, file);
}

size_t get_var_header_size(
        const char *database_name,
        const char *table_name,
        const char *table_spec_ref,
        const char *comment,
        size_t num_attr
)
{
    return strlen(database_name) + strlen(table_name) + strlen(table_spec_ref) + strlen(comment) +
            num_attr * sizeof(attr_t);
}

timg_error_t write_table_data(
    FILE *file,
    schema_t *schema,
    const void *tuple_data,
    size_t num_tuples,
    enum tuplet_format format_in,
    enum tuplet_format format_out,
    size_t num_attr,
    size_t tuple_size)
{
    size_t field_offs = 0;
    size_t field_size = 0;
    const void   *cursor    = NULL;
//    attr_t  *attr      = (attr_t  *) schema->attr->data;

    switch (format_in) {
        case TF_NSM:
            gs_checksum_nsm(schema, tuple_data, num_tuples);
            switch (format_out) {
                case TF_NSM:
                    fwrite(tuple_data, tuple_size, num_tuples, file);
                    break;
                case TF_DSM:
                    for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
                        cursor     = tuple_data;
                        panic(NOTIMPLEMENTED, to_string(get_tuple_size) " and " to_string(get_field_size))
                        //field_offs = get_tuple_size(schema);
                        //field_size = get_field_size(attr, attr_idx);
                        for (size_t tuple_idx = 0; tuple_idx < num_tuples; tuple_idx++) {
                            fwrite(cursor + field_offs, field_size, 1, file);
                            cursor += tuple_size;
                        }
                    }
                    break;
                default:
                    return TIMG_ERR_BADSERIALFORMAT;
            }
            break;
        case TF_DSM:
            gs_checksum_dms(schema, tuple_data, num_tuples);
            switch (format_out) {
                case TF_NSM:
                    cursor     = tuple_data;
                    for (size_t tuple_idx = 0; tuple_idx < num_attr; tuple_idx++) {
                        for (size_t attr_idx = 0; attr_idx < num_attr; attr_idx++) {
                            field_offs = 0;

                            panic(NOTIMPLEMENTED, to_string(get_field_size) " and " to_string(get_field_size))
                            //for (size_t prev_attr_idx = 0; prev_attr_idx < attr_idx; prev_attr_idx++) {
                            //    field_offs = num_tuples * get_field_size(attr, attr_idx);
                            //}
                            //field_size = get_field_size(attr, attr_idx);
                            field_offs += tuple_idx * field_size;
                            fwrite(cursor + field_offs, field_size, 1, file);
                        }
                    }
                    break;
                case TF_DSM:
                    fwrite(tuple_data, tuple_size, num_tuples, file);
                    break;
                default:
                    return TIMG_ERR_BADSERIALFORMAT;
            }
            break;
        default:
            return TIMG_ERR_BADSERIALFORMAT;
    }
    return TIMG_ERR_OK;
}

timg_error_t test_file_io(FILE *file)
{
    int file_desc = fileno(file);
    fcntl(file_desc, F_GETFL);
    if (fcntl(file_desc, F_GETFL) == -1 && errno == EBADF)
        return TIMG_ERR_BADFILE;
    rewind(file);
    return TIMG_ERR_OK;
}

timg_error_t check_write_args(
        FILE *file,
        const char *database_name,
        const char *table_name,
        const char *table_spec_ref,
        const char *comment,
        schema_t *schema,
        const void *tuple_data)
{
    if ((file == NULL ) || (database_name == NULL) || (table_name == NULL) || (table_spec_ref == NULL) ||
        (schema == NULL) || (tuple_data == NULL) || (comment == NULL) || (schema->attr->num_elements == 0))
        return TIMG_ERR_ILLEGALARG;
    else return TIMG_ERR_OK;
}

timg_error_t tableimg_fwrite(
        FILE *file,
        timg_version_t version,
        const char *db_name,
        const char *tab_name,
        const char *spec_ref,
        const char *comment,
        schema_t *schema,
        const void *tab_data,
        size_t num_tuples,
        enum tuplet_format in,
        enum tuplet_format out)
{
    timg_error_t  result;

    if ((result = check_write_args(file, db_name, tab_name, spec_ref, comment, schema, tab_data)) != TIMG_ERR_OK)
        return result;

    if ((result = test_file_io(file)) != TIMG_ERR_OK)
        return result;

    panic(NOTIMPLEMENTED, to_string(tableimg_fwrite))
    size_t       size       = 0; //get_tuple_size(schema);
    attr_t *attr       = (attr_t *) schema->attr->data;
    size_t       num_attr   = schema->attr->num_elements;

    write_header(file, version, db_name, tab_name, spec_ref, comment, tab_data, num_tuples, out, num_attr, size);
    size_t last_pos         = ftell(file);
    size_t var_header_size  = get_var_header_size(db_name, tab_name, spec_ref, comment, num_attr);
    fseek(file, var_header_size, SEEK_CUR);
    write_table_data(file, schema, tab_data, num_tuples, in, out, num_attr, size);
    fseek(file, last_pos, SEEK_SET);
    write_var_header(file, db_name, tab_name, spec_ref, comment, attr, num_attr);

    return TIMG_ERR_OK;
}



timg_error_t tableimg_header_load(
        FILE *file,
        timg_header_t *header,
        timg_var_header_t *var_header)
{
    if (file == NULL || header == NULL || var_header == NULL)
        return TIMG_ERR_ILLEGALARG;

    int file_desc = fileno(file);
    fcntl(file_desc, F_GETFL);
    if (fcntl(file_desc, F_GETFL) == -1 && errno == EBADF)
        return TIMG_ERR_BADFILE;

    fseek(file, 0L, SEEK_END);
    if (ftell(file) < sizeof(timg_header_t))
        return TIMG_ERR_CORRUPTED;
    fseek(file, 0L, SEEK_SET);

    fread(header, sizeof(timg_header_t), 1, file);
    var_header->database_name = malloc(header->database_name_len + 1);
    var_header->table_name = malloc(header->table_name_len + 1);
    var_header->table_spec_ref = malloc(header->table_spec_ref_len + 1);
    var_header->comment = malloc(header->comment_len + 1);
    var_header->attributes = malloc(header->num_attributes_len * sizeof(attr_t));

    fread(var_header->database_name, header->database_name_len, 1, file);
    fread(var_header->table_name, header->table_name_len, 1, file);
    fread(var_header->table_spec_ref, header->table_spec_ref_len, 1, file);
    fread(var_header->comment, header->comment_len, 1, file);

    var_header->database_name[header->database_name_len] = var_header->table_name[header->table_name_len] =
            var_header->table_spec_ref[header->table_spec_ref_len] = var_header->comment[header->comment_len] = '\0';

    fread(var_header->attributes, sizeof(attr_t), header->num_attributes_len, file);

    return TIMG_ERR_OK;
}

timg_error_t tableimg_header_free(
    timg_var_header_t *var_header)
{
    if (var_header == NULL)
        return TIMG_ERR_ILLEGALARG;

    if (var_header->attributes != NULL)
        free (var_header->attributes);
    if (var_header->database_name != NULL)
        free (var_header->database_name);
    if (var_header->comment != NULL)
        free (var_header->comment);
    if (var_header->table_name != NULL)
        free (var_header->table_name);
    if (var_header->table_spec_ref != NULL)
        free (var_header->table_spec_ref);

    var_header->database_name = var_header->comment =
    var_header->table_name    = var_header->table_spec_ref = NULL;
    var_header->attributes = NULL;

    return TIMG_ERR_OK;
}