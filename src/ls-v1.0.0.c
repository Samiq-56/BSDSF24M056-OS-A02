#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <getopt.h>

void do_ls(const char *dirname, int long_format, int horizontal_format) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("ls: cannot open directory");
        return;
    }

    struct dirent *entry;
    int count = 0;
    int capacity = 10;
    
    // Task 2: Dynamically allocate array of strings to gather all filenames
    char **filenames = malloc(capacity * sizeof(char *));
    if (filenames == NULL) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    size_t max_len = 0;

    // Read all directory entries and track max length
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue; // Skip hidden files for now
        }

        if (count >= capacity) {
            capacity *= 2;
            char **temp = realloc(filenames, capacity * sizeof(char *));
            if (temp == NULL) {
                perror("realloc failed");
                for (int i = 0; i < count; i++) {
                    free(filenames[i]);
                }
                free(filenames);
                closedir(dir);
                return;
            }
            filenames = temp;
        }

        filenames[count] = strdup(entry->d_name);
        
        size_t current_len = strlen(entry->d_name);
        if (current_len > max_len) {
            max_len = current_len;
        }

        count++;
    }

    closedir(dir);

    if (count == 0) {
        free(filenames);
        return;
    }

    // Task 3: Calculate Column Layout
    int terminal_width = 80; // Default fallback width
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (w.ws_col > 0) {
            terminal_width = w.ws_col;
        }
    }

    int spacing = 2; // Spacing between columns
    int col_width = max_len + spacing;
    int num_cols = terminal_width / col_width;
    if (num_cols < 1) {
        num_cols = 1;
    }

    int num_rows = (count + num_cols - 1) / num_cols;

    // Task 4: Implement "Down Then Across" Printing
    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int index = c * num_rows + r;
            if (index < count) {
                if (c == num_cols - 1) {
                    // Last column on the row doesn't need trailing padding
                    printf("%s", filenames[index]);
                } else {
                    printf("%-*s", col_width, filenames[index]);
                }
            }
        }
        printf("\n");
    }

    // Clean up allocated memory
    for (int i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;
    int horizontal_format = 0;

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            case 'x':
                horizontal_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        do_ls(".", long_format, horizontal_format);
    } else {
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            do_ls(argv[i], long_format, horizontal_format);
        }
    }

    return 0;
}
