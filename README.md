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
