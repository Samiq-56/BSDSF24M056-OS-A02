/*
* Programming Assignment 02: ls
* Version 1.1.0: long listing format (-l)
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define SPACING 2   /* spaces between columns */

extern int errno;

void do_ls(const char *dir);
void do_ls_long(const char *dir);
void mode_to_string(mode_t mode, char *str);

int main(int argc, char *argv[])
{
    int long_flag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        switch (opt)
        {
        case 'l':
            long_flag = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [dir...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        if (long_flag)
            do_ls_long(".");
        else
            do_ls(".");
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s : \n", argv[i]);
            if (long_flag)
                do_ls_long(argv[i]);
            else
                do_ls(argv[i]);
            puts("");
        }
    }
    return 0;
}

/* Terminal width via ioctl(TIOCGWINSZ); fall back to 80 if unavailable */
static int get_terminal_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
        return 80;
    return ws.ws_col;
}

/* Print names in "down then across" order.
 * Item in row r, column c is names[r + c * rows]. */
static void print_columns(char **names, size_t count, size_t maxlen)
{
    int term_width = get_terminal_width();
    size_t col_width = maxlen + SPACING;

    size_t cols = (size_t)term_width / col_width;
    if (cols == 0)
        cols = 1;                            /* very long name */
    size_t rows = (count + cols - 1) / cols; /* ceiling division */

    for (size_t r = 0; r < rows; r++)
    {
        for (size_t c = 0; c < cols; c++)
        {
            size_t idx = r + c * rows;
            if (idx >= count)
                break;

            /* pad only if another item follows in this row */
            if (idx + rows < count)
                printf("%-*s", (int)col_width, names[idx]);
            else
                printf("%s", names[idx]);
        }
        printf("\n");
    }
}

void do_ls(const char *dir)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }

    char **names = NULL;   /* dynamic array of filenames */
    size_t count = 0;      /* how many names are stored */
    size_t capacity = 0;   /* how many slots are allocated */
    size_t maxlen = 0;     /* length of the longest filename */
    int failed = 0;
    struct dirent *entry;

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        /* array full: double its capacity (start with 16 slots) */
        if (count == capacity)
        {
            size_t newcap = capacity ? capacity * 2 : 16;
            char **tmp = realloc(names, newcap * sizeof(char *));
            if (tmp == NULL)
            {
                perror("realloc failed");
                failed = 1;
                break;
            }
            names = tmp;
            capacity = newcap;
        }

        /* store a private copy of the name */
        names[count] = strdup(entry->d_name);
        if (names[count] == NULL)
        {
            perror("strdup failed");
            failed = 1;
            break;
        }

        /* track the longest filename */
        size_t len = strlen(names[count]);
        if (len > maxlen)
            maxlen = len;
        count++;
    }

    if (!failed && errno != 0)
        perror("readdir failed");

    closedir(dp);

    if (count > 0)
        print_columns(names, count, maxlen);

    for (size_t i = 0; i < count; i++)
        free(names[i]);
    free(names);
}

/* Convert st_mode into a string like "drwxr-xr-x" (str must hold 11 bytes) */
void mode_to_string(mode_t mode, char *str)
{
    /* 1) file type: first character */
    if (S_ISREG(mode))       str[0] = '-';
    else if (S_ISDIR(mode))  str[0] = 'd';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else if (S_ISCHR(mode))  str[0] = 'c';
    else if (S_ISBLK(mode))  str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else                     str[0] = '?';

    /* 2) permission bits: owner, group, others */
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';

    /* 3) special bits: setuid, setgid, sticky */
    if (mode & S_ISUID) str[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) str[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) str[9] = (mode & S_IXOTH) ? 't' : 'T';

    str[10] = '\0';
}

/* Resolve uid/gid to names; fall back to the number if no name exists */
static const char *get_user(uid_t uid, char *buf, size_t n)
{
    struct passwd *pw = getpwuid(uid);
    if (pw != NULL)
        return pw->pw_name;
    snprintf(buf, n, "%u", (unsigned)uid);
    return buf;
}

static const char *get_group(gid_t gid, char *buf, size_t n)
{
    struct group *gr = getgrgid(gid);
    if (gr != NULL)
        return gr->gr_name;
    snprintf(buf, n, "%u", (unsigned)gid);
    return buf;
}

static int num_width(long long v)
{
    return snprintf(NULL, 0, "%lld", v);
}

void do_ls_long(const char *dir)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory : %s\n", dir);
        return;
    }

    struct dirent *entry;
    struct stat st;
    char path[PATH_MAX];
    char ubuf[32], gbuf[32];
    long long total = 0;
    int w_link = 0, w_user = 0, w_group = 0, w_size = 0;
    int w;

    /* ---- Pass 1: find column widths and the "total" blocks ---- */
    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (lstat(path, &st) == -1)
        {
            perror(path);
            continue;
        }
        total += st.st_blocks;

        w = num_width(st.st_nlink);
        if (w > w_link) w_link = w;
        w = (int)strlen(get_user(st.st_uid, ubuf, sizeof(ubuf)));
        if (w > w_user) w_user = w;
        w = (int)strlen(get_group(st.st_gid, gbuf, sizeof(gbuf)));
        if (w > w_group) w_group = w;
        w = num_width(st.st_size);
        if (w > w_size) w_size = w;
    }
    if (errno != 0)
        perror("readdir failed");

    /* st_blocks is in 512-byte units, ls prints 1K blocks */
    printf("total %lld\n", total / 2);

    /* ---- Pass 2: print every entry ---- */
    rewinddir(dp);
    time_t now = time(NULL);
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (lstat(path, &st) == -1)
            continue;

        char perms[11];
        mode_to_string(st.st_mode, perms);

        /* ctime() gives "Mon Oct 12 02:59:11 2026\n" */
        char *t = ctime(&st.st_mtime);
        char timebuf[32];
        if (now - st.st_mtime > 15778476 || st.st_mtime > now)
            snprintf(timebuf, sizeof(timebuf), "%.6s  %.4s", t + 4, t + 20);
        else
            snprintf(timebuf, sizeof(timebuf), "%.12s", t + 4);

        printf("%s %*lu %-*s %-*s %*lld %s %s",
               perms,
               w_link, (unsigned long)st.st_nlink,
               w_user, get_user(st.st_uid, ubuf, sizeof(ubuf)),
               w_group, get_group(st.st_gid, gbuf, sizeof(gbuf)),
               w_size, (long long)st.st_size,
               timebuf,
               entry->d_name);

        /* symbolic link: show "-> target" */
        if (S_ISLNK(st.st_mode))
        {
            char target[PATH_MAX];
            ssize_t len = readlink(path, target, sizeof(target) - 1);
            if (len != -1)
            {
                target[len] = '\0';
                printf(" -> %s", target);
            }
        }
        printf("\n");
    }

    closedir(dp);
}
