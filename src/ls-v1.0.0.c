#include <stdio.h>
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

/* ANSI escape codes for colors */
#define COLOR_RESET    "\033[0m"
#define COLOR_BLUE     "\033[0;34m"   /* directories */
#define COLOR_GREEN    "\033[0;32m"   /* executables */
#define COLOR_RED      "\033[0;31m"   /* tarballs: .tar .gz .zip */
#define COLOR_PINK     "\033[0;35m"   /* symbolic links */
#define COLOR_REVERSE  "\033[7m"      /* special files (devices, sockets) */

void print_long_format(const char *dirname, char **filenames, int count);
void print_horizontal_format(char **filenames, int count, int term_width, const char *dirname);
void print_column_format(char **filenames, int count, int term_width, const char *dirname);
void do_ls(const char *dirname, int long_format, int horizontal_format, int recursive_format);
void print_colored(const char *name, const char *color);

/* Print a name wrapped in a color code, then reset the color. */
void print_colored(const char *name, const char *color) {
    printf("%s%s%s", color, name, COLOR_RESET);
}

int compare_strings(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2);
}

/* Helper to determine color based on file type/name */
void print_file_with_color(const char *fullpath, const char *filename) {
    struct stat st;
    char color[32] = "";
    int use_color = 0;

    if (lstat(fullpath, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            snprintf(color, sizeof(color), "%s", COLOR_BLUE);
            use_color = 1;
        } else if (S_ISLNK(st.st_mode)) {
            snprintf(color, sizeof(color), "%s", COLOR_PINK);
            use_color = 1;
        } else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) {
            snprintf(color, sizeof(color), "%s", COLOR_REVERSE);
            use_color = 1;
        } else if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) {
            snprintf(color, sizeof(color), "%s", COLOR_GREEN);
            use_color = 1;
        } else {
            /* Check extensions for tarballs */
            if (strstr(filename, ".tar") || strstr(filename, ".gz") || strstr(filename, ".zip")) {
                snprintf(color, sizeof(color), "%s", COLOR_RED);
                use_color = 1;
            }
        }
    }

    if (use_color) {
        print_colored(filename, color);
    } else {
        printf("%s", filename);
    }
}

void print_long_format(const char *dirname, char **filenames, int count) {
    struct stat st;
    char fullpath[1024];

    for (int i = 0; i < count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, filenames[i]);
        if (lstat(fullpath, &st) == -1) continue;

        /* Permissions */
        char perms[11] = "----------";
        if (S_ISDIR(st.st_mode)) perms[0] = 'd';
        else if (S_ISLNK(st.st_mode)) perms[0] = 'l';
        else if (S_ISCHR(st.st_mode)) perms[0] = 'c';
        else if (S_ISBLK(st.st_mode)) perms[0] = 'b';
        else if (S_ISFIFO(st.st_mode)) perms[0] = 'p';
        else if (S_ISSOCK(st.st_mode)) perms[0] = 's';

        if (st.st_mode & S_IRUSR) perms[1] = 'r';
        if (st.st_mode & S_IWUSR) perms[2] = 'w';
        if (st.st_mode & S_IXUSR) perms[3] = (st.st_mode & S_ISUID) ? 's' : 'x';
        else if (st.st_mode & S_ISUID) perms[3] = 'S';

        if (st.st_mode & S_IRGRP) perms[4] = 'r';
        if (st.st_mode & S_IWGRP) perms[5] = 'w';
        if (st.st_mode & S_IXGRP) perms[6] = (st.st_mode & S_ISGID) ? 's' : 'x';
        else if (st.st_mode & S_ISGID) perms[6] = 'S';

        if (st.st_mode & S_IROTH) perms[7] = 'r';
        if (st.st_mode & S_IWOTH) perms[8] = 'w';
        if (st.st_mode & S_IXOTH) perms[9] = (st.st_mode & S_ISVTX) ? 't' : 'x';
        else if (st.st_mode & S_ISVTX) perms[9] = 'T';

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        char *time_str = ctime(&st.st_mtime);
        if (time_str) time_str[strlen(time_str) - 1] = '\0';

        printf("%s %3ld %-8s %-8s %8ld %s ", 
            perms, 
            (long)st.st_nlink, 
            pw ? pw->pw_name : "unknown", 
            gr ? gr->gr_name : "unknown", 
            (long)st.st_size, 
            time_str ? time_str : "");

        print_file_with_color(fullpath, filenames[i]);
        printf("\n");
    }
}

void print_horizontal_format(char **filenames, int count, int term_width, const char *dirname) {
    int current_width = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(filenames[i]);
        if (current_width + len + 2 > term_width && current_width > 0) {
            printf("\n");
            current_width = 0;
        }
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, filenames[i]);
        print_file_with_color(fullpath, filenames[i]);
        printf("  ");
        current_width += len + 2;
    }
    printf("\n");
}

void print_column_format(char **filenames, int count, int term_width, const char *dirname) {
    if (count == 0) return;
    int max_len = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(filenames[i]);
        if (len > max_len) max_len = len;
    }
    int col_width = max_len + 2;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;

    int num_rows = (count + num_cols - 1) / num_cols;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int index = c * num_rows + r;
            if (index < count) {
                char fullpath[1024];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, filenames[index]);
                print_file_with_color(fullpath, filenames[index]);
                if (c < num_cols - 1) {
                    int padding = col_width - strlen(filenames[index]);
                    for (int p = 0; p < padding; p++) printf(" ");
                }
            }
        }
        printf("\n");
    }
}

void do_ls(const char *dirname, int long_format, int horizontal_format, int recursive_format) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("ls");
        return;
    }

    struct dirent *entry;
    int capacity = 10;
    int count = 0;
    char **filenames = malloc(capacity * sizeof(char *));

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; /* Skip hidden files */
        if (count >= capacity) {
            capacity *= 2;
            filenames = realloc(filenames, capacity * sizeof(char *));
        }
        filenames[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    qsort(filenames, count, sizeof(char *), compare_strings);

    if (long_format) {
        print_long_format(dirname, filenames, count);
    } else if (horizontal_format) {
        print_horizontal_format(filenames, count, 80, dirname);
    } else {
        print_column_format(filenames, count, 80, dirname);
    }

    /* Handle Recursive Listing (-R) */
    if (recursive_format) {
        for (int i = 0; i < count; i++) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, filenames[i]);

            struct stat st;
            if (lstat(fullpath, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    printf("\n%s:\n", fullpath);
                    do_ls(fullpath, long_format, horizontal_format, recursive_format);
                }
            }
        }
    }

    for (int i = 0; i < count; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

int main(int argc, char *argv[]) {
    int opt, long_format = 0, horizontal_format = 0, recursive_format = 0;

    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l': long_format = 1; break;
            case 'x': horizontal_format = 1; break;
            case 'R': recursive_format = 1; break;
            default: 
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [file...]\n", argv[0]); 
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        do_ls(".", long_format, horizontal_format, recursive_format);
    } else {
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            do_ls(argv[i], long_format, horizontal_format, recursive_format);
            if (i < argc - 1) printf("\n");
        }
    }

    return 0;
}
