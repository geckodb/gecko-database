
// ---------------------------------------------------------------------------------------------------------------------
// I N C L U D E S
// ---------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <defs.h>
#include <tableimg.h>
#include <time.h>

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   P R O T O T Y P E S
// ---------------------------------------------------------------------------------------------------------------------

void
show_version();

void
show_usage();

void
show_file_info(
        char *file
);

void
show_table_head(
        size_t tuple_pos_start,
        size_t limit,
        char *file
);

void
print_table_header(
        timg_header_t *header,
        timg_var_header_t *var_header
);

void
print_table_using_nsm(
        FILE *file,
        timg_header_t *header,
        timg_var_header_t *var_header
);

void
print_table_using_dsm(
        FILE *file,
        timg_header_t *header,
        timg_var_header_t *var_header
);

void
print_h_line(
        timg_header_t *header,
        timg_var_header_t *var_header
);

// ---------------------------------------------------------------------------------------------------------------------
// M A I N
// ---------------------------------------------------------------------------------------------------------------------

void print_table_footer(timg_header_t *header, timg_var_header_t *var_header, size_t current_num,
                        size_t max_num);

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        show_version();
    } else if (argc == 3 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--info") == 0)) {
        show_file_info(argv[2]);
    } else if (argc == 5 && (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--look") == 0)) {
        show_table_head(atoi(argv[2]), atoi(argv[3]), argv[4]);
    } else {
        show_usage();
    }
    return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------------------
// H E L P E R   I M P L E M E N T A T I O N
// ---------------------------------------------------------------------------------------------------------------------

void show_version()
{
    printf("tableimg version 1.0.00 ((c) 2017 Marcus Pinnecke)\n"
           "The tool tableimg is used to view or manipulate table image files (*timg).\n");
}

void show_usage()
{
    printf("usage: tableimg [--version] [-i | --info <file>] [-l | --look <tuple num start> <limit> <file>]");
}

void show_file_info(char *file)
{
    FILE *file_ptr = fopen(file, "rb");
    if (!file_ptr) {
        perror("Unable to open input file.");
        exit(EXIT_FAILURE);
    } else {
        timg_header_t header;
        timg_var_header_t var_header;
        tableimg_header_load(file_ptr, &header, &var_header);

        if (header.format_version != TIMG_VER_1) {
            perror("Unsupported tuplet_format version.");
            exit(EXIT_FAILURE);
        }

        time_t t = header.timestamp_created;
        char buf[80];
        struct tm* st = localtime(&t);
        strftime(buf, 80, "%c", st);


        printf("================================================================================\n");
        printf(" *** %s/%s *** (IMAGE)\n", var_header.database_name, var_header.table_name);
        printf("================================================================================\n");
        printf("%llu tuples (%llu fields) are stored in this image.\n",
               header.num_tuples, header.num_tuples * header.num_attributes_len);
        printf("\n");
        printf("META\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("specification......: %s\n", var_header.table_spec_ref);
        printf("content size.......: %0.6fG\n", (tableimg_rawsize(&header, &var_header)/1024.0/1024.0/1024.0));
        printf("serialization......: %s\n", header.flags.serial_format_type == TIMG_FORMAT_NSM ? "row-wise" : "columnar");
        printf("comment............: %s\n", var_header.comment);
        printf("md5 checksum.......: ");
        for(int i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", header.raw_table_data_checksum[i]);
        printf("\n\n");
        printf("schema_t\n");
        printf("--------------------------------------------------------------------------------\n");
        for (int i = 0; i < header.num_attributes_len; i++) {
            attr_t attr = var_header.attributes[i];
            printf("attribute name.....: %s\n", attr.name);
            if (attr.type_rep > 1)
                printf("data type..........: %s (%zu)\n", gs_type_str(attr.type), attr.type_rep);
            else
                printf("data type..........: %s\n", gs_type_str(attr.type));
            printf("primary/foreign....: %d/%d\n",
                   attr.flags.primary, attr.flags.foreign);
            printf("unique/null/inc....: %d/%d/%d\n",
                   attr.flags.unique, attr.flags.nullable,
                   attr.flags.autoinc);
            printf("md5 checksum.......: ");
            for(int i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", attr.checksum[i]);
            printf("\n");
            printf("--------------------------------------------------------------------------------\n");
        }
        printf("image tuplet_format version %d; created on %s\n", header.format_version, buf);
        tableimg_header_free(&var_header);
    }
}

void show_table_head(size_t tuple_pos_start, size_t limit, char *file) {

    FILE *file_ptr = fopen(file, "rb");
    if (!file_ptr) {
        perror("Unable to open input file.");
        exit(EXIT_FAILURE);
    } else {

        timg_header_t header;
        timg_var_header_t var_header;
        tableimg_header_load(file_ptr, &header, &var_header);

        if (header.format_version != TIMG_VER_1) {
            perror("Unsupported tuplet_format version.");
            exit(EXIT_FAILURE);
        }

        print_table_header(&header, &var_header);

        switch (header.flags.serial_format_type) {
            case TIMG_FORMAT_NSM:
                print_table_using_nsm(file_ptr, &header, &var_header);
                break;
            case TIMG_FORMAT_DSM:
                print_table_using_dsm(file_ptr, &header, &var_header);
                break;
            default:
                perror("Unknown serialization tuplet_format.");
                exit(EXIT_FAILURE);
        }

        print_table_footer(&header, &var_header, (tuple_pos_start), header.num_tuples);


    }
}

void
print_table_footer(
    timg_header_t *header,
    timg_var_header_t *var_header,
    size_t current_num,
    size_t max_num)
{
    print_h_line(header, var_header);
    printf(" displayed %zu of %zu records", current_num, max_num);
}

void
print_h_line(
        timg_header_t *header,
        timg_var_header_t *var_header)
{
    for (size_t attr_idx = 0; attr_idx < header->num_attributes_len; attr_idx++) {
        attr_t attr = var_header->attributes[attr_idx];
        size_t column_width = max(strlen(attr.name), attr.str_format_mlen);

        printf("+");
        for (size_t i = 0; i < column_width + 2; i++)
            printf("-");
    }

    printf("+\n");
}

void
print_table_header(
    timg_header_t *header,
    timg_var_header_t *var_header)
{
    char format_buffer[2048];

    print_h_line(header, var_header);

    for (size_t attr_idx = 0; attr_idx < header->num_attributes_len; attr_idx++) {
        attr_t attr = var_header->attributes[attr_idx];
        size_t column_width = max(strlen(attr.name), attr.str_format_mlen);
        sprintf(format_buffer, "| %%-%zus ", column_width);
        printf(format_buffer, attr.name);
    }
    printf("|\n");

    print_h_line(header, var_header);
}

void print_table_using_nsm(
    FILE *file,
    timg_header_t *header,
    timg_var_header_t *var_header) {

 //   char format_buffer[2048];


    for (size_t tuple_id = 0; tuple_id < header->num_tuples; tuple_id++) {
        for (size_t attr_idx = 0; attr_idx < header->num_attributes_len; attr_idx++) {
//            attr_t attr = var_header->attr[attr_idx];
//            size_t column_width = max(strlen(attr.name), attr.str_format_mlen);

         /*   switch (attr.type) {
                case FT_BOOL:
                    sprintf(format_buffer, "| %%-%zud ", column_width);
                    printf(format_buffer, *(BOOL*)%d);
                case FT_INT8:
                case FT_INT16:
                case FT_INT32:
                case FT_INT64:

                case FT_UINT8:
                case FT_UINT16:
                case FT_UINT32:
                case FT_UINT64:
                case FT_FLOAT32:
                case FT_FLOAT64:
                case FT_CHAR:
                default:
                    perror("Unknown type");
                    abort();
            }*/
        }
    }

}

void print_table_using_dsm(
    FILE *file,
    timg_header_t *header,
    timg_var_header_t *var_header) {



}