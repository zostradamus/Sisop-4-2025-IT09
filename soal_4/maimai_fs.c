#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

static const char *dirpath_chiho = "/home/kali/Documents/soal_4_modul4/soal_4/chiho";

void encode(const char* src, char* dst, int len) {
    for (int i = 0; i < len; i++) {
        dst[i] = src[i] + (i % 256);
    }
}

void decode(const char* src, char* dst, int len) {
    for (int i = 0; i < len; i++) {
        dst[i] = src[i] - (i % 256);
    }
}

void rot13(const char *src, char *dst, int len) {
    for (int i = 0; i < len; i++) {
        if ((src[i] >= 'A' && src[i] <= 'Z'))
            dst[i] = (src[i] - 'A' + 13) % 26 + 'A';
        else if ((src[i] >= 'a' && src[i] <= 'z'))
            dst[i] = (src[i] - 'a' + 13) % 26 + 'a';
        else
            dst[i] = src[i];
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf) {
    char fpath[1024];
    if (strncmp(path, "/metro/", 7) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro%s.ccc", dirpath_chiho, path + 6);
    } else if (strncmp(path, "/starter/", 9) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/starter%s.mai", dirpath_chiho, path + 8);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon%s.rot", dirpath_chiho, path + 7);
    } else {
        snprintf(fpath, sizeof(fpath), "%s%s", dirpath_chiho, path);
    }
    int res = lstat(fpath, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    char fpath[1024];

    (void) offset;
    (void) fi;

    if (strcmp(path, "/metro") == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro", dirpath_chiho);
    } else if (strcmp(path, "/starter") == 0) {
        snprintf(fpath, sizeof(fpath), "%s/starter", dirpath_chiho);
    } else if (strcmp(path, "/dragon") == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon", dirpath_chiho);
    } else if (strcmp(path, "/") == 0) {
        snprintf(fpath, sizeof(fpath), "%s", dirpath_chiho);
    } else {
        snprintf(fpath, sizeof(fpath), "%s%s", dirpath_chiho, path);
    }

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((de = readdir(dp)) != NULL) {
        size_t len = strlen(de->d_name);

        if (strcmp(path, "/metro") == 0) {
            if (len > 4 && strcmp(de->d_name + len - 4, ".ccc") == 0) {
                char name[256];
                strncpy(name, de->d_name, len - 4);
                name[len - 4] = '\0';
                filler(buf, name, NULL, 0);
            }
        } else if (strcmp(path, "/starter") == 0) {
            if (len > 4 && strcmp(de->d_name + len - 4, ".mai") == 0) {
                char name[256];
                strncpy(name, de->d_name, len - 4);
                name[len - 4] = '\0';
                filler(buf, name, NULL, 0);
            }
        } else if (strcmp(path, "/dragon") == 0) {
            if (len > 4 && strcmp(de->d_name + len - 4, ".rot") == 0) {
                char name[256];
                strncpy(name, de->d_name, len - 4);
                name[len - 4] = '\0';
                filler(buf, name, NULL, 0);
            }
        } else {
            filler(buf, de->d_name, NULL, 0);
        }
    }

    closedir(dp);
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
    char fpath[1024];
    if (strncmp(path, "/metro/", 7) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro%s.ccc", dirpath_chiho, path + 6);
    } else if (strncmp(path, "/starter/", 9) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/starter%s.mai", dirpath_chiho, path + 8);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon%s.rot", dirpath_chiho, path + 7);
    } else {
        snprintf(fpath, sizeof(fpath), "%s%s", dirpath_chiho, path);
    }
    int fd = open(fpath, fi->flags);
    if (fd == -1) return -errno;
    fi->fh = fd;
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    char fpath[1024];
    if (strncmp(path, "/metro/", 7) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro%s.ccc", dirpath_chiho, path + 6);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon%s.rot", dirpath_chiho, path + 7);
    } else {
        return pread(fi->fh, buf, size, offset);
    }

    int fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;

    off_t filesize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char *enc_buf = malloc(filesize);
    if (!enc_buf) {
        close(fd);
        return -ENOMEM;
    }

    ssize_t read_bytes = read(fd, enc_buf, filesize);
    close(fd);
    if (read_bytes != filesize) {
        free(enc_buf);
        return -EIO;
    }

    char *dec_buf = malloc(filesize);
    if (!dec_buf) {
        free(enc_buf);
        return -ENOMEM;
    }

    if (strncmp(path, "/metro/", 7) == 0)
        decode(enc_buf, dec_buf, filesize);
    else if (strncmp(path, "/dragon/", 8) == 0)
        rot13(enc_buf, dec_buf, filesize);

    free(enc_buf);

    if (offset < filesize) {
        if (offset + size > filesize)
            size = filesize - offset;
        memcpy(buf, dec_buf + offset, size);
    } else {
        size = 0;
    }

    free(dec_buf);
    return size;
}

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi) {
    char fpath[1024];
    int is_metro = 0, is_dragon = 0;

    if (strncmp(path, "/metro/", 7) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro%s.ccc", dirpath_chiho, path + 6);
        is_metro = 1;
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon%s.rot", dirpath_chiho, path + 7);
        is_dragon = 1;
    } else {
        return pwrite(fi->fh, buf, size, offset);
    }

    int fd = open(fpath, O_RDWR);
    char *dec_buf = NULL;
    ssize_t filesize = 0;

    if (fd == -1) {
        filesize = offset + size;
        dec_buf = calloc(filesize, 1);
        if (!dec_buf) return -ENOMEM;
    } else {
        filesize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        char *enc_buf = malloc(filesize);
        if (!enc_buf) {
            close(fd);
            return -ENOMEM;
        }
        read(fd, enc_buf, filesize);
        close(fd);

        dec_buf = malloc(filesize > offset + size ? filesize : offset + size);
        if (!dec_buf) {
            free(enc_buf);
            return -ENOMEM;
        }

        if (is_metro)
            decode(enc_buf, dec_buf, filesize);
        else if (is_dragon)
            rot13(enc_buf, dec_buf, filesize);
        free(enc_buf);

        if (filesize < offset + size) {
            dec_buf = realloc(dec_buf, offset + size);
            if (!dec_buf) return -ENOMEM;
            memset(dec_buf + filesize, 0, offset + size - filesize);
            filesize = offset + size;
        }
    }

    memcpy(dec_buf + offset, buf, size);

    int fdw = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdw == -1) {
        free(dec_buf);
        return -errno;
    }

    char *enc_buf = malloc(filesize);
    if (!enc_buf) {
        free(dec_buf);
        close(fdw);
        return -ENOMEM;
    }

    if (is_metro)
        encode(dec_buf, enc_buf, filesize);
    else if (is_dragon)
        rot13(dec_buf, enc_buf, filesize);

    write(fdw, enc_buf, filesize);
    free(dec_buf);
    free(enc_buf);
    close(fdw);

    return size;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fpath[1024];
    if (strncmp(path, "/metro/", 7) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro%s.ccc", dirpath_chiho, path + 6);
    } else if (strncmp(path, "/starter/", 9) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/starter%s.mai", dirpath_chiho, path + 8);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon%s.rot", dirpath_chiho, path + 7);
    } else {
        snprintf(fpath, sizeof(fpath), "%s%s", dirpath_chiho, path);
    }
    int fd = open(fpath, O_CREAT | O_RDWR, mode);
    if (fd == -1) return -errno;
    fi->fh = fd;
    return 0;
}

static int xmp_unlink(const char *path) {
    char fpath[1024];
    int res;

    if (strncmp(path, "/metro/", 7) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/metro%s.ccc", dirpath_chiho, path + 6);
    } else if (strncmp(path, "/starter/", 9) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/starter%s.mai", dirpath_chiho, path + 8);
    } else if (strncmp(path, "/dragon/", 8) == 0) {
        snprintf(fpath, sizeof(fpath), "%s/dragon%s.rot", dirpath_chiho, path + 7);
    } else {
        snprintf(fpath, sizeof(fpath), "%s%s", dirpath_chiho, path);
    }

    res = unlink(fpath);
    if (res == -1) return -errno;
    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open    = xmp_open,
    .read    = xmp_read,
    .write   = xmp_write,
    .create  = xmp_create,
    .unlink  = xmp_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
