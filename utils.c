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
    BMPImage img;
    img.data = (unsigned char *)ptr;
    img.width = *(int *)&img.data[18];  
    img.height = *(int *)&img.data[22]; 
    img.size = size; 

    return img;
}

void applyBlurFilter(BMPImage *img, int start_row, int end_row) {
    int kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    int kernel_sum = 9;

    int width = img->width;
    int height = img->height;
    unsigned char *data = img->data;

    // Calcular el tamaño de las filas con padding
    int row_padded = (width * 3 + 3) & (~3);

    // Crear un buffer temporal para almacenar los resultados
    unsigned char *temp_data = malloc(row_padded * height);
    memcpy(temp_data, data + 54, row_padded * height);  // Copiar solo los datos de píxeles

    // Aplicar el filtro de desenfoque
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0;

            // Recorrer el kernel
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel_index = ((y + ky) * row_padded) + ((x + kx) * 3);
                    r += temp_data[pixel_index + 2] * kernel[ky + 1][kx + 1]; // Rojo
                    g += temp_data[pixel_index + 1] * kernel[ky + 1][kx + 1]; // Verde
                    b += temp_data[pixel_index + 0] * kernel[ky + 1][kx + 1]; // Azul
                }
            }

            // Guardar el resultado en el buffer original
            int pixel_index = (y * row_padded) + (x * 3);
            data[54 + pixel_index + 2] = r / kernel_sum;  // Rojo
            data[54 + pixel_index + 1] = g / kernel_sum;  // Verde
            data[54 + pixel_index + 0] = b / kernel_sum;  // Azul
        }
    }

    free(temp_data);  // Liberar el buffer temporal
}

void applyEdgeDetection(BMPImage *img, int start_row, int end_row) {
    int kernel[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
    int kernel_sum = 9;

    int width = img->width;
    int height = img->height;
    unsigned char *data = img->data;

    // Calcular el tamaño de las filas con padding
    int row_padded = (width * 3 + 3) & (~3);

    // Crear un buffer temporal para almacenar los resultados
    unsigned char *temp_data = malloc(row_padded * height);
    memcpy(temp_data, data + 54, row_padded * height);  // Copiar solo los datos de píxeles

    // Aplicar el filtro de desenfoque
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0;

            // Recorrer el kernel
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel_index = ((y + ky) * row_padded) + ((x + kx) * 3);
                    r += temp_data[pixel_index + 2] * kernel[ky + 1][kx + 1]; // Rojo
                    g += temp_data[pixel_index + 1] * kernel[ky + 1][kx + 1]; // Verde
                    b += temp_data[pixel_index + 0] * kernel[ky + 1][kx + 1]; // Azul
                }
            }

            // Guardar el resultado en el buffer original
            int pixel_index = (y * row_padded) + (x * 3);
            data[54 + pixel_index + 2] = r / kernel_sum;  // Rojo
            data[54 + pixel_index + 1] = g / kernel_sum;  // Verde
            data[54 + pixel_index + 0] = b / kernel_sum;  // Azul
        }
    }

    free(temp_data);  // Liberar el buffer temporal
}

void writeBMP(const char *filename, BMPImage *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error al guardar la imagen");
        return;
    }

    // Escribir la cabecera BMP (primeros 54 bytes)
    if (fwrite(img->data, 1, 54, file) != 54) {
        printf("Error al escribir la cabecera BMP.\n");
        fclose(file);
        return;
    }

    // Calcular el tamaño de los datos de píxeles
    int row_padded = (img->width * 3 + 3) & (~3);  // Alinear filas a múltiplo de 4 bytes
    int pixel_data_size = row_padded * img->height;

    // Escribir los datos de los píxeles
    if (fwrite(img->data + 54, 1, pixel_data_size, file) != (size_t)pixel_data_size) {
        printf("Error al escribir los datos de los píxeles.\n");
        fclose(file);
        return;
    }

    fclose(file);
    printf("Imagen guardada correctamente en: %s\n", filename);
}
