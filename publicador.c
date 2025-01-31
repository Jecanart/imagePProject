#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "utils.h"  // Funciones propias de BMP

#define SHM_NAME "/shm_image"

int main() {
    char image_path[256];
    BMPImage img;

    while (1) {
        printf("Ingrese la ruta de la imagen BMP: ");   
        // Usar fgets para leer la entrada
        scanf("%s", image_path);

       
        
        if(image_path != NULL){
            break;
        };

    }
    img = readBMP(image_path);

    // Crear memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, img.size);
    void *shm_ptr = mmap(0, img.size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    memcpy(shm_ptr, img.data, img.size);
    printf("Imagen cargada en memoria compartida.\n");

    // Crear semáforos
    sem_t *sem_proc = sem_open("/sem_proc", O_CREAT, 0666, 0);

    // Liberar semáforo para iniciar procesamiento
    sem_post(sem_proc);

    // Limpiar recursos
    munmap(shm_ptr, img.size);
    close(shm_fd);
    sem_close(sem_proc);
    printf("Publicador finalizado.\n");

    return 0;
}
