/**
 * @file texture.c
 * @brief Carga de texturas AVS y relleno texturizado de polígonos.
 */

#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

/* ========================================================================= */
/*  Utilidades                                                               */
/* ========================================================================= */

/**
 * @brief Lee un entero de 32 bits almacenado en Big Endian.
 */
static int read_uint32_be(FILE *file, uint32_t *value)
{
    unsigned char bytes[4];

    if (!file || !value)
        return -1;

    if (fread(bytes, 1, 4, file) != 4)
        return -1;

    *value = ((uint32_t)bytes[0] << 24) |
             ((uint32_t)bytes[1] << 16) |
             ((uint32_t)bytes[2] << 8)  |
             (uint32_t)bytes[3];

    return 0;
}

/**
 * @brief Envuelve un índice dentro de un rango [0, size).
 *
 * Funciona también con índices negativos.
 *
 * Ejemplo con size = 800:
 *   800 -> 0
 *   801 -> 1
 *   -1  -> 799
 */
static int wrap_index(int value, int size)
{
    int result;

    if (size <= 0)
        return 0;

    result = value % size;

    if (result < 0)
        result += size;

    return result;
}

/* ========================================================================= */
/*  Carga de AVS                                                             */
/* ========================================================================= */

int texture_load_avs(const char *filepath, Texture *out)
{
    FILE *file;
    uint32_t width;
    uint32_t height;
    size_t pixel_count;
    size_t data_size;

    if (!filepath || !out)
        return -1;

    out->width = 0;
    out->height = 0;
    out->channels = 0;
    out->pixels = NULL;

    file = fopen(filepath, "rb");

    if (!file)
        return -1;

    /*
     * Formato AVS utilizado por el proyecto:
     *
     * 4 bytes -> width
     * 4 bytes -> height
     * 4 bytes por píxel -> A R G B
     */
    if (read_uint32_be(file, &width) != 0 ||
        read_uint32_be(file, &height) != 0) {

        fclose(file);
        return -1;
    }

    if (width == 0 ||
        height == 0 ||
        width > (uint32_t)INT_MAX ||
        height > (uint32_t)INT_MAX) {

        fclose(file);
        return -1;
    }

    /* Evitar overflow al calcular width * height. */
    if ((size_t)width > SIZE_MAX / (size_t)height) {
        fclose(file);
        return -1;
    }

    pixel_count = (size_t)width * (size_t)height;

    /* Cada píxel AVS tiene 4 bytes: A R G B. */
    if (pixel_count > SIZE_MAX / 4u) {
        fclose(file);
        return -1;
    }

    data_size = pixel_count * 4u;

    out->pixels = (unsigned char *)malloc(data_size);

    if (!out->pixels) {
        fclose(file);
        return -1;
    }

    if (fread(out->pixels, 1, data_size, file) != data_size) {
        free(out->pixels);
        out->pixels = NULL;

        fclose(file);
        return -1;
    }

    fclose(file);

    out->width = (int)width;
    out->height = (int)height;
    out->channels = 4;

    return 0;
}

/* ========================================================================= */
/*  Liberación                                                               */
/* ========================================================================= */

void texture_free(Texture *tex)
{
    if (!tex)
        return;

    free(tex->pixels);

    tex->pixels = NULL;
    tex->width = 0;
    tex->height = 0;
    tex->channels = 0;
}

/* ========================================================================= */
/*  Muestreo de textura                                                      */
/* ========================================================================= */

Color texture_sample(const Texture *tex, float u, float v)
{
    Color black = {0, 0, 0};

    int x;
    int y;
    size_t index;

    if (!tex ||
        !tex->pixels ||
        tex->width <= 0 ||
        tex->height <= 0 ||
        tex->channels < 3) {

        return black;
    }

    /*
     * Las coordenadas UV se repiten cada 1.0.
     *
     * 0.0 -> 0.0
     * 1.0 -> 0.0
     * 1.2 -> 0.2
     * -0.1 -> 0.9
     */
    u = u - floorf(u);
    v = v - floorf(v);

    x = (int)(u * (float)tex->width);
    y = (int)(v * (float)tex->height);

    /*
     * Seguridad ante errores de redondeo.
     */
    x = wrap_index(x, tex->width);
    y = wrap_index(y, tex->height);

    /*
     * AVS almacena la imagen de arriba hacia abajo,
     * mientras nuestro framebuffer usa origen abajo a la izquierda.
     */
    y = tex->height - 1 - y;

    index = ((size_t)y * (size_t)tex->width +
             (size_t)x) * (size_t)tex->channels;

    /*
     * AVS:
     *   [0] = A
     *   [1] = R
     *   [2] = G
     *   [3] = B
     */
    if (tex->channels >= 4) {
        black.r = tex->pixels[index + 1];
        black.g = tex->pixels[index + 2];
        black.b = tex->pixels[index + 3];
    } else {
        black.r = tex->pixels[index + 0];
        black.g = tex->pixels[index + 1];
        black.b = tex->pixels[index + 2];
    }

    return black;
}

/* ========================================================================= */
/*  Relleno texturizado                                                      */
/* ========================================================================= */

void fill_polygon_texture(Framebuffer *fb,
                          const Polygon *poly,
                          const Texture *tex,
                          const BoundingBox *bbox)
{
    typedef struct {
        int y_min;
        int y_max;
        float x;
        float dx_dy;
    } Edge;

    Edge *edges;
    Edge *active;

    int edge_count = 0;
    int active_count;

    int min_y;
    int max_y;

    int i;
    int y;

    (void)bbox;

    if (!fb ||
        !poly ||
        !poly->vertices ||
        poly->count < 3 ||
        !tex ||
        !tex->pixels ||
        tex->width <= 0 ||
        tex->height <= 0) {

        return;
    }

    /* ------------------------------------------------------------- */
    /* Bounding box vertical del polígono.                           */
    /* ------------------------------------------------------------- */

    min_y = (int)floorf(poly->vertices[0].y);
    max_y = (int)ceilf(poly->vertices[0].y);

    for (i = 1; i < poly->count; i++) {
        int vertex_min_y = (int)floorf(poly->vertices[i].y);
        int vertex_max_y = (int)ceilf(poly->vertices[i].y);

        if (vertex_min_y < min_y)
            min_y = vertex_min_y;

        if (vertex_max_y > max_y)
            max_y = vertex_max_y;
    }

    /*
     * El framebuffer es finito.
     * No necesitamos procesar scanlines que están fuera.
     */
    if (min_y < 0)
        min_y = 0;

    if (max_y >= fb->height)
        max_y = fb->height - 1;

    if (min_y > max_y)
        return;

    /* ------------------------------------------------------------- */
    /* Construir tabla de aristas.                                  */
    /* ------------------------------------------------------------- */

    edges = (Edge *)malloc(
        sizeof(Edge) * (size_t)poly->count
    );

    if (!edges)
        return;

    for (i = 0; i < poly->count; i++) {
        int next = (i + 1) % poly->count;

        int x0 = (int)poly->vertices[i].x;
        int y0 = (int)poly->vertices[i].y;

        int x1 = (int)poly->vertices[next].x;
        int y1 = (int)poly->vertices[next].y;

        /*
         * Los bordes horizontales se ignoran,
         */
        if (y0 == y1)
            continue;

        if (y0 < y1) {
            edges[edge_count].y_min = y0;
            edges[edge_count].y_max = y1;
            edges[edge_count].x = (float)x0;
            edges[edge_count].dx_dy =
                (float)(x1 - x0) / (float)(y1 - y0);
        } else {
            edges[edge_count].y_min = y1;
            edges[edge_count].y_max = y0;
            edges[edge_count].x = (float)x1;
            edges[edge_count].dx_dy =
                (float)(x0 - x1) / (float)(y0 - y1);
        }

        edge_count++;
    }

    if (edge_count == 0) {
        free(edges);
        return;
    }

    active = (Edge *)malloc(
        sizeof(Edge) * (size_t)edge_count
    );

    if (!active) {
        free(edges);
        return;
    }

    /* ------------------------------------------------------------- */
    /* Scanline.                                                      */
    /* ------------------------------------------------------------- */

    for (y = min_y; y <= max_y; y++) {

        active_count = 0;

        /*
         * Encontrar intersecciones de la scanline con las aristas.
         */
        for (i = 0; i < edge_count; i++) {

            if (y >= edges[i].y_min &&
                y < edges[i].y_max) {

                active[active_count] = edges[i];

                active[active_count].x =
                    edges[i].x +
                    edges[i].dx_dy *
                    (float)(y - edges[i].y_min);

                active_count++;
            }
        }

        /* --------------------------------------------------------- */
        /* Ordenar intersecciones.                                  */
        /* --------------------------------------------------------- */

        for (i = 0; i < active_count - 1; i++) {
            int j;

            for (j = i + 1; j < active_count; j++) {

                if (active[j].x < active[i].x) {
                    Edge temp = active[i];
                    active[i] = active[j];
                    active[j] = temp;
                }
            }
        }

        /* --------------------------------------------------------- */
        /* Rellenar entre pares.                                    */
        /* --------------------------------------------------------- */

        for (i = 0; i + 1 < active_count; i += 2) {

            int x_start = (int)ceilf(active[i].x);
            int x_end   = (int)floorf(active[i + 1].x);

            int x;

            if (x_start < 0)
                x_start = 0;

            if (x_end >= fb->width)
                x_end = fb->width - 1;

            if (x_start > x_end)
                continue;

            for (x = x_start; x <= x_end; x++) {

                /*
                 * Convertimos el píxel del framebuffer a UV.
                 *
                 * Al dividir entre el tamaño de la textura:
                 *
                 * x = 0   -> u = 0.0
                 * x = TH  -> u = 1.0 -> vuelve a 0
                 *
                 * Esto reproduce:
                 *
                 *     TEXTURA[x % TH][y % TV]
                 */
                float u = (float)x / (float)tex->width;
                float v = (float)y / (float)tex->height;

                Color color = texture_sample(tex, u, v);

                framebuffer_put_pixel(
                    fb,
                    x,
                    y,
                    color
                );
            }
        }
    }

    free(active);
    free(edges);
}