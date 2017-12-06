
#include <gecko-commons/gs_base_relative_ptr.h>

int main(void) {

    base_relative_ptr_t ptr;

    int data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    size_t data_base_adr = (size_t) &data[0];

    BASE_RELATIVE_PTR_SET(ptr, &data_base_adr, data + 4);

//    gs_base_relative_ptr_create(&ptr, &data_base_adr, data + 4);

    void *dst             = gs_base_relative_ptr_get_dst(&ptr);
    void *base            = gs_base_relative_ptr_get_base(&ptr);
    void *ptr_to_base_adr = gs_base_relative_ptr_get_ptr_to_base_adr(&ptr);
    uint32_t offset       = gs_base_relative_ptr_get_offset_to_base(&ptr);

    assert (dst == data + 4);
    assert (base == data);
    assert (ptr_to_base_adr == &data_base_adr);
    assert (offset == (data[4] - data[0]) * sizeof(int));

    data_base_adr = (size_t) &data[1];

    gs_base_relative_ptr_move_base(&ptr, &data_base_adr);
    dst             = gs_base_relative_ptr_get_dst(&ptr);
    base            = gs_base_relative_ptr_get_base(&ptr);
    ptr_to_base_adr = gs_base_relative_ptr_get_ptr_to_base_adr(&ptr);
    offset          = gs_base_relative_ptr_get_offset_to_base(&ptr);

    assert (dst == data + 5);
    assert (base == data + 1);
    assert (ptr_to_base_adr == &data_base_adr);
    assert (offset == (data[5] - data[1]) * sizeof(int));

    gs_base_relative_ptr_move_dst(&ptr, &data[6]);
    dst             = gs_base_relative_ptr_get_dst(&ptr);
    base            = gs_base_relative_ptr_get_base(&ptr);
    ptr_to_base_adr = gs_base_relative_ptr_get_ptr_to_base_adr(&ptr);
    offset          = gs_base_relative_ptr_get_offset_to_base(&ptr);

    assert (dst == data + 6);
    assert (base == data + 1);
    assert (ptr_to_base_adr == &data_base_adr);
    assert (offset == (data[6] - data[1]) * sizeof(int));


    //GS_DECLARE(gs_status_t) gs_base_relative_ptr_move_dst(gs_base_relative_ptr_t *ptr, void *dst);

    //GS_DECLARE(int32_t) gs_base_relative_ptr_get_offset_to_base



    return 0;
}