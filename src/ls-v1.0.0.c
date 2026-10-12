#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>

void do_ls(const char *dirname, int long_format) {
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

    if (long_format) {
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

                printf(" %3ld", file_stat.st_nlink);
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
    } else {
        // Temporary verification print for Task 2
        for (int i = 0; i < count; i++) {
            printf("%s  ", filenames[i]);
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

    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        do_ls(".", long_format);
    } else {
        for (int i = optind; i < argc; i++) {
            printf("%s:\n", argv[i]);
            do_ls(argv[i], long_format);
        }
    }

    return 0;
}
