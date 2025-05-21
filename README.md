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
