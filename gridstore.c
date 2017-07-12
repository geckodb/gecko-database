#include <gridstore.h>

error_code gridstore_init(gridstore_instance_t *instance)
{
    assert (instance);
    instance->timestamp_start = time(NULL);
    return err_no_error;
}

const char *gridstore_get_config(gridstore_instance_t *instance, const char *settings_name)
{
    return dict_get(instance->config, settings_name);
}

void gridstore_shutdown(gridstore_instance_t *instance)
{
    assert (instance);
    //conf_free(instance->config);  // TODO: freeing up config causes freeing up unallocated pointer
}