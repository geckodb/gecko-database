

#include <stdlib.h>
#include <printf.h>
#include <memory.h>
#include <shell.h>

#define CMD_SHELL   "shell"

static void show_usage();

static void start_shell();

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        show_usage();
    } else if (argc == 2) {
        const char *command = argv[1];
        if (strcmp(command, CMD_SHELL) == 0) {
            start_shell();
        } else show_usage();
    }


    return EXIT_SUCCESS;
}

static void show_usage()
{
    printf("usage: gs [-v | --version] <command> [<args>]\n\n");
    printf("These are common gs commands:\n\n");
    printf("\tshell\t\tStarts gs with an interactive shell.\n");
}

static void start_shell()
{
    gs_shell_start();
}
