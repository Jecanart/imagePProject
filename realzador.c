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

typedef struct {
    int start_row, end_row;
    BMPImage *img;
} ThreadData;

void *edge_filter(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    applyEdgeDetection(data->img, data->start_row, data->end_row);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {

    if (argc != 1) {
        printf("Uso: realzador");
        return 1;
    }

    // Acceder a la memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    struct stat shm_stat;
    fstat(shm_fd, &shm_stat);
    void *shm_ptr = mmap(0, shm_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Cargar la imagen desde la memoria compartida
    BMPImage img = loadBMPFromMemory(shm_ptr, shm_stat.st_size);
    if (!img.data) {
        printf("Error al cargar la imagen desde la memoria compartida.\n");
        return 1;
    }

    int mid = img.height / 2;

    applyEdgeDetection(&img, mid, img.height);
    
    printf("Realce de bordes aplicado.\n");
    munmap(shm_ptr, shm_stat.st_size);
    close(shm_fd);
    return 0;
}
