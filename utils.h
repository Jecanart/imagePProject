#ifndef UTILS_H
#define UTILS_H

#define SHM_NAME "/shm_image"
#define THREADS 4

typedef struct {
    unsigned char *data;
    int width, height, size;
} BMPImage;

BMPImage readBMP(const char *filename);
BMPImage loadBMPFromMemory(void *ptr, int size);
void applyBlurFilter(BMPImage *img, int start, int end);
void applyEdgeDetection(BMPImage *img, int start, int end);
void writeBMP(const char *filename, BMPImage *img);

#endif
