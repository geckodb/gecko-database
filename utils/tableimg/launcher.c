#include <stdio.h>
#include <stdlib.h>
#include <defs.h>
#include <fcntl.h>
#include <errno.h>
#include <tableimg.h>
#include <time.h>
#include <ctype.h>
#include <curses.h>

void show_version();
void show_usage();
void show_file_info(char *file);
void show_table_head(size_t limit, char *file);

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        show_version();
    } else if (argc == 3 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--info") == 0)) {
        show_file_info(argv[2]);
    } else if (argc == 4 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--head") == 0)) {
        show_table_head(atoi(argv[2]), argv[3]);
    } else {
        show_usage();
    }
    return EXIT_SUCCESS;
}

void show_version()
{
    printf("tableimg version 1.0.00 ((c) 2017 Marcus Pinnecke)\n"
           "The tool tableimg is used to view or manipulate table image files (*timg).\n");
}

void show_usage()
{
    printf("usage: tableimg [--version] [-i | --info <file>] [-h | --head <file>]");
}

void show_file_info(char *file)
{
    FILE *file_ptr = fopen(file, "rb");
    if (!file_ptr) {
        perror("Unable to open input file.");
        exit(EXIT_FAILURE);
    } else {
        timg_header_t header;
        tableimg_header(file_ptr, &header);

        if (header.format_version != TIMG_VER_1) {
            perror("Unsupported format version.");
            exit(EXIT_FAILURE);
        }

        time_t t = header.timestamp_created;
        char buf[80];
        struct tm* st = localtime(&t);
        strftime(buf, 80, "%c", st);


        printf("================================================================================\n");
        printf(" *** %s/%s *** (IMAGE)\n", header.database_name, header.table_name);
        printf("================================================================================\n");
        printf("%llu tuples (%llu fields) are stored in this image.\n",
               header.num_tuples, header.num_tuples * header.num_attributes);
        printf("\n");
        printf("META\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("specification......: %s\n", header.table_spec_ref);
        printf("content size.......: %0.6fG\n", (tableimg_rawsize(&header)/1024.0/1024.0/1024.0));
        printf("serialization......: %s\n", header.flags.serial_format_type == TIMG_FORMAT_NSM ? "row-wise" : "columnar");
        printf("comment............: %s\n", header.comment);
        printf("md5 checksum.......: ");
        for(int i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", header.raw_table_data_checksum[i]);
        printf("\n\n");
        printf("SCHEMA\n");
        printf("--------------------------------------------------------------------------------\n");
        for (int i = 0; i < header.num_attributes; i++) {
            timg_attr_t attr = header.attributes[i];
            printf("attribute name.....: %s\n", attr.name);
            if (attr.data_type_rep > 1)
                printf("data type..........: %s (%zu)\n", tableimg_datatype_str(attr.data_type), attr.data_type_rep);
            else
                printf("data type..........: %s\n", tableimg_datatype_str(attr.data_type));
            printf("primary/foreign....: %d/%d\n",
                   attr.attr_flags.is_primary_key, attr.attr_flags.is_foreign_key);
            printf("unique/null/inc....: %d/%d/%d\n",
                   attr.attr_flags.is_unique, attr.attr_flags.is_nullable,
                   attr.attr_flags.is_autoinc);
            printf("md5 checksum.......: ");
            for(int i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", attr.checksum[i]);
            printf("\n");
            printf("--------------------------------------------------------------------------------\n");
        }
        printf("image format version %d; created on %s\n", header.format_version, buf);
    }
}

void quit()
{
    endwin();
}

void handle_winch(int sig){
    signal(SIGWINCH, SIG_IGN);
    // Reinitialize the window to update data structures.
    endwin();
    initscr();
    refresh();
    clear();

    char tmp[128];
    sprintf(tmp, "%dx%d", COLS, LINES);

    init_pair (1, COLOR_RED, COLOR_CYAN);
    WINDOW *win = newwin(20, 20, 20, 20);
    wattron(win,COLOR_PAIR(1));
    box(win,0,0);
    wattroff(win,COLOR_PAIR(1));
    wrefresh(win);

    // Approximate the center
    int x = COLS / 2 - strlen(tmp) / 2;
    int y = LINES / 2 - 1;

    mvaddstr(y, x, tmp);
    refresh();

    signal(SIGWINCH, handle_winch);
}

void show_table_head(size_t limit, char *file) {
    FILE *file_ptr = fopen(file, "rb");
    if (!file_ptr) {
        perror("Unable to open input file.");
        exit(EXIT_FAILURE);
    } else {
        timg_header_t header;
        tableimg_header(file_ptr, &header);

        if (header.format_version != TIMG_VER_1) {
            perror("Unsupported format version.");
            exit(EXIT_FAILURE);
        }

      /*  initscr();
        if (has_colors()) {
            use_default_colors();
            start_color();
            init_pair(1, COLOR_BLACK, COLOR_WHITE);
        }

        WINDOW *win_body = newwin(24, 80, 0, 0);
        WINDOW *win_form = derwin(win_body, 20, 78, 3, 1);
        box(win_body, 0, 0);
        box(win_form, 0, 0);
        mvwprintw(win_body, 1, 2, "Press F1 to quit and F2 to print fields content");

        attrset(COLOR_PAIR(1));
        printw("  %s/%s", header.database_name, header.table_name);
        printw(" in file (%s)", file);


        getch();
        endwin();*/



        signal(SIGWINCH, handle_winch);
        refresh();

        while(getch() != 27){
            /* Nada */
        }

        endwin();

    }
}