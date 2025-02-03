#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <getopt.h>
#include "utils.h"  // Funciones propias de BMP

#define SHM_NAME "/shm_image"

void print_help(char *command) {
    printf("Publicador lee una imagen de entrada en formato bmp, la coloca en un recurso compartido, y\n");
    printf("mediante llamadas a Desenfocador y Realzador devuelve una nueva imagen modificada.\n");
    printf("uso:\n %s [-t <threads>]\n", command);
    printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
	printf(" -t\t\t\tEspecifica la cantidad de threads/hilos a utilizar\n");
}

int main(int argc, char **argv) {
    BMPImage img;
    char *num_threads = "4";
    char image_path[256];
    int opt;

    while((opt = getopt (argc, argv, "ht:")) != -1) {
		switch(opt) {
			case 'h':
				print_help(argv[0]);
				return 0;
			case 't':
				num_threads = optarg;
				break;
			case '?':
				// Manejo de opciones sin argumentos o desconocidas
				if (optopt == 't') {
					fprintf(stderr, "Opción -%c requiere un argumento.\n", optopt);
				} else {
					fprintf(stderr, "Opción '-%c' desconocida.\n", optopt);
				}
                fprintf(stderr, "uso:\n %s [-t <threads>]\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
                break;
			default:
				fprintf(stderr, "uso:\n %s [-t <threads>]\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return 1;
		}
	}

    while (1) {
        // fgets para leer la entrada
        printf("Ingrese la ruta de la imagen BMP: ");

        // EOF
        if (fgets(image_path, sizeof(image_path), stdin) == NULL) {
            printf("\nError o EOF encontrado\n");
            exit(EXIT_FAILURE);
        }
        // Entrada vacía
        if (image_path[0] == '\n') {
            printf("Entrada vacía. Intente de nuevo\n");
            continue;
        }
        // Remover newline
        image_path[strcspn(image_path, "\n")] = '\0';
        img = readBMP(image_path);

        // Archivo inexistente
        if (img.data == NULL) {
            printf("Intente de nuevo\n");
            continue;
        }
        printf("Imagen: %s\n", image_path);

        // Crear memoria compartida
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, img.size);
        void *shm_ptr = mmap(0, img.size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        memcpy(shm_ptr, img.data, img.size);
        printf("Imagen cargada en memoria compartida.\n");

        // Crear procesos independientes
        pid_t pid1, pid2;

        if ((pid1 = fork()) == 0) {
            execlp("./desenfocador", "./desenfocador", num_threads, (char *)NULL);
            perror("execlp ha fallado");
            exit(EXIT_FAILURE);
        }

        if ((pid2 = fork()) == 0) {
            execlp("./realzador", "./realzador", (char *)NULL);
            perror("execlp ha fallado");
            exit(EXIT_FAILURE);
        }

        int status;
        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);

        // Recoger trabajo realizado
        BMPImage result = loadBMPFromMemory(shm_ptr, img.size);
        if (!result.data) {
            printf("Error al cargar la imagen desde la memoria compartida.\n");
            return 1;
        }

        // Guardar nueva imagen en un archivo
        writeBMP("imagen_modificada.bmp", &result);
        printf("Publicador finalizado.\n");

        // Limpiar recursos
        free(img.data);
        munmap(shm_ptr, img.size);
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }

    return 0;
}
