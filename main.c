
#include <containers/vector.h>
#include <storage/memory.h>
#include <pref.h>

#define TUPLE_PER_BLOCK     80

typedef struct {
    size_t         a;
    size_t         b;
    size_t         c;
} hardcoded_tuple_t;

void
print_to_console(
    void *         caputure,
    const void *   data)
{
    for (int i = 0; i < TUPLE_PER_BLOCK; i++) {
        hardcoded_tuple_t *tuple = (hardcoded_tuple_t *) (data + i * sizeof(hardcoded_tuple_t));
        printf("tuple (%zu, %zu, %zu)\n", tuple->a, tuple->b, tuple->c);
    }
}

int
main(void)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // B U F F E R   M A N A G E R   T E S T
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 1000; i++) {
        anti_buf_t *buffer = buf_create(DEFAULT_BUF_CONFIG);
        cursor_t *  cursor = buf_alloc(buffer, TUPLE_PER_BLOCK * sizeof(hardcoded_tuple_t), 10,
                                       positioning_first_nomerge);
        buf_open(cursor);
        while (buf_next(cursor)) {
            for (int i = 0; i < TUPLE_PER_BLOCK; i++) {
                hardcoded_tuple_t my_tuple = {
                        .a = i,
                        .b = i + 1,
                        .c = i + 2
                };
                buf_memcpy(cursor, i * sizeof(hardcoded_tuple_t), &my_tuple, sizeof(hardcoded_tuple_t));
            }
        }
        buf_close(cursor);

        pref_t pref;
        pref_create(&pref, "/Users/marcus/temp/test.conf");
        const char *value = pref_get_str(&pref, "ein.schlüssel", "nütschs");
        printf(">> value = '%s'\n", value);
        const char *value2 = pref_get_str(&pref, "ein.schlüssel2", "nütschs");
        printf(">> value2 = '%s'\n", value2);
        const char *value3 = pref_get_str(&pref, "ein.schlüssel3", "nütschs");
        printf(">> value3 = '%s'\n", value3);
        const char *value4 = pref_get_str(&pref, "Ein_Schlüssel_Ohne", "nütschs");
        printf(">> value4 = '%s'\n", value4);
        const char *value5 = pref_get_str(&pref, "Ein_Schlüssel_Ohn", "nütschs");
        printf(">> value5 = '%s'\n", value5);
        const char *value6 = pref_get_str(&pref, "Das_ist_noch_mehr_schluessel", "nütschs");
        printf(">> value6 = '%s'\n", value6);


        buf_open(cursor);
        while (buf_next(cursor)) {
            buf_read(cursor, NULL, print_to_console);
        }
        buf_close(cursor);
        buf_release(cursor);

        cursor_t *  cursor2 = buf_alloc(buffer, HOTSTORE_SIZELIMIT, 10,
                                       positioning_first_nomerge);
        buf_open(cursor2);
    }

    return EXIT_SUCCESS;
}