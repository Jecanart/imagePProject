CC = gcc
CFLAGS = -Wall -Wextra -pthread -lrt
TARGETS = publicador desenfocador realzador

# Definir los archivos fuente
SRCS = publicador.c desenfocador.c realzador.c utils.c
OBJS = $(SRCS:.c=.o)

# Compilar todos los programas
all: $(TARGETS)

# Compilar el Publicador
publicador: publicador.o utils.o
	$(CC) $(CFLAGS) -o publicador publicador.o utils.o

# Compilar el Desenfocador
desenfocador: desenfocador.o utils.o
	$(CC) $(CFLAGS) -o desenfocador desenfocador.o utils.o

# Compilar el Realzador
realzador: realzador.o utils.o
	$(CC) $(CFLAGS) -o realzador realzador.o utils.o

# Compilar archivos
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar archivos objeto y ejecutables
clean:
	rm -f $(TARGETS) *.o

# Regla para ejecutar todos los programas en paralelo
run: all
	./publicador &
	./desenfocador &
	./realzador &

.PHONY: all clean run
