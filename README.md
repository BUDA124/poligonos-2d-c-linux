# Proyecto 1 — Manejo de Polígonos en 2D: Mapa de Costa Rica

## Descripción

Aplicación gráfica interactiva en **C puro** que renderiza un mapa de Costa Rica
utilizando un **framebuffer de software** (CPU) con MESA/FreeGLUT como backend de
ventana.  El proyecto implementa tres modos de visualización:

1. **Wireframe**: Contornos con algoritmo de Bresenham.
2. **Sólido**: Relleno con Scanline fill.
3. **Textura**: Mapeo de texturas `.avs` sobre los polígonos.

## Requisitos del Sistema

- **Sistema Operativo**: Linux (probado en Ubuntu 22.04+)
- **Compilador**: GCC con soporte C99
- **Dependencias**:
  ```bash
  sudo apt-get install build-essential freeglut3-dev mesa-utils
  ```

## Compilación y Ejecución

```bash
# Compilar
make

# Ejecutar
make run

# Limpiar archivos generados
make clean

# Análisis de memoria con Valgrind
make valgrind

# Empaquetar para entrega
make tar
```

## Controles

| Tecla          | Acción                        |
|----------------|-------------------------------|
| `1`            | Modo wireframe (bordes)       |
| `2`            | Modo sólido (relleno)         |
| `3`            | Modo textura                  |
| `Flechas`      | Mover el mapa (pan)           |
| `+` / `-`      | Zoom in / out                 |
| `r` / `R`      | Rotar CW / CCW                |
| `0`            | Restablecer vista por defecto |
| `q` / `ESC`    | Salir                         |
| `SHIFT + tecla`| Velocidad rápida              |
| `CTRL + tecla` | Velocidad lenta               |

## Estructura del Proyecto

```
CG_P1/
├── Makefile                 # Sistema de compilación
├── README.md                # Este archivo
├── src/
│   ├── main.c               # Bucle GLUT y callbacks (Persona 1)
│   ├── framebuffer.h/.c     # Framebuffer de software (Persona 1)
│   ├── app_state.h          # Estado global y modos (Persona 1)
│   ├── geometry.h           # Tipos y carga de datos (Persona 2)
│   ├── transform.h          # Transformaciones 2D (Persona 2)
│   ├── clipping.h           # Recorte de líneas/polígonos (Persona 2)
│   ├── raster.h             # Bresenham y Scanline (Persona 3)
│   ├── texture.h            # Texturas .avs (Persona 4)
│   └── stubs.c              # Implementaciones provisionales
└── data/
    ├── provincias/           # Archivos de vértices por provincia
    └── texturas/             # Archivos de textura .avs
```

## Roles del Equipo

| Persona | Responsabilidad                                            |
|---------|------------------------------------------------------------|
| 1 (Líder)| Framebuffer, ventana GLUT, arquitectura, Makefile         |
| 2       | Geometría, transformaciones, clipping                      |
| 3       | Rasterización: Bresenham y Scanline fill                   |
| 4       | Carga de texturas .avs y mapeo de texturas                 |

## Formato de Archivos de Datos (`data/provincias/`)

Cada archivo de provincia (ej. `san_jose.txt`) sigue una estructura numérica simple:
```text
P              <-- Cantidad de polígonos que forman la provincia (islas/componentes)
N_1            <-- Cantidad de vértices del polígono 1
x_1 y_1        <-- Vértices en coordenadas continuas (flotantes)
x_2 y_2
...
x_N1 y_N1
N_2            <-- Cantidad de vértices del polígono 2 (si P > 1)
x_1 y_1
...
```

Un archivo de ejemplo inicial se encuentra en `data/provincias/san_jose.txt`.

## Protocolo de Integración

1. Cada persona trabaja en sus archivos `.c` correspondientes (ej. `geometry.c`, `raster.c`, `texture.c`).
2. Los headers (`.h`) son contratos **inmutables** definidos por el Líder.
3. Antes de fusionar código, debe:
   - Compilar sin warnings con `make` (`-Wall -Wextra -pedantic`)
   - Pasar `make valgrind` sin fugas de memoria
4. Al integrar, reemplazar las funciones provisionales de `stubs.c` por los archivos
   `.c` reales y actualizar la lista `SRCS` del `Makefile`.

