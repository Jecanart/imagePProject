#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>


#define SHM_NAME "/shm_image"

typedef struct {
    int start_row, end_row;
    BMPImage *img;
} ThreadData;

void *edge_filter(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    applyEdgeDetection(data->img, data->start_row, data->end_row);
    pthread_exit(NULL);
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    struct stat shm_stat;
    fstat(shm_fd, &shm_stat);
    void *shm_ptr = mmap(0, shm_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    BMPImage img = loadBMPFromMemory(shm_ptr, shm_stat.st_size);
    int mid = img.height / 2;

    applyEdgeDetection(&img, mid, img.height);
    
    writeBMP("imagen_modificada.bmp", &img);

    printf("Realce de bordes aplicado.\n");

    return 0;
}
