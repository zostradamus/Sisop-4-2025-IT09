# LAPORAN RESMI MODUL 4 SISOP

## ANGGOTA KELOMPOK
| Nama                           | NRP        |
| -------------------------------| ---------- |
| Shinta Alya Ramadani           | 5027241016 |
| Prabaswara Febrian Winandika   | 5027241069 |
| Muhammad Farrel Rafli Al Fasya | 5027241075 |

## Soal 1
### Deskripsi
Shorekeeper menemukan anomali berupa teks acak yang disimpan dalam format hexadecimal. Ia mencurigai bahwa teks tersebut mengandung data gambar yang disandikan. Tugas praktikan adalah membantu Shorekeeper melakukan:
1. Unzip file yang berisi teks hexadecimal, lalu hapus file zip-nya.
2. Buat sistem file menggunakan FUSE yang akan otomatis mengkonversi isi file teks hexadecimal menjadi gambar PNG saat file dibuka.
3. Simpan hasil gambar ke folder anomali/image/ dengan format:
```
[nama_file]_image_[YYYY-mm-dd]_[HH:MM:SS].png
```
4. Catat setiap konversi ke dalam file anomali/conversion.log dengan format:
```
[YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text <input_file> to <output_file>.
```
```
.
├── anomali/
│   ├── image/                # Menyimpan file hasil konversi
│   └── conversion.log        # Log file konversi                
├── hexed.c                   # Program utama
└── anomali.zip               # File input
```
### Code hexed.c
#### hex_to_bin
1. Fungsi ini mengubah string hexadecimal menjadi file biner.
2. hex_str adalah input string heksadesimal (misalnya "ffd8ffe0...").
3. output_file adalah nama file hasil (misalnya 1_image_2025-05-11_18:35:26.png).
Langkah:
1. Buka file output dalam mode wb (write binary).
2. Loop per dua karakter (karena 1 byte = 2 hex digit).
3. Ubah tiap 2 digit hex menjadi 1 byte biner menggunakan strtol.
4. Tulis byte ke file output.
Code:
```
int hex_to_bin(const char *hex_str, const char *output_file) {
    FILE *fp = fopen(output_file, "wb");
    if (!fp) return 0;

    size_t len = strlen(hex_str);
    char byte_str[3] = {0};

    for (size_t i = 0; i < len; i += 2) {
        if (i + 1 >= len) break;
        byte_str[0] = hex_str[i];
        byte_str[1] = hex_str[i + 1];
        unsigned char byte = (unsigned char)strtol(byte_str, NULL, 16);
        fwrite(&byte, sizeof(unsigned char), 1, fp);
    }

    fclose(fp);
    return 1;
}
```
#### int main
##### Fungsi di main
1. Mengecek apakah input file diberikan di command line.
2. Jika tidak, tampilkan petunjuk penggunaan dan keluar.
```
if (argc != 2) {
        fprintf(stderr, "Usage: %s <hex_file.txt>\n", argv[0]);
        return 1;
    }
```
##### Ambil nama file dari path input
1. Mengambil nama file saja tanpa path (1.txt dari folder/1.txt).
2. Disimpan ke filename_only.
```
char *input_file = argv[1];
    char filename_only[64];
    char *last_slash = strrchr(input_file, '/');
    if (last_slash)
        strcpy(filename_only, last_slash + 1);
    else
        strcpy(filename_only, input_file);
```
3. Menghapus ekstensi .txt → hasilnya jadi 1.
```
char base_name[64];
    strncpy(base_name, filename_only, strlen(filename_only) - 4);
    base_name[strlen(filename_only) - 4] = '\0';
```
##### Ambil waktu saat ini
1. Mengambil waktu lokal saat konversi dilakukan.
```
time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
```
2. Format waktu menjadi string seperti: 2025-05-11_18:35:26.
```
char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H:%M:%S", tm_now);
```
##### Membuat folder hasil
Membuat folder anomali dan image jika belum ada.
```
char output_dir[] = "anomali/image";
    mkdir("anomali", 0755); // jika belum ada
    mkdir(output_dir, 0755);
```
##### Menyiapkan nama file hasil
Menyusun nama file: anomali/image/1_image_2025-05-11_18:35:26.png.
```
char image_file[MAX_PATH];
    snprintf(image_file, sizeof(image_file), "%s/%s_image_%s.png", output_dir, base_name, time_str);
```
##### Membaca isi file hex
1. Menentukan panjang file, lalu kembali ke awal.
```
FILE *fp = fopen(input_file, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);
```
2. Alokasi memori untuk string hex.
3. hex_data[len] = '\0'; menandai akhir string.
```
char *hex_data = malloc(len + 1);
    if (!hex_data) {
        fclose(fp);
        return 1;
    }

    fread(hex_data, 1, len, fp);
    hex_data[len] = '\0';
    fclose(fp);
```
##### Konversi hex ke gambar
Panggil fungsi hex_to_bin, lalu cek keberhasilannya.
```
if (!hex_to_bin(hex_data, image_file)) {
        fprintf(stderr, "Failed to convert %s\n", input_file);
        free(hex_data);
        return 1;
    }
```
#####  Logging hasil konversi
```
char log_path[] = "anomali/conversion.log";
    FILE *log_fp = fopen(log_path, "a");
```
Buka file anomali/conversion.log dalam mode append.
```
if (log_fp) {
        char log_date[16], log_time[16];
        strftime(log_date, sizeof(log_date), "%Y-%m-%d", tm_now);
        strftime(log_time, sizeof(log_time), "%H:%M:%S", tm_now);
        fprintf(log_fp, "[%s][%s]: Successfully converted hexadecimal text %s to %s.\n",
                log_date, log_time, filename_only, strrchr(image_file, '/') + 1);
        fclose(log_fp);
    }
```
Format log sesuai dengan ketentuan:
```
[YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text <namafile> to <outputfile>.
```
#### Code penuh
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 256

int hex_to_bin(const char *hex_str, const char *output_file) {
    FILE *fp = fopen(output_file, "wb");
    if (!fp) return 0;

    size_t len = strlen(hex_str);
    char byte_str[3] = {0};

    for (size_t i = 0; i < len; i += 2) {
        if (i + 1 >= len) break;
        byte_str[0] = hex_str[i];
        byte_str[1] = hex_str[i + 1];
        unsigned char byte = (unsigned char)strtol(byte_str, NULL, 16);
        fwrite(&byte, sizeof(unsigned char), 1, fp);
    }

    fclose(fp);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hex_file.txt>\n", argv[0]);
        return 1;
    }

    char *input_file = argv[1];
    char filename_only[64];
    char *last_slash = strrchr(input_file, '/');
    if (last_slash)
        strcpy(filename_only, last_slash + 1);
    else
        strcpy(filename_only, input_file);

    char base_name[64];
    strncpy(base_name, filename_only, strlen(filename_only) - 4);
    base_name[strlen(filename_only) - 4] = '\0';

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H:%M:%S", tm_now);

    char output_dir[] = "anomali/image";
    mkdir("anomali", 0755); // jika belum ada
    mkdir(output_dir, 0755);

    char image_file[MAX_PATH];
    snprintf(image_file, sizeof(image_file), "%s/%s_image_%s.png", output_dir, base_name, time_str);

    FILE *fp = fopen(input_file, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *hex_data = malloc(len + 1);
    if (!hex_data) {
        fclose(fp);
        return 1;
    }

    fread(hex_data, 1, len, fp);
    hex_data[len] = '\0';
    fclose(fp);

    if (!hex_to_bin(hex_data, image_file)) {
        fprintf(stderr, "Failed to convert %s\n", input_file);
        free(hex_data);
        return 1;
    }

    char log_path[] = "anomali/conversion.log";
    FILE *log_fp = fopen(log_path, "a");
    if (log_fp) {
        char log_date[16], log_time[16];
        strftime(log_date, sizeof(log_date), "%Y-%m-%d", tm_now);
        strftime(log_time, sizeof(log_time), "%H:%M:%S", tm_now);
        fprintf(log_fp, "[%s][%s]: Successfully converted hexadecimal text %s to %s.\n",
                log_date, log_time, filename_only, strrchr(image_file, '/') + 1);
        fclose(log_fp);
    }

    free(hex_data);
    return 0;
}
```
## Soal 2
### Deskripsi
Seorang ilmuwan muda menemukan sebuah drive tua berisi pecahan data dari robot legendaris Baymax. File asli Baymax telah terfragmentasi menjadi 14 bagian (masing-masing 1KB) dengan nama Baymax.jpeg.000 hingga Baymax.jpeg.013 yang berada dalam folder relics. Ilmuwan tersebut ingin melihat Baymax dalam bentuk file utuh tanpa merusak fragmen aslinya.

Tugas praktikan adalah :
1. Membuat filesystem virtual menggunakan FUSE.
2. File Baymax.jpeg ditampilkan secara utuh saat FUSE di-mount.
3. File baru yang dibuat di direktori mount akan otomatis terpecah menjadi potongan 1KB dan disimpan di relics/.
4. Jika file dihapus dari mount point, semua fragmennya di relics/ juga ikut terhapus.
5. Seluruh aktivitas dicatat dalam activity.log.

Fitur yang Diimplementasikan

🔍 getattr
   - Menangani metadata file.
   - Jika file adalah Baymax.jpeg, maka ukuran dihitung sebagai 14 * 1024 byte.
   - Jika file berasal dari buffer sementara atau memiliki fragmen .000, maka dikenali juga sebagai file yang valid.

📁 readdir
   - Menampilkan isi direktori mount, termasuk Baymax.jpeg.
   - Mendeteksi file baru berdasarkan pecahan .000 di direktori relics.

📖 open dan read
   - Saat Baymax.jpeg dibuka atau dibaca, akan menyatukan 14 file fragment .000 sampai .013.
   - Log aktivitas READ dicatat di activity.log.

📝 create, write, dan release
   - File baru yang dibuat di mount point akan disimpan di buffer sementara.
   - Saat file ditutup (release), akan dipotong-potong 1KB dan disimpan ke dalam relics/ dengan format [namafile].000, [namafile].001, dst.
   - Log aktivitas WRITE mencatat nama file dan seluruh fragmennya.

❌ unlink
   - Menghapus file dari mount akan menghapus semua file [namafile].xxx di relics/.
   - Log DELETE dicatat saat file dan fragmennya dihapus.

Struktur Folder :
```
├── baymax.c           # Implementasi FUSE
├── relics/            # Folder penyimpanan fragmen
│   ├── Baymax.jpeg.000
│   ├── ...
│   └── Baymax.jpeg.013
├── mount_dir/         # Mount point FUSE
└── activity.log       # Log aktivitas
```

Format Log Aktivitas
```
[2025-05-11 10:24:01] READ: Baymax.jpeg
[2025-05-11 10:25:14] WRITE: hero.txt -> hero.txt.000, hero.txt.001
[2025-05-11 10:26:03] DELETE: Baymax.jpeg.000 - Baymax.jpeg.013
[2025-05-11 10:27:45] COPY: Baymax.jpeg -> /tmp/Baymax.jpeg
```

Kode Program baymax.c
```
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
    if (strcmp(path, "/") != 0) return -ENOENT;

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
    if (offset >= total_size) return 0;
    if (offset + size > total_size) size = total_size - offset;

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

    free(fb->data); free(fb->name);
    fb->data = NULL; fb->name = NULL; fb->size = 0;
    return 0;
}

static int baymax_unlink(const char *path) {
    const char *filename = path + 1;
    char frag_path[256];
    int part = 0, found_any = 0;

    while (1) {
        snprintf(frag_path, sizeof(frag_path), "%s/%s.%03d", RELICS_DIR, filename, part);
        if (access(frag_path, F_OK) != 0) break;
        if (unlink(frag_path) == 0) found_any = 1;
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
```

REVISI
(full code baymax.c)
```
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#define FRAGMENTS 14
#define CHUNK_SIZE 1024

static const char* relics_dir = "/home/shintaar/modul4/soal_2/relics";
static const char* log_path = "/home/shintaar/modul4/soal_2/activity.log";

void log_activity(const char* format, ...) {
    FILE* log_file = fopen(log_path, "a");
    if (!log_file) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", t);

    fprintf(log_file, "%s ", timestamp);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fclose(log_file);
}

static int fs_getattr(const char* path, struct stat* stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    const char* filename = path + 1;
    char frag_path[256];
    sprintf(frag_path, "%s/%s.000", relics_dir, filename);

    if (access(frag_path, F_OK) == 0) {
        int total_frag = 0;
        for (int i = 0; i < 100; i++) {
            sprintf(frag_path, "%s/%s.%03d", relics_dir, filename, i);
            if (access(frag_path, F_OK) != 0)
                break;
            total_frag++;
        }

        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
        stbuf->st_size = total_frag * CHUNK_SIZE;
        return 0;
    }

    return -ENOENT;
}

static int fs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    if (strcmp(path, "/") != 0)
    return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, "Baymax.jpeg", NULL, 0);

    return 0;
}

static int fs_open(const char* path, struct fuse_file_info* fi) {
    const char* filename = path + 1;
    char frag_path[256];
    sprintf(frag_path, "%s/%s.000", relics_dir, filename);

    int fd = open(frag_path, fi->flags);
    if (fd == -1)
        return -errno;

    fi->fh = fd;
    return 0;
}

static int fs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0)
        return -ENOENT;

    size_t total_size = FRAGMENTS * CHUNK_SIZE;
    if (offset >= total_size)
        return 0;

    if (offset + size > total_size)
        size = total_size - offset;

    size_t start_frag = offset / CHUNK_SIZE;
    size_t end_frag = (offset + size - 1) / CHUNK_SIZE;

    size_t copied = 0;
    for (size_t i = start_frag; i <= end_frag; ++i) {
        char frag_path[256];
        sprintf(frag_path, "%s/Baymax.jpeg.%03zu", relics_dir, i);

        FILE* fp = fopen(frag_path, "rb");
        if (!fp) {
	    perror(frag_path);
	    return -errno;
	}

        char chunk[CHUNK_SIZE];
        size_t len = fread(chunk, 1, CHUNK_SIZE, fp);
        fclose(fp);

        size_t frag_start = (i == start_frag) ? (offset % CHUNK_SIZE) : 0;
        size_t frag_end = (i == end_frag) ? ((offset + size - 1) % CHUNK_SIZE + 1) : len;

        memcpy(buf + copied, chunk + frag_start, frag_end - frag_start);
        copied += frag_end - frag_start;
    }

    log_activity("READ: Baymax.jpeg");

	pid_t pid = fuse_get_context()->pid;
	char proc_path[256];
	sprintf(proc_path, "/proc/%d/cmdline", pid);
	FILE* f = fopen(proc_path, "r");
		if (f) {
	    char cmdline[256] = {0};
	    fread(cmdline, 1, sizeof(cmdline), f);
	    fclose(f);

    if (strstr(cmdline, "cp")) {
        log_activity("COPY: Baymax.jpeg");
      }
    }

    return copied;
}

static int fs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    const char* filename = path + 1;

    char frag_path[256];
    sprintf(frag_path, "%s/%s.000", relics_dir, filename);

    int fd = open(frag_path, O_WRONLY | O_CREAT, 0666);
    if (fd == -1) return -errno;

    fi->fh = fd;

    log_activity("CREATE: %s.000", filename);
    return 0;
}

static int fs_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    const char* filename = path + 1;

    if (fi->fh > 0) {
        int res = pwrite(fi->fh, buf, size, offset);
        if (res == -1) return -errno;

        log_activity("WRITE: %s -> %s.000", filename, filename);
        return res;
    }

    size_t parts = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    for (size_t i = 0; i < parts; ++i) {
        char frag_path[256];
        sprintf(frag_path, "%s/%s.%03zu", relics_dir, filename, i);

        FILE* fp = fopen(frag_path, "wb");
        if (!fp) return -errno;

        size_t chunk_size = (i < parts - 1) ? CHUNK_SIZE : (size - i * CHUNK_SIZE);
        fwrite(buf + i * CHUNK_SIZE, 1, chunk_size, fp);
        fclose(fp);
    }

    char log_msg[512] = "";
    sprintf(log_msg, "WRITE: %s -> ", filename);
    for (size_t i = 0; i < parts; ++i) {
        char frag[32];
        sprintf(frag, "%s.%03zu", filename, i);
        strcat(log_msg, frag);
        if (i < parts - 1) strcat(log_msg, ", ");
    }
    log_activity("%s", log_msg);

    return size;
}

static int fs_unlink(const char* path) {
    const char* filename = path + 1;
    int last_idx = -1;

    for (int i = 0; i < 100; ++i) {
        char frag_path[256];
        sprintf(frag_path, "%s/%s.%03d", relics_dir, filename, i);
        if (access(frag_path, F_OK) != 0)
            break;
        remove(frag_path);
        last_idx = i;
    }

    if (last_idx >= 0) {
        log_activity("DELETE: %s.000 - %s.%03d", filename, filename, last_idx);
    }

    return 0;
}

static struct fuse_operations fs_oper = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open = fs_open,
    .read = fs_read,
    .create = fs_create,
    .write = fs_write,
    .unlink = fs_unlink,
};

int main(int argc, char* argv[]) {
    umask(0);
    return fuse_main(argc, argv, &fs_oper, NULL);
}
```



