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
