#include <sys/stat.h>

#include <stdio.h>
#include <errno.h>

#include <zbar.h>

#if !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wshift-negative-value"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_BMP
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_JPEG
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_PSD
#define STBI_NO_TGA
#include "stb_image.h"
#if !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif

int main(int argc, char **argv)
{
    argc--;
    argv++;
    const char *filename = *argv;

    struct stat test;
    if (!argc || stat(filename, &test) != 0)
        filename = "example/test.png";
    if (stat(filename, &test) != 0) {
        fprintf(stderr, "failed to load test: %s (%s)\n", filename, strerror(errno));
        return 1;
    }

    zbar_image_t *image = zbar_image_create();
    zbar_image_scanner_t *scanner = zbar_image_scanner_create();

    zbar_image_scanner_set_config(scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    zbar_image_scanner_enable_cache(scanner, 1);

    int width = 0;
    int height = 0;
    int components = 0;
    unsigned char *data = stbi_load(filename, &width, &height, &components, 1);
    if (!data) {
        fprintf(stderr, "failed to load test: %s (image loading failed)\n", filename);
        goto error;
    }

    zbar_image_set_format(image, *(int *)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, data, width * height, NULL);

    int decoded = zbar_scan_image(scanner, image);
    if (decoded <= 0) {
        fprintf(stderr, "failed to scan image\n");
        goto error;
    }

    char *decode_data = 0;
    size_t decode_size = 0;

    for (const zbar_symbol_t *symbol = zbar_image_first_symbol(image); symbol; symbol = zbar_symbol_next(symbol)) {
        const char *symbol_data = zbar_symbol_get_data(symbol);
        size_t symbol_length = zbar_symbol_get_data_length(symbol);
        decode_data = realloc(decode_data, decode_size + symbol_length);
        memcpy(decode_data + decode_size, symbol_data, symbol_length);
        decode_size += symbol_length;
    }
    stbi_image_free(data);

    decode_data = realloc(decode_data, decode_size + 1);
    decode_data[decode_size] = '\0';

    char *find = strrchr(decode_data, '\n');
    if (!find)
        goto failed;

    *find = '\0';

    printf("success: %s\n", decode_data);

    free(decode_data);
    zbar_image_scanner_destroy(scanner);
    zbar_image_destroy(image);

    return 0;

failed:
    fprintf(stderr, "failed: expected: `zbar-lite` got: `%s`\n", decode_data);
    free(decode_data);

error:
    zbar_image_scanner_destroy(scanner);
    zbar_image_destroy(image);

    return 1;
}
