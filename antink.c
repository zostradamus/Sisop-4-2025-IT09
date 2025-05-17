/*
 * antink.c
 * AntiNK: FUSE-based anomaly detector for “nafis” & “kimcun”
 * - Reverse filenames containing keywords
 * - ROT13‐encrypt normal .txt reads; plain for dangerous files
 * - Log all events to specified logfile
 *
 * Compile with:
 *   gcc -std=gnu11 -Wall -Wextra antink.c -o antink-fuse `pkg-config fuse3 --cflags --libs`
 */

#define FUSE_USE_VERSION 35
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

static const char *root_dir   = "/srv/antiNK/original";
static const char *mount_dir  = "/srv/antiNK/mount";
static const char *log_path   = "/srv/antiNK/logs/it24.log";
static const char *bad_keys[] = { "nafis", "kimcun", NULL };

/* Log a timestamped message */
static void write_log(const char *fmt, ...)
{
    FILE *f = fopen(log_path, "a");
    if (!f) return;
    va_list ap;
    va_start(ap, fmt);

    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
    fprintf(f, "[%s] ", buf);
    vfprintf(f, fmt, ap);
    fprintf(f, "\n");

    va_end(ap);
    fclose(f);
}

/* Build full host‐fs path */
static void fullpath(char out[PATH_MAX], const char *path) {
    snprintf(out, PATH_MAX, "%s%s", root_dir, path);
}

/* Check if filename contains any bad keyword */
static int is_dangerous(const char *name) {
    char lower[PATH_MAX];
    for (size_t i = 0; i < strlen(name) && i+1 < sizeof(lower); i++)
        lower[i] = tolower((unsigned char)name[i]);
    lower[strlen(name)] = '\0';
    for (const char **k = bad_keys; *k; ++k) {
        if (strstr(lower, *k))
            return 1;
    }
    return 0;
}

/* Reverse a string in place */
static void str_reverse(char *s) {
    size_t i = 0, j = strlen(s);
    if (j == 0) return;
    for (j--; i < j; ++i, --j) {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
    }
}

/* FUSE: getattr */
static int ak_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
    char fp[PATH_MAX];
    fullpath(fp, path);
    int res = lstat(fp, st);
    return res == -1 ? -errno : 0;
}

/* FUSE: readdir */
static int ak_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags)
{
    (void) offset; (void) fi; (void) flags;
    DIR *dp;
    struct dirent *de;
    char fp[PATH_MAX];
    fullpath(fp, path);

    dp = opendir(fp);
    if (!dp) return -errno;

    while ((de = readdir(dp)) != NULL) {
        char namebuf[PATH_MAX];
        strncpy(namebuf, de->d_name, sizeof(namebuf)-1);
        namebuf[sizeof(namebuf)-1] = '\0';

        if (is_dangerous(namebuf)) {
            write_log("WARN file detected: %s", namebuf);
            str_reverse(namebuf);
        }
        filler(buf, namebuf, NULL, 0, 0);
    }
    closedir(dp);
    return 0;
}

/* ROT13 transform on buffer */
static void apply_rot13(char *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        char c = data[i];
        if ('a' <= c && c <= 'z')
            data[i] = 'a' + (c - 'a' + 13) % 26;
        else if ('A' <= c && c <= 'Z')
            data[i] = 'A' + (c - 'A' + 13) % 26;
    }
}

/* FUSE: open */
static int ak_open(const char *path, struct fuse_file_info *fi) {
    char fp[PATH_MAX];
    fullpath(fp, path);
    int fd = open(fp, fi->flags);
    if (fd < 0) return -errno;
    close(fd);
    return 0;
}

/* FUSE: read */
static int ak_read(const char *path, char *buf, size_t size,
                   off_t offset, struct fuse_file_info *fi)
{
    (void) fi;
    char fp[PATH_MAX];
    fullpath(fp, path);

    int fd = open(fp, O_RDONLY);
    if (fd < 0) return -errno;

    /* read raw data */
    ssize_t res = pread(fd, buf, size, offset);
    if (res < 0) {
        close(fd);
        return -errno;
    }
    close(fd);

    /* apply ROT13 if .txt & not dangerous */
    if (!is_dangerous(path) && strstr(path, ".txt")) {
        apply_rot13(buf, (size_t)res);
    }
    write_log("INFO read %s offset=%jd size=%zu", path, (intmax_t)offset, size);
    return res;
}

/* FUSE operations struct */
static struct fuse_operations ak_ops = {
    .getattr = ak_getattr,
    .readdir = ak_readdir,
    .open    = ak_open,
    .read    = ak_read,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &ak_ops, NULL);
}
