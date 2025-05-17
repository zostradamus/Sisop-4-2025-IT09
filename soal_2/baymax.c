#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#define FRAGMENT_COUNT 14
#define FRAGMENT_SIZE 1024
#define RELICS_DIR "./relics"
#define LOG_FILE "./activity.log"

struct FileBuffer {
    char *name;
    char *data;
    size_t size;
};

static struct FileBuffer buffers[100];
static const char *baymax_filename = "Baymax.jpeg";

void log_activity(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            t->tm_year+1900, t->tm_mon+1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, message);
    fclose(log);
}

static int baymax_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    const char *filename = path + 1;

    if (strcmp(filename, baymax_filename) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = FRAGMENT_COUNT * FRAGMENT_SIZE;
        return 0;
    }

    for (int i = 0; i < 100; i++) {
        if (buffers[i].name && strcmp(filename, buffers[i].name) == 0) {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = buffers[i].size;
            return 0;
        }
    }

    char test_path[256];
    snprintf(test_path, sizeof(test_path), "%s/%s.000", RELICS_DIR, filename);
    if (access(test_path, F_OK) == 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = 0;
        return 0;
    }

    return -ENOENT;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, baymax_filename, NULL, 0);

    for (int i = 0; i < 100; i++) {
        if (buffers[i].name)
            filler(buf, buffers[i].name, NULL, 0);
    }

    DIR *dir = opendir(RELICS_DIR);
    if (!dir) return 0;

    struct dirent *de;
    char listed[100][256]; int count = 0;

    while ((de = readdir(dir)) != NULL) {
        if (strstr(de->d_name, ".000")) {
            char base[256];
            strncpy(base, de->d_name, strlen(de->d_name) - 4);
            base[strlen(de->d_name) - 4] = '\0';

            int found = 0;
            for (int i = 0; i < count; i++) {
                if (strcmp(listed[i], base) == 0) {
                    found = 1; break;
                }
            }
            if (!found) {
                strcpy(listed[count++], base);
                filler(buf, base, NULL, 0);
            }
        }
    }
    closedir(dir);
    return 0;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path + 1, baymax_filename) != 0)
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    (void) fi;

    if (strcmp(path + 1, baymax_filename) != 0)
        return -ENOENT;

    size_t total_size = FRAGMENT_COUNT * FRAGMENT_SIZE;
    if (offset >= total_size)
        return 0;

    if (offset + size > total_size)
        size = total_size - offset;

    size_t bytes_read = 0;
    int frag_index = offset / FRAGMENT_SIZE;
    size_t frag_offset = offset % FRAGMENT_SIZE;

    while (bytes_read < size && frag_index < FRAGMENT_COUNT) {
        char frag_path[256];
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_DIR, baymax_filename, frag_index);
        FILE *frag = fopen(frag_path, "rb");
        if (!frag) break;

        fseek(frag, frag_offset, SEEK_SET);
        size_t to_read = FRAGMENT_SIZE - frag_offset;
        if (to_read > size - bytes_read)
            to_read = size - bytes_read;

        fread(buf + bytes_read, 1, to_read, frag);
        fclose(frag);

        bytes_read += to_read;
        frag_index++;
        frag_offset = 0;
    }

    log_activity("READ: Baymax.jpeg");
    return bytes_read;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) mode;
    const char *filename = path + 1;

    for (int i = 0; i < 100; i++) {
        if (buffers[i].data == NULL) {
            buffers[i].data = malloc(1);
            buffers[i].size = 0;
            buffers[i].name = strdup(filename);
            fi->fh = i;
            return 0;
        }
    }
    return -ENOMEM;
}

static int baymax_write(const char *path, const char *buf, size_t size, off_t offset,
                        struct fuse_file_info *fi) {
    int index = fi->fh;
    if (index < 0 || index >= 100 || buffers[index].data == NULL)
        return -EIO;

    struct FileBuffer *fb = &buffers[index];
    size_t new_size = offset + size;

    fb->data = realloc(fb->data, new_size);
    memcpy(fb->data + offset, buf, size);
    if (new_size > fb->size)
        fb->size = new_size;

    return size;
}

static int baymax_release(const char *path, struct fuse_file_info *fi) {
    int index = fi->fh;
    if (index < 0 || index >= 100 || buffers[index].data == NULL)
        return 0;

    struct FileBuffer *fb = &buffers[index];
    const char *filename = fb->name;

    mkdir(RELICS_DIR, 0755);

    size_t written = 0;
    int part = 0;

    while (written < fb->size) {
        char frag_path[256];
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_DIR, filename, part++);

        FILE *frag = fopen(frag_path, "wb");
        if (!frag) break;

        size_t to_write = FRAGMENT_SIZE;
        if (fb->size - written < FRAGMENT_SIZE)
            to_write = fb->size - written;

        fwrite(fb->data + written, 1, to_write, frag);
        fclose(frag);
        written += to_write;
    }

    char log_msg[512];
    snprintf(log_msg, sizeof(log_msg), "WRITE: %s ->", filename);
    for (int i = 0; i < part; i++) {
        char part_name[32];
        snprintf(part_name, sizeof(part_name), " %s.%03d", filename, i);
        strcat(log_msg, part_name);
    }
    log_activity(log_msg);

    free(fb->data);
    free(fb->name);
    fb->data = NULL;
    fb->name = NULL;
    fb->size = 0;

    return 0;
}

static int baymax_unlink(const char *path) {
    const char *filename = path + 1;
    char frag_path[256];
    int part = 0;
    int found_any = 0;

    while (1) {
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_DIR, filename, part);
        if (access(frag_path, F_OK) != 0) {

            break;
        }
        if (unlink(frag_path) == 0) {
            found_any = 1;
        }
        part++;
    }

    if (found_any) {
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "DELETE: %s and fragments removed", filename);
        log_activity(log_msg);
        return 0;
    } else {
        return -ENOENT;
    }
}

static struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .open    = baymax_open,
    .read    = baymax_read,
    .create  = baymax_create,
    .write   = baymax_write,
    .release = baymax_release,
    .unlink  = baymax_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &baymax_oper, NULL);
}
