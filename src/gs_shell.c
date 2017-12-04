#include <stdatomic.h>

#include <apr_file_io.h>
#include <apr_strings.h>

#include <gecko-commons/c11threads.h>
#include <gecko-http/gs_server.h>

#include <gs_shell.h>
#include <gs_dispatcher.h>

typedef struct gs_shell_t {
    gs_system_t         *system;
    apr_pool_t          *pool;
    thrd_t               thread;
    atomic_bool          is_running;
    atomic_bool          is_disposable;
    //apr_uid_t            user_id;
    //apr_gid_t            group_id;
    //char                *user_name;
    apr_time_t           uptime;
} gs_shell_t;

typedef struct gs_shell_loop_args_t {
    gs_system_t         *system;
    gs_shell_t          *shell;
} gs_shell_loop_args_t;

 int shell_loop(void *args);

 bool process_input(gs_system_t *system, gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input);

 void process_command_help(gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input);
 void process_command_exit(gs_system_t *system, gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input);
 void process_command_uptime(gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input);
void process_command_port_info(gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input);

GS_DECLARE(gs_status_t) gs_shell_create(gs_shell_t **shell, gs_system_t *system)
{
    gs_shell_t *result = GS_REQUIRE_MALLOC(sizeof(gs_shell_t));
    result->system = system;

    result->is_running = ATOMIC_VAR_INIT(false);
    result->is_disposable = ATOMIC_VAR_INIT(false);
    apr_pool_create(&result->pool, NULL);
  //  apr_uid_current(&result->user_id, &result->group_id, result->pool);
  //  apr_uid_name_get(&result->user_name, result->user_id, result->pool);
    result->uptime = apr_time_now();
    *shell = result;
    return GS_SUCCESS;
}

GS_DECLARE(gs_status_t) gs_shell_start(gs_shell_t *shell, gs_system_t *system)
{
    GS_REQUIRE_NONNULL(shell);
    if (!atomic_load(&shell->is_running)) {
        GS_DEBUG("shell %p is starting", shell);
        atomic_store(&shell->is_running, true);

        gs_shell_loop_args_t *args = GS_REQUIRE_MALLOC(sizeof(gs_shell_loop_args_t));
        args->shell = shell;
        args->system = system;

        thrd_create(&shell->thread, shell_loop, args);
        return GS_SUCCESS;
    } else {
        warn("shell %p was request to start but runs already", shell);
        return GS_FAILED;
    }
}

GS_DECLARE(gs_status_t) gs_shell_dispose(gs_shell_t **shell_ptr)
{
    GS_REQUIRE_NONNULL(shell_ptr);
    GS_REQUIRE_NONNULL(*shell_ptr);
    gs_shell_t *shell = *shell_ptr;
    if (shell->is_disposable) {
        apr_pool_destroy(shell->pool);
        GS_DEBUG("shell %p disposed", shell);
        free(shell);
        *shell_ptr = NULL;
        return GS_SUCCESS;
    } else return GS_TRYAGAIN;
}

GS_DECLARE(gs_status_t) gs_shell_handle_events(const gs_event_t *event)
{
    GS_REQUIRE_NONNULL(event);
    GS_EVENT_FILTER_BY_RECEIVER_TAG(GS_OBJECT_TYPE_SHELL);

    gs_shell_t      *shell  = GS_EVENT_GET_RECEIVER(gs_shell_t);
    gs_signal_type_e signal = GS_EVENT_GET_SIGNAL();

    switch (signal) {
        case GS_SIG_SHUTDOWN:
            GS_DEBUG("shell %p received shutdown signal", shell);
            gs_shell_shutdown(shell);
            return GS_SKIPPED;
        default:
        warn("shell %p received event for signal %d that is not handled", shell, signal);
            return GS_SKIPPED;
    }
}

GS_DECLARE(gs_status_t) gs_shell_shutdown(gs_shell_t *shell)
{
    if (atomic_load(&shell->is_running)) {
        GS_DEBUG("shell %p is shutting down", shell);
        atomic_store(&shell->is_running, false);
        return GS_SUCCESS;
    } else {
        GS_DEBUG("shell %p is already shutting down", shell);
        return GS_FAILED;
    }
}

 int shell_loop(void *vargs)
{
    assert (vargs);

    gs_shell_loop_args_t *args = (gs_shell_loop_args_t *) vargs;

    gs_shell_t   *shell = args->shell;
    gs_system_t  *system = args->system;
    apr_file_t   *in, *out;
    char          buffer[STDIN_BUFFER_SIZE];
    bool          accept_input = true;

    gs_error_if((apr_file_open_stdin(&in, shell->pool) != APR_SUCCESS), err_no_stdin);
    gs_error_if((apr_file_open_stdout(&out, shell->pool) != APR_SUCCESS), err_no_stdout);

    GS_DEBUG("shell %p enters main loop", shell);
    apr_file_printf(out, "Type 'help' for hints on usage of this shell.\n\n");
    while (atomic_load(&shell->is_running)) {
        if (accept_input) {
            apr_file_printf(out, "gecko$ ");
            apr_file_gets(buffer, STDIN_BUFFER_SIZE, in);
            accept_input = process_input(system, shell, in, out, buffer);
            apr_file_flush(out);
        }
    }
    GS_DEBUG("shell %p left main loop", shell);
    atomic_store(&shell->is_disposable, true);

    free (args);
    return EXIT_SUCCESS;
}

 bool process_input(gs_system_t *system, gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input)
{
    if (apr_strnatcasecmp(input, "help") == 0) {
        process_command_help(shell, in, out, input);
    } else if (apr_strnatcasecmp(input, "exit") == 0) {
        process_command_exit(system, shell, in, out, input);
        return false;
    } else if (apr_strnatcasecmp(input, "uptime") == 0) {
        process_command_uptime(shell, in, out, input);
    } else if (apr_strnatcasecmp(input, "port info") == 0) {
        process_command_port_info(shell, in, out, input);
    } else {
        apr_file_printf(out, "no such command: %s", input);
    }
    return true;
}

 void process_command_help(gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input)
{
    GS_DEBUG("shell %p command 'help' entered", shell);
    apr_file_printf(out, "Available commands:\n");
    apr_file_printf(out, "  help        Shows this help message\n");
    apr_file_printf(out, "  uptime      Shows how long the system is running\n");
    apr_file_printf(out, "  port info   Lists socket configuration of this service\n");
    apr_file_printf(out, "  exit        Shutdowns the system, and exit the shell\n");

}

 void process_command_exit(gs_system_t *system, gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input)
{
    GS_DEBUG("shell %p command 'exit' entered", shell);
    gs_dispatcher_publish(gs_system_get_dispatcher(shell->system),
                          gs_event_system_exit(system, gs_system_get_dispatcher(shell->system),
                                              GS_OBJECT_TYPE_SHELL, shell));
}

 void process_command_uptime(gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input)
{
    GS_DEBUG("shell %p command 'uptime' entered", shell);
    apr_time_exp_t result;
    apr_time_exp_gmt(&result, apr_time_now() - shell->uptime);
    printf("%dh, %dmin, %dsec\n", result.tm_hour, result.tm_min, result.tm_sec);
}

void process_command_port_info(gs_shell_t *shell, apr_file_t *in, apr_file_t *out, char *input)
{
    GS_DEBUG("shell %p command 'port info' entered", shell);
    gs_server_pool_t *pool = gs_system_get_server_pool(shell->system);
    gs_server_pool_print(stdout, pool);
}