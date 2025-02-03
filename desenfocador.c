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

void *blur_filter(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    applyBlurFilter(data->img, data->start_row, data->end_row);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Uso: desenfocador <num_threads>");
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
    int num_threads = atoi(argv[1]);
    int rows_per_thread = mid / num_threads;

    // Crear hilos
    pthread_t threads[num_threads];
    ThreadData threadData[num_threads];
    for (int i = 0; i < num_threads; i++) {
        threadData[i].start_row = rows_per_thread * i;
        threadData[i].end_row = (i == num_threads - 1) ? mid : rows_per_thread * (i + 1);
        threadData[i].img = &img;
        pthread_create(&threads[i], NULL, blur_filter, &threadData[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Desenfoque aplicado.\n");
    munmap(shm_ptr, shm_stat.st_size);
    close(shm_fd);
    return 0;
}
