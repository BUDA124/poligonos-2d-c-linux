# ============================================================================
# Makefile — Proyecto 1: Mapa de Costa Rica en 2D
# Computación Gráfica
# ============================================================================
#
# Uso:
#   make            Compilar el proyecto
#   make run        Compilar y ejecutar
#   make clean      Limpiar archivos generados
#   make valgrind   Ejecutar con Valgrind (detección de fugas de memoria)
#   make tar        Empaquetar el proyecto para entrega
#
# ============================================================================

# Compilador y flags estrictos (C99)
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -pedantic -O2
LDFLAGS  = -lGL -lGLU -lglut -lm

# Nombre del ejecutable
TARGET   = mapa_cr

# Directorios
SRC_DIR  = src

# Archivos fuente
SRCS     = $(SRC_DIR)/main.c       \
           $(SRC_DIR)/framebuffer.c \
           $(SRC_DIR)/geometry.c   \
           $(SRC_DIR)/transform.c  \
           $(SRC_DIR)/clipping.c   \
           $(SRC_DIR)/stubs.c

# Archivos objeto
OBJS     = $(SRCS:.c=.o)

# Nombre del paquete para entrega
TAR_NAME = proyecto1_cg

# ============================================================================
# Reglas
# ============================================================================

.PHONY: all clean run valgrind tar

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(TAR_NAME).tgz

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

tar: clean
	tar -czvf $(TAR_NAME).tgz \
		--exclude='*.o' \
		--exclude='$(TARGET)' \
		--exclude='$(TAR_NAME).tgz' \
		--exclude='.git' \
		Makefile README.md src/ data/

