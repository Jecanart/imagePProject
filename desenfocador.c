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
#define THREADS 4

typedef struct {
    int start_row, end_row;
    BMPImage *img;
} ThreadData;

void *blur_filter(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    applyBlurFilter(data->img, data->start_row, data->end_row);
    pthread_exit(NULL);
}

int main() {
    sem_t *sem_proc = sem_open("/sem_proc", O_CREAT, 0666, 0);
    sem_wait(sem_proc);

    // Acceder a la memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    struct stat shm_stat;
    fstat(shm_fd, &shm_stat);
    void *shm_ptr = mmap(0, shm_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    BMPImage img = loadBMPFromMemory(shm_ptr, shm_stat.st_size);
    int mid = img.height / 2;

    // Crear hilos
    pthread_t threads[THREADS];
    ThreadData threadData[THREADS];
    for (int i = 0; i < THREADS; i++) {
        threadData[i].start_row = (mid / THREADS) * i;
        threadData[i].end_row = (mid / THREADS) * (i + 1);
        threadData[i].img = &img;
        pthread_create(&threads[i], NULL, blur_filter, &threadData[i]);
    }
    
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Desenfoque aplicado.\n");
    sem_close(sem_proc);
    return 0;
}
