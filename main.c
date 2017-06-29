
#include <containers/vector.h>
#include <storage/memory.h>

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
        anti_buf_t *buffer = buf_create(10000, 100, 100, 50000);
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

        buf_open(cursor);
        while (buf_next(cursor)) {
            buf_read(cursor, NULL, print_to_console);
        }
        buf_close(cursor);
        buf_release(cursor);
    }

    return EXIT_SUCCESS;
}