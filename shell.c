#include <shell.h>
#include <curses.h>
#include <string.h>
#include <mondrian.h>

static bool loader_is_running = true;
static bool shell_is_runnnig = true;

#define GRIDSTORE_VERSION_STR "0.0.0-0001-vanilla"

static mondrian_t *GRIDSTORE_INSTANCE;

// *** DEBUG INFORMATION AND NON-UNIX SYSTEMS *** uncomment this line to disable curses terminal
//#define NCURSES

// ---------------------------------------------------------------------------------------------------------------------
size_t BUFFER_MAX = 2048;

#ifdef __GNUC__
    #define SUPRESS_UNUSED(x)       UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
    #define SUPRESS_UNUSED(x)       /*@unused@*/ x
#else
    #define SUPRESS_UNUSED(x)       x
#endif

#ifndef NCURSES
    #define INITCR() initscr();

    #define SETUP_COLOR_PAIRS()                                                                                            \
    {                                                                                                                      \
        start_color();                                                                                                     \
        use_default_colors();                                                                                              \
        keypad(stdscr, TRUE);                                                                                              \
        init_pair(COLOR_PAIR_SHELL_DEFAULT,  COLOR_WHITE, COLOR_BLUE);                                                     \
        init_pair(COLOR_PAIR_TEXT_HIGHLIGHT, COLOR_BLACK, COLOR_WHITE);                                                    \
    }

    #define PRINTW(...) printw(__VA_ARGS__)

    #define PRINTW_ARGS(format, ...)             printw(format, __VA_ARGS__)

    #define HALFDELAY(...) halfdelay(__VA_ARGS__)

    #define GETCH() getch()

    #define ENDWIN() endwin()

    #define CLEAR() clear()
    #define NOECHO() noecho()
    #define ECHO()  echo()

    #define destroy_window(win)                                         \
    {                                                                   \
        wborder(*win, ' ', ' ', ' ',' ',' ',' ',' ',' ');               \
        wrefresh(*win);                                                 \
        delwin(*win);                                                   \
        *win = NULL;                                                    \
    }

    #define print_bold(win, text)                                       \
    {                                                                   \
        wattron(win, A_BOLD);                                           \
        wprintw(win, text);                                             \
        wattroff(win, A_BOLD);                                          \
    }

    #define print_high(win, text)                   \
    {                                                                   \
        wattron(win, COLOR_PAIR(COLOR_PAIR_TEXT_HIGHLIGHT));            \
        wprintw(win, text);                                             \
        wattroff(win, COLOR_PAIR(COLOR_PAIR_TEXT_HIGHLIGHT));           \
    }

    #define DECLARE(x)          x                        \

    #define create_window_status_bar(window_name, ncommands, keys, desc)    \
    ({                                                                      \
        WINDOW *win;                                                        \
        win = newwin(2, COLS, LINES - 1, 0);                                \
        print_bold(win, window_name);                                       \
        print_bold(win, " ");                                               \
        for (int i = 0; i < ncommands; i++) {                               \
            print_high(win, keys[i]); wprintw(win, " %s ", desc[i]);        \
        }                                                                   \
        wrefresh(win);                                                      \
        win;                                                                \
    })

    #define show_status_bar(/*const char* */window_name, /*int */ncommands, /*const char **/keys/*[]*/, /*const char **/desc/*[]*/) \
    {                                                                                                                      \
        refresh();                                                                                                         \
        if (window_status_bar == NULL)                                                                                     \
            window_status_bar = create_window_status_bar(window_name, ncommands, keys, desc);                              \
    }

    #define create_window_prompt_border()                   \
    ({                                                      \
        refresh();                                          \
        const char header[] = "grid store";                 \
        WINDOW *win;                                        \
        win = newwin(LINES - 1, COLS, 0, 0);                \
        box(win, 0 , 0);                                                    \
        wattron(win, COLOR_PAIR(COLOR_PAIR_TEXT_HIGHLIGHT));                    \
        mvwprintw(win, 0, (COLS - strlen(header)) / 2 - 2, " %s ", header); \
        wattroff(win, COLOR_PAIR(COLOR_PAIR_TEXT_HIGHLIGHT));               \
        wrefresh(win);                                                      \
        win;                                                                \
    })

    #define create_window_prompt()                      \
    ({                                                  \
        refresh();                                      \
        WINDOW *win;                                    \
        win = newwin(LINES - 3, COLS - 3, 1, 2);        \
        wrefresh(win);                                  \
        win;                                            \
    })

    #define show_promt_window()                                         \
    {                                                                                  \
        if (prompt_window == NULL) {                                            \
            prompt_window_border = create_window_prompt_border();               \
            prompt_window = create_window_prompt();                             \
            WPRINTW(prompt_window, "Welcome to GridStore. Type 'help' to get an overview on available commands.\n");    \
            print_h_line();                                                     \
        }                                                                       \
    }

    #define SCROLLOK(...) scrollok(__VA_ARGS__)
    #define WPRINTW(window, text) wprintw(window, text)
    #define WPRINTW_ARGS(window, text, ...) wprintw(window, text, __VA_ARGS__)
    #define WGETSTR(window, input) wgetstr(window, input)
    #define WLCLEAR(window) wclear(window)
#else
    #define INITCR()                { }

    #define SETUP_COLOR_PAIRS()     { }

    #define PRINTW(...)             printf("%s", __VA_ARGS__)

    #define PRINTW_ARGS(format, ...)             printf(format, __VA_ARGS__)

    #define HALFDELAY(...)          { }

    #define GETCH()                 getchar()

    #define ENDWIN()                { }

    #define CLEAR()                 { }
    #define NOECHO()                { }
    #define ECHO()                  { }

    #define destroy_window(win)     { }

    #define print_bold(win, text)   printf(text)

    #define print_high(win, text)   printf(text)

    #define DECLARE(x)

    #define create_window_status_bar(window_name, ncommands, keys, desc)   { }

    #define show_status_bar(/*const char* */window_name, /*int */ncommands, /*const char **/keys/*[]*/, /*const char **/desc/*[]*/) { }

    #define create_window_prompt_border()                   { }

    #define create_window_prompt()                          { }

    #define show_promt_window()                             { }

    #define SCROLLOK(...)                                   { }
    #define WPRINTW(window, text)                           printf("%s", text)
    #define WPRINTW_ARGS(window, format, ...)               printf(format, __VA_ARGS__)
    #define WGETSTR(window, input)                          fgets(input, BUFFER_MAX, stdin)
    #define WLCLEAR(window)                                 { }
#endif

// ---------------------------------------------------------------------------------------------------------------------

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

// ---------------------------------------------------------------------------------------------------------------------

enum loader_state {
    LS_STARTING,
    LS_ABORT,
    LS_RUNNING,
    LS_SHUTDOWN,
    LS_ENDED
} loader_state;

static inline void switch_loader_state(enum loader_state state) {
    loader_state = state;
}

static inline void loader_exit() {
    loader_is_running = false;
}

// ---------------------------------------------------------------------------------------------------------------------

enum shell_state {
    SS_WINDOW_NONE,
    SS_WINDOW_INFO,
    SS_WINDOW_QUERY_SQL,
    SS_WINDOW_COMPILED_QUERY_CACHE,
    SS_WINDOW_DATABASES,
    SS_WINDOW_TABLE,
    SS_WINDOW_SHUTDOWN,
    SS_ABORT
} shell_state;

static inline void switch_shell_state(enum shell_state state) {
    shell_state = state;
}

static inline void shell_exit() {
    shell_is_runnnig = false;
}

//----------------------------------------------------------------------------------------------------------------------

DECLARE(WINDOW *prompt_window = NULL);
DECLARE(WINDOW *prompt_window_border = NULL);

// ---------------------------------------------------------------------------------------------------------------------

#define COMMAND_SUCCESS ""
#define COMMAND_ILLEGALARGS "Illegal arguments for this command"

const char *cmd_exit(const char *args);
void cmd_exit_usage();

const char *cmd_enter(const char *args);
void cmd_enter_usage();

const char *cmd_version(const char *args);
void cmd_version_usage();

struct shell_command_t{
    const char cmd_name[256];
    const char cmd_desc[1024];
    const char * (*exec)(const char *args);
    void (*show_usage)();
} shell_commands[] = {
    {"enter",   "enters a subsystem shell, e.g. SQL",            cmd_enter,   cmd_enter_usage},
    {"exit",    "ends this session and shutdowns grid store",    cmd_exit,    cmd_exit_usage},
    {"version", "displays version information",                  cmd_version, cmd_version_usage}
};

const char *cmd_enter(const char *args)
{
    if (args == NULL || strlen(args) > 0) {
        WPRINTW(prompt_window, "and now enter SQL parser...\n");
        return COMMAND_SUCCESS;
    } else return COMMAND_ILLEGALARGS;
}

void cmd_enter_usage()
{
    WPRINTW(prompt_window, "usage: enter <sub-system>\n\n"
                           "Enters a sub-system interface defined by <sub-system>.\n"
                           "The following sub-systems are available:\n"
                           " * 'sql-ddl'\tSQL data definition sub system to manipulate tables");
}

const char *cmd_exit(const char *args)
{
    if (args == NULL || strlen(args) == 0) {
        shell_exit();
        return COMMAND_SUCCESS;
    } else return COMMAND_ILLEGALARGS;
}

void cmd_exit_usage()
{
    WPRINTW(prompt_window, "exit");
}

void print_version_information()
{
    WPRINTW(prompt_window, "grid store " GRIDSTORE_VERSION_STR " ");
    WPRINTW_ARGS(prompt_window, "%s %s\n", __DATE__, __TIME__);
}

const char *cmd_version(const char *args)
{
    if (args == NULL || strlen(args) == 0) {
        WPRINTW(prompt_window, "grid store " GRIDSTORE_VERSION_STR " ");
        WPRINTW_ARGS(prompt_window, "%s %s\n", __DATE__, __TIME__);
        return COMMAND_SUCCESS;
    } else return COMMAND_ILLEGALARGS;
}

void cmd_version_usage()
{
    WPRINTW(prompt_window, "version");
}

// ---------------------------------------------------------------------------------------------------------------------

static int start_shell();

#define COLOR_PAIR_SHELL_DEFAULT     1
#define COLOR_PAIR_TEXT_HIGHLIGHT    2


static bool loader_run_startup() {
    PRINTW("Loader is starting...\n");
    SETUP_COLOR_PAIRS();

    mondrian_open(&GRIDSTORE_INSTANCE);

    return true;
}

static bool loader_run_main() {
    PRINTW("Entering shell...\n");
    start_shell();
    return true;
}

static bool loader_run_shutdown() {
    PRINTW("Loader is shutting down...\n");
    mondrian_close(GRIDSTORE_INSTANCE);
    return true;
}

static bool loader_show_end_message() {
    PRINTW("Bye...\n");
    return true;
}

static bool loader_show_abort_message() {
    PRINTW("Loader was aborted.\n");
    return true;
}

static bool loader_show_internal_failure_message() {
    PRINTW("Loader was terminated since it runs into an illegal state.\nABORT.\n");
    return true;
}

void gs_shell_start(const char *connection_str)
{
    bool success;
    INITCR();

    switch_loader_state(LS_STARTING);
    while (loader_is_running) {
        switch (loader_state) {
            case LS_STARTING:
                success = loader_run_startup();
                switch_loader_state(success ? LS_RUNNING : LS_ABORT);
                break;
            case LS_RUNNING:
                success = loader_run_main();
                switch_loader_state(success ? LS_SHUTDOWN : LS_ABORT);
                break;
            case LS_SHUTDOWN:
                success = loader_run_shutdown();
                switch_loader_state(success ? LS_ENDED : LS_ABORT);
                break;
            case LS_ENDED:
                loader_show_end_message();
                loader_exit();
                break;
            case LS_ABORT:
                loader_show_abort_message();
                loader_exit();
                break;
            default:
                loader_show_internal_failure_message();
                loader_exit();
                break;
        }
    }

    HALFDELAY(10);
    GETCH();
    ENDWIN();
}

// ---------------------------------------------------------------------------------------------------------------------

int LAST_CHAR;
static inline int input_key_get() {
    return LAST_CHAR;
}

static inline void input_key_reset() {
    LAST_CHAR = UINT32_MAX;
}

// ---------------------------------------------------------------------------------------------------------------------

void show_shell_window_info();
void show_shell_window_prompt();
void show_shell_window_query_sql();
void show_shell_window_compiled_query_cache();
void show_shell_window_databases();
void show_shell_window_table();
int show_shell_window_shutdown();

void print_h_line()
{
    for (size_t i = 0; i < COLS - 5; i++)
        WPRINTW(prompt_window, "-");
    WPRINTW(prompt_window, "\n");
}


int start_shell()
{
    CLEAR();
    int return_value = EXIT_FAILURE;
    switch_shell_state(SS_WINDOW_NONE);

    while (shell_is_runnnig) {

        show_shell_window_prompt();

        switch (shell_state) {
            case SS_WINDOW_INFO:
                show_shell_window_info();
                break;
            case SS_WINDOW_NONE:

                break;
            case SS_WINDOW_QUERY_SQL:
                show_shell_window_query_sql();
                break;
            case SS_WINDOW_COMPILED_QUERY_CACHE:
                show_shell_window_compiled_query_cache();
                break;
            case SS_WINDOW_DATABASES:
                show_shell_window_databases();
                break;
            case SS_WINDOW_TABLE:
                show_shell_window_table();
                break;
            case SS_WINDOW_SHUTDOWN:
                return_value = show_shell_window_shutdown();
                shell_exit();
                break;
            case SS_ABORT: default:
                loader_exit();
                return_value = EXIT_FAILURE;
                break;
        }

#ifndef NCURSES
        NOECHO();
        HALFDELAY(1);
        LAST_CHAR = GETCH();
        ECHO();
#endif
    }
    return return_value;
}

void shell_shutdown()
{
    switch_shell_state(SS_WINDOW_SHUTDOWN);
}

//----------------------------------------------------------------------------------------------------------------------

DECLARE(WINDOW *window_status_bar = NULL);

void process_ctrl_commands()
{
    switch (input_key_get()) {
        case CTRL('x'):
            input_key_reset();
            destroy_window(&window_status_bar);
            shell_shutdown();
            break;
    }
}

//----------------------------------------------------------------------------------------------------------------------

void show_shell_window_info()
{

}

//----------------------------------------------------------------------------------------------------------------------

bool starts_with(const char *str, const char *prefix)
{
    size_t lenpre = strlen(prefix), lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(prefix, str, lenpre) == 0;
}

#define MODE_INVOKE_COMMAND 0
#define MODE_MANOF_COMMAND  1

void process_input() {
    SCROLLOK(DECLARE(prompt_window), true);
    WPRINTW(DECLARE(prompt_window), "gs$ ");
    char input[BUFFER_MAX];
#ifdef NOCURSE
    WGETSTR(DECLARE(prompt_window), input);
#else
    WGETSTR(DECLARE(prompt_window), &input[0]);
#endif


    if (strlen(input) == 1 && input[0] == CTRL('X')) {
        LAST_CHAR = input[0];
        process_ctrl_commands();
    } else {
        char *token, *tofree;
        size_t num_commands = sizeof(shell_commands) / sizeof(shell_commands[0]);

        token = tofree = strdup(&input[0]);
        size_t token_len = strlen(token);
        if (token_len > 0 && token[token_len - 1] == '\n')
            token[token_len - 1] = '\0';
        char *command_in = strsep(&token, " ");

        int mode = MODE_INVOKE_COMMAND;
        if (command_in != NULL) {
            if (strcmp(command_in, "help") == 0) {
                if (token == NULL || strlen(token) == 0) {
                    WLCLEAR(prompt_window);
                    WPRINTW(prompt_window, "Command Help\n");
                    print_h_line();

                    for (size_t i = 0; i < num_commands; i++) {
                        WPRINTW_ARGS(prompt_window, " * %s", shell_commands[i].cmd_name);
                        WPRINTW_ARGS(prompt_window, ": %s\n", shell_commands[i].cmd_desc);
                    }

                    print_h_line();
                    WPRINTW(prompt_window,
                            "\nFor more information on the usage of a specific command, run 'help <command>'.\n\n");
                    return;
                } else {
                    mode = MODE_MANOF_COMMAND;
                }
            }
        }

        /* scan for registered commands */
        bool handled = false;
        command_in = (mode == MODE_MANOF_COMMAND ? token : command_in);
        for (size_t i = 0; i < num_commands; i++) {
            if (command_in != NULL && strcmp(command_in, shell_commands[i].cmd_name) == 0) {
                const char *result;
                switch (mode) {
                    case MODE_INVOKE_COMMAND:
                        result = shell_commands[i].exec(token);
                        if (strcmp(result, COMMAND_SUCCESS) != 0) {
                            WPRINTW(prompt_window, result);
                        }
                        break;
                    case MODE_MANOF_COMMAND:
                        shell_commands[i].show_usage();
                        break;
                }
                WPRINTW(prompt_window, "\n");
                handled = true;
            }
            if (handled)
                break;
        }
        if (!handled) {
            if (mode == MODE_INVOKE_COMMAND)
                WPRINTW_ARGS(prompt_window, "unknown command: %s\n", input);
            else
                WPRINTW(prompt_window, "no command to shown for 'man'. Try 'help'\n");
        }
        free(tofree);
    }
}

void show_shell_window_prompt()
{
    const char *keys[] = { "^X",   "^T"};
    const char *desc[] = { "Exit", "Tables"};

#pragma unused (keys)
#pragma unused (desc)

    show_promt_window();
    show_status_bar("shell ", 2, keys, desc);
    process_input();
}

void show_shell_window_query_sql()
{

}

void show_shell_window_compiled_query_cache()
{

}

void show_shell_window_databases()
{

}

void show_shell_window_table()
{

}

int show_shell_window_shutdown()
{
    return EXIT_SUCCESS;
}



