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
## Soal 3
# AntiNK (Anti Napis Kimcun)

Pujo, komting angkatan 24 yang baik hati, membangun sistem **AntiNK** (Anti Napis Kimcun) untuk mendeteksi dan menanggulangi "kenakalan" mahasiswa Nafis dan Kimcun. Sistem ini menggunakan **FUSE** (Filesystem in Userspace) di dalam **Docker** container, serta **docker-compose** untuk mengelola:

* `antink-server` (FUSE filesystem)
* `antink-logger` (monitoring log real-time)

Transformasi file terjadi *on-the-fly* dalam container tanpa mengubah file asli di host.

---

## 🗂️ Repository Structure

```
soal_3/
├── Dockerfile             # Dockerfile untuk antink-server
├── docker-compose.yml     # Orkestrasi antink-server & antink-logger
├── antink.c               # Implementasi FUSE filesystem
├── data/                  # Bind mount untuk file asli (host)
└── logs/
    └── it24.log           # Bind mount untuk log (host)
```

---

## ⚙️ Installation & Setup

1. **Clone repository**

   ```bash
   git clone <repo-url>
   cd soal_3
   ```
2. **Create host directories**

   ```bash
   mkdir -p data logs
   ```
3. **Build and start all services**

   ```bash
   docker-compose up --build -d
   ```
4. **Verify containers**

   ```bash
   docker ps
   # Should see antink-server and antink-logger running
   ```

---

## 📦 Code Explanation

### 1. `antink.c`

```c
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

static const char *root_dir = "/it24_host";          // Direktori asal di host
static const char *log_path = "/antink_logs/it24.log"; // Path file log di container
static const char *keys[] = { "nafis", "kimcun", NULL }; // Kata kunci deteksi file berbahaya
```

* **root\_dir** dan **log\_path**: Lokasi mount bind host
* **keys\[]**: Mendefinisikan kata kunci

```c
// Fungsi untuk menuliskan log dengan timestamp
static void write_log(const char *fmt, ...) { ... }
```

* Membuka `it24.log`, menambahkan timestamp, dan mencatat pesan dengan format variadic.

```c
// Membangun path absolut di host
static void fullpath(char buf[PATH_MAX], const char *path) {
    snprintf(buf, PATH_MAX, "%s%s", root_dir, path);
}
```

```c
// Deteksi nama file berbahaya
static int is_bad(const char *name) { ... }
// Deteksi nama yang sudah dibalik
static int is_reversed_bad(const char *name) { ... }
```

* `is_bad()`: Memeriksa substring `nafis` atau `kimcun`
* `is_reversed_bad()`: Membalik string dan memeriksa kembali

```c
// Reverse string in-place
static void strrev(char *s) { ... }
```

#### FUSE callbacks

```c
static int ak_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) { ... }
```

* **Intercept** `stat()`
* Jika `is_bad` atau `is_reversed_bad`, log **ALERT** dan **REVERSE**
* Panggil `lstat` pada file asli

```c
static int ak_readdir(const char *path, void *buf, fuse_fill_dir_t filler, ...) { ... }
```

* **Intercept** `ls` (directory listing)
* Untuk setiap entri, jika `is_bad`, log **ALERT** dan **REVERSE**, tampilkan nama dibalik

```c
static int ak_open(const char *path, struct fuse_file_info *fi) { ... }
```

* **Intercept** `open()`
* Jika file berbahaya, log **REVERSE** sebelum membuka file

```c
static int ak_read(const char *path, char *buf, size_t size, off_t offset, ...) { ... }
```

* **Intercept** `read()`
* Jika normal `.txt`, terapkan **ROT13** dan log **ENCRYPT**
* Jika berbahaya, log **REVERSE**

```c
int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &ops, NULL);
}
```

* Inisialisasi FUSE dengan operasi yang telah didefinisikan

---

### 2. `Dockerfile`

```dockerfile
FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    gcc make pkg-config fuse3 libfuse3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
RUN mkdir -p /it24_host /antink_mount /antink_logs
COPY antink.c .
RUN gcc -std=gnu11 -Wall -Wextra antink.c -o antink-fuse $(pkg-config fuse3 --cflags --libs)

ENTRYPOINT ["/app/antink-fuse"]
CMD ["/antink_mount", "-f"]
```

* **Install**: compiler, pkg-config, FUSE3 dev
* **Direktori kerja**: membuat mount points dalam container
* **Compile**: menghasilkan binary `antink-fuse`
* **Entrypoint** & **CMD**: menjalankan FUSE di foreground pada `/antink_mount`

---

### 3. `docker-compose.yml`

```yaml
version: '3.8'
services:
  antink-server:
    build: .
    container_name: antink-server
    privileged: true        # akses FUSE
    devices:
      - /dev/fuse
    security_opt:
      - apparmor:unconfined
    volumes:
      - ./data:/it24_host:ro
      - antink_mount:/antink_mount
      - ./logs:/antink_logs

  antink-logger:
    image: alpine:3.14
    container_name: antink-logger
    depends_on:
      - antink-server
    entrypoint: sh -c "tail -F /antink_logs/it24.log"
    volumes:
      - ./logs:/antink_logs:ro

volumes:
  antink_mount:
```

* **antink-server**: menjalankan FUSE, mount host `data/` dan `logs/`
* **antink-logger**: menampilkan log secara real-time

---

## 🧪 Testing

1. **File normal** (ROT13 + ENCRYPT)

   ```bash
   echo "hello world" > data/foo.txt
   docker exec antink-server cat /antink_mount/foo.txt
   # → uryyb jbeyq
   ```
2. **File berbahaya** (reverse + plain)

   ```bash
   echo "secret kimcun" > data/kimcun_plan.txt
   docker exec antink-server ls /antink_mount
   # → txt.nalp_nucmik
   docker exec antink-server cat /antink_mount/txt.nalp_nucmik
   # → secret kimcun
   ```
3. **Log**:

   ```bash
   docker logs -f antink-logger
   # menampilkan ALERT, REVERSE, ENCRYPT
   ```
4. **File asli** tetap aman:

   ```bash
   cat data/foo.txt
   cat data/kimcun_plan.txt
   ```

---

## 🧹 Cleanup


```bash
docker-compose down --volumes
```

## Soal 4
### Deskripsi
SEGA menunjuk kita sebagai administrator sistem file chiho di universe maimai. Tugas utama adalah memastikan 7 area dalam filesystem chiho/ berfungsi sesuai spesifikasi. Praktikum kali ini hanya mengimplementasikan 3 area pertama:
1. Starter Area (/starter)
- File disimpan dengan ekstensi .mai di direktori asli (chiho/starter).
- Ekstensi tersebut disembunyikan dari pengguna saat diakses melalui fuse_dir/starter.
2. Metropolis Area (/metro)
- File disimpan dengan ekstensi .ccc.
- Konten file di-encode menggunakan algoritma shifting berdasarkan posisi byte (i % 256).
- Saat dibaca, dilakukan decode untuk mengembalikan isi file ke bentuk asli.
3. Dragon Area (/dragon)
- File disimpan dengan ekstensi .rot.
- Isi file di-encrypt menggunakan algoritma ROT13 saat ditulis dan didekripsi saat dibaca.
```
├── chiho/
│   ├── starter/
│   ├── metro/
│   ├── dragon/
│   └── ...
└── fuse_dir/
    ├── starter/
    ├── metro/
    ├── dragon/
    └── ...
```
### Code maimai_fs.c
#### Konstanta dan Fungsi Utility
1. Menentukan versi API FUSE yang digunakan.
```
#define FUSE_USE_VERSION 31
```
2. Path asli tempat file disimpan di luar FUSE mount point.
```
static const char *dirpath_chiho = "/home/kali/Documents/soal_4_modul4/soal_4/chiho";
```
#### Fungsi Enkripsi/Encoding
1. Melakukan encoding shift posisi karakter berdasarkan index: dst[i] = src[i] + (i % 256);
Digunakan di metro (Metropolis Chiho).
```
void encode(const char* src, char* dst, int len)
```
```
void decode(...) // kebalikan dari encode
```
2. Enkripsi ROT13 (menggeser huruf alfabet sebanyak 13 posisi). Non-huruf tidak berubah.
```
void rot13(...) // digunakan untuk Dragon Chiho
```
#### Fungsi Operasi FUSE
##### Mengubah path FUSE ke path asli di chiho/, menambahkan ekstensi (getattr)
- starter: .mai
- metro: .ccc
- dragon: .rot
Contoh: /starter/abc → chiho/starter/abc.mai
```
static int xmp_getattr(const char *path, struct stat *stbuf)
```
##### readdir
- Membaca isi direktori.
- Memfilter dan menghilangkan ekstensi .mai, .ccc, .rot saat ditampilkan di direktori FUSE.
```
static int xmp_readdir(...)
```
##### open
- Membuka file asli berdasarkan path FUSE.
- Sama seperti getattr, menambahkan ekstensi berdasarkan area.
```
static int xmp_open(const char *path, struct fuse_file_info *fi)
```
##### read
Membaca isi file:
- Untuk metro: decode isi file sebelum diberikan ke FUSE.
- Untuk dragon: lakukan ROT13 decoding.
- Untuk lainnya (misal starter), langsung baca biasa (via pread).
```
static int xmp_read(...)
```
##### write
Menulis ke file:
- Pertama baca isi file dan decode (metro dan dragon).
- Gabungkan dengan buffer baru (data baru).
- Encode kembali sebelum disimpan (encode atau rot13).
```
static int xmp_write(...)
```
##### create
Membuat file baru dengan ekstensi:
- starter: .mai
- metro: .ccc
- dragon: .rot
##### unlink
Menghapus file. Sama, mengubah path dan menambahkan ekstensi sebelum menjalankan unlink().
```
static int xmp_unlink(const char *path)
```
##### xmp_oper dan main
1. Mendaftarkan fungsi-fungsi yang diimplementasikan ke dalam struktur operasi FUSE.
```
static struct fuse_operations xmp_oper = {...}
```
2. Menjalankan filesystem FUSE dengan operasi yang sudah didefinisikan.
```
int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```
#### Code lengkap
```
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
```
