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

