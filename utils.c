#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

BMPImage readBMP(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error al abrir el archivo");
        return (BMPImage){NULL, 0, 0, 0};
    }

    fseek(file, 18, SEEK_SET);
    int width, height;
    fread(&width, sizeof(int), 1, file);
    fread(&height, sizeof(int), 1, file);

    fseek(file, 0, SEEK_SET);
    int size = width * height * 3 + 54; // Tamaño total del archivo BMP
    unsigned char *data = malloc(size);
    fread(data, size, 1, file);
    fclose(file);

    return (BMPImage){data, width, height, size};
}

BMPImage loadBMPFromMemory(void *ptr, int size) {
    unsigned char *data = (unsigned char *)ptr;
    int width = *(int *)&data[18];
    int height = *(int *)&data[22];

    return (BMPImage){data, width, height, size};
}

void applyBlurFilter(BMPImage *img, int start_row, int end_row) {
    int width = img->width;
    int height = img->height;
    unsigned char *data = img->data;

    int kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    int kernel_sum = 9;

    for (int y = start_row; y < end_row; y++) {
        for (int x = 1; x < width - 1; x++) {
            int r = 0, g = 0, b = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = (y + ky) * width + (x + kx);
                    r += data[px * 3 + 2] * kernel[ky + 1][kx + 1];
                    g += data[px * 3 + 1] * kernel[ky + 1][kx + 1];
                    b += data[px * 3 + 0] * kernel[ky + 1][kx + 1];
                }
            }

            int index = y * width + x;
            data[index * 3 + 2] = r / kernel_sum;
            data[index * 3 + 1] = g / kernel_sum;
            data[index * 3 + 0] = b / kernel_sum;
        }
    }
}

void applyEdgeDetection(BMPImage *img, int start_row, int end_row) {
    int width = img->width;
    int height = img->height;
    unsigned char *data = img->data;

    int kernel[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

    for (int y = start_row; y < end_row; y++) {
        for (int x = 1; x < width - 1; x++) {
            int r = 0, g = 0, b = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = (y + ky) * width + (x + kx);
                    r += data[px * 3 + 2] * kernel[ky + 1][kx + 1];
                    g += data[px * 3 + 1] * kernel[ky + 1][kx + 1];
                    b += data[px * 3 + 0] * kernel[ky + 1][kx + 1];
                }
            }

            int index = y * width + x;
            data[index * 3 + 2] = r > 255 ? 255 : (r < 0 ? 0 : r);
            data[index * 3 + 1] = g > 255 ? 255 : (g < 0 ? 0 : g);
            data[index * 3 + 0] = b > 255 ? 255 : (b < 0 ? 0 : b);
        }
    }
}

void writeBMP(const char *filename, BMPImage *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error al guardar la imagen");
        return;
    }

    fwrite(img->data, 1, img->size, file);
    fclose(file);
    printf("Imagen guardada en: %s\n", filename);
}



