k#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>

// Function declarations
void print_long_format(const char *dirname, char **filenames, int count);
void print_horizontal_format(char **filenames, int count, size_t max_len);
void print_vertical_format(char **filenames, int count, size_t max_len);
int compare_strings(const void *a, const void *b);

void do_ls(const char *dirname, int long_format, int horizontal_format) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("ls: cannot open directory");
        return;
    }

    struct dirent *entry;
    int count = 0;
    int capacity = 10;
    
    char **filenames = malloc(capacity * sizeof(char *));
    if (filenames == NULL) {
        perror("malloc failed");
        closedir(dir);
        return;
    }

    size_t max_len = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue; // Skip hidden files
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

    // Task 2: Sort filenames alphabetically using qsort
    qsort(filenames, count, sizeof(char *), compare_strings);

    // Task 3: Integrate Sorted Array into Display Modes
    if (long_format) {
        print_long_format(dirname, filenames, count);
    } else if (horizontal_format) {
        print_horizontal_format(filenames, count, max_len);
    } else {
        print_vertical_format(filenames, count, max_len);
    }

    for (int i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

// Comparison function for qsort (Task 2)
int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
}

// Long Listing Format (-l)
void print_long_format(const char *dirname, char **filenames, int count) {
    for (int i = 0; i < count; i++) {
        struct stat file_stat;
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dirname, filenames[i]);
        
        if (lstat(filepath, &file_stat) == 0) {
            printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
            printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");

            printf(" %3ld", (long)file_stat.st_nlink);
            struct passwd *pw = getpwuid(file_stat.st_uid);
            struct group *gr = getgrgid(file_stat.st_gid);
            printf(" %s  %s", (pw ? pw->pw_name : "unknown"), (gr ? gr->gr_name : "unknown"));
            printf(" %8ld", (long)file_stat.st_size);
            
            char time_buf[80];
            struct tm *timeinfo = localtime(&file_stat.st_mtime);
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
            printf(" %s %s\n", time_buf, filenames[i]);
        }
    }
}

// Horizontal Display Logic (-x)
void print_horizontal_format(char **filenames, int count, size_t max_len) {
    int terminal_width = 80;
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (w.ws_col > 0) {
            terminal_width = w.ws_col;
        }
    }

    int spacing = 2;
    int col_width = max_len + spacing;
    int current_pos = 0;

    for (int i = 0; i < count; i++) {
        if (current_pos + col_width > terminal_width && current_pos > 0) {
            printf("\n");
            current_pos = 0;
        }

        if (i == count - 1) {
            printf("%s", filenames[i]);
        } else {
            printf("%-*s", col_width, filenames[i]);
        }
        current_pos += col_width;
    }
    printf("\n");
}

// Default Vertical "Down Then Across" Format
void print_vertical_format(char **filenames, int count, size_t max_len) {
    int terminal_width = 80;
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (w.ws_col > 0) {
            terminal_width = w.ws_col;
        }
    }

    int spacing = 2;
    int col_width = max_len + spacing;
    int num_cols = terminal_width / col_width;
    if (num_cols < 1) {
        num_cols = 1;
    }

    int num_rows = (count + num_cols - 1) / num_cols;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int index = c * num_rows + r;
            if (index < count) {
                if (c == num_cols - 1) {
                    printf("%s", filenames[index]);
                } else {
                    printf("%-*s", col_width, filenames[index]);
                }
            }
        }
        printf("\n");
    }
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
