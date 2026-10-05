/**
 * @file texture.c
 * @brief Carga de texturas AVS y relleno texturizado de polígonos.
 *
 */

#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

/* ========================================================================= */
/*  Lectura de enteros Big Endian                                           */
/* ========================================================================= */

/**
 * @brief Lee un entero unsigned de 32 bits en Big Endian.
 *
 * El formato AVS almacena width y height utilizando 4 bytes Big Endian.
 */
static int read_uint32_be(FILE *file, uint32_t *value)
{
    unsigned char bytes[4];

    if (!file || !value) {
        return -1;
    }

    if (fread(bytes, 1, 4, file) != 4) {
        return -1;
    }

    *value = ((uint32_t)bytes[0] << 24) |
             ((uint32_t)bytes[1] << 16) |
             ((uint32_t)bytes[2] << 8)  |
             ((uint32_t)bytes[3]);

    return 0;
}

/* ========================================================================= */
/*  Carga de AVS                                                            */
/* ========================================================================= */

int texture_load_avs(const char *filepath, Texture *out)
{
    FILE *file;
    uint32_t width;
    uint32_t height;
    size_t pixel_count;
    size_t data_size;

    if (!filepath || !out) {
        return -1;
    }

    out->width = 0;
    out->height = 0;
    out->channels = 0;
    out->pixels = NULL;

    file = fopen(filepath, "rb");
    if (!file) {
        return -1;
    }

    /*
     * AVS:
     *
     * 4 bytes -> width
     * 4 bytes -> height
     * da -> píxeles A R G B
     */
    if (read_uint32_be(file, &width) != 0 ||
        read_uint32_be(file, &height) != 0) {
        fclose(file);
        return -1;
    }

    /* restricciones */
    if (width == 0 || height == 0 ||
        width > (uint32_t)INT_MAX ||
        height > (uint32_t)INT_MAX) {
        fclose(file);
        return -1;
    }


    if ((size_t)width > SIZE_MAX / (size_t)height) {
        fclose(file);
        return -1;
    }

    pixel_count = (size_t)width * (size_t)height;

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

    /*
     * Leer todos los píxeles directamente.
     */
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
/*  Liberación                                                              */
/* ========================================================================= */

void texture_free(Texture *tex)
{
    if (!tex) {
        return;
    }

    free(tex->pixels);

    tex->pixels = NULL;
    tex->width = 0;
    tex->height = 0;
    tex->channels = 0;
}

/* ========================================================================= */
/*  Muestreo de textura                                                     */
/* ========================================================================= */

/**
 * @brief Envuelve una coordenada para mantenerla dentro de [0,1).
 */
static float wrap_coordinate(float value)
{
    value = fmodf(value, 1.0f);

    if (value < 0.0f) {
        value += 1.0f;
    }

    return value;
}

Color texture_sample(const Texture *tex, float u, float v)
{
    Color black = {0, 0, 0};

    int x;
    int y;
    size_t index;

    if (!tex || !tex->pixels ||
        tex->width <= 0 || tex->height <= 0 ||
        tex->channels < 3) {
        return black;
    }


    u = wrap_coordinate(u);
    v = wrap_coordinate(v);

    /*
     * u = 0.0 -> primera columna
     * u -> 1.0 -> última columna
     */
    x = (int)(u * (float)tex->width);

    if (x >= tex->width) {
        x = tex->width - 1;
    }


    y = (int)(v * (float)tex->height);

    if (y >= tex->height) {
        y = tex->height - 1;
    }

    y = tex->height - 1 - y;

    index = ((size_t)y * (size_t)tex->width +
             (size_t)x) * (size_t)tex->channels;

    /*
     * AVS:
     * pixels[index + 0] = A
     * pixels[index + 1] = R
     * pixels[index + 2] = G
     * pixels[index + 3] = B
     * El framebuffer solamente necesita RGB.
     */
    if (tex->channels >= 4) {
        black.r = tex->pixels[index + 1];
        black.g = tex->pixels[index + 2];
        black.b = tex->pixels[index + 3];
    } else {
        /*
         * Soporte adicional por seguridad para una textura RGB.
         */
        black.r = tex->pixels[index + 0];
        black.g = tex->pixels[index + 1];
        black.b = tex->pixels[index + 2];
    }

    return black;
}

/* ========================================================================= */
/*  Relleno texturizado                                                     */
/* ========================================================================= */

void fill_polygon_texture(Framebuffer *fb,
                          const Polygon *poly,
                          const Texture *tex,
                          const BoundingBox *bbox)
{
    float min_x;
    float max_x;
    float min_y;
    float max_y;

    int y_start;
    int y_end;

    int i;

    typedef struct {
        float y_min;
        float y_max;
        float x_at_y_min;
        float dx_dy;
    } TextureEdge;

    TextureEdge *edges;
    TextureEdge *active;

    int edge_count = 0;
    int active_count;

    if (!fb || !poly || !poly->vertices ||
        poly->count < 3 ||
        !tex || !tex->pixels ||
        tex->width <= 0 || tex->height <= 0) {
        return;
    }

    /*
     * Por seguridad, calculamos aquí el bounding box del polígono
     * que realmente estamos rasterizando (main usa universales). 
     */
    (void)bbox;

    min_x = poly->vertices[0].x;
    max_x = poly->vertices[0].x;
    min_y = poly->vertices[0].y;
    max_y = poly->vertices[0].y;

    for (i = 1; i < poly->count; i++) {
        if (poly->vertices[i].x < min_x)
            min_x = poly->vertices[i].x;

        if (poly->vertices[i].x > max_x)
            max_x = poly->vertices[i].x;

        if (poly->vertices[i].y < min_y)
            min_y = poly->vertices[i].y;

        if (poly->vertices[i].y > max_y)
            max_y = poly->vertices[i].y;
    }

    /*
     * Limitar las scanlines al framebuffer.
     */
    y_start = (int)ceilf(min_y);

    if (y_start < 0)
        y_start = 0;

    y_end = (int)floorf(max_y);

    if (y_end >= fb->height)
        y_end = fb->height - 1;

    if (y_start > y_end) {
        return;
    }

    /*
     * Una arista por cada lado del polígono.
     * Las aristas horizontales se ignoran.
     */
    edges = (TextureEdge *)malloc(
        sizeof(TextureEdge) * (size_t)poly->count
    );

    if (!edges) {
        return;
    }

    for (i = 0; i < poly->count; i++) {
        int next = (i + 1) % poly->count;

        Vertex a = poly->vertices[i];
        Vertex b = poly->vertices[next];

        if (a.y == b.y) {
            continue;
        }

        if (a.y < b.y) {
            edges[edge_count].y_min = a.y;
            edges[edge_count].y_max = b.y;
            edges[edge_count].x_at_y_min = a.x;
            edges[edge_count].dx_dy =
                (b.x - a.x) / (b.y - a.y);
        } else {
            edges[edge_count].y_min = b.y;
            edges[edge_count].y_max = a.y;
            edges[edge_count].x_at_y_min = b.x;
            edges[edge_count].dx_dy =
                (a.x - b.x) / (a.y - b.y);
        }

        edge_count++;
    }

    active = (TextureEdge *)malloc(
        sizeof(TextureEdge) * (size_t)edge_count
    );

    if (!active) {
        free(edges);
        return;
    }

    /*
     * Procesar cada scanline.
     */
    for (int y = y_start; y <= y_end; y++) {
        float scan_y = (float)y;
        int x;

        active_count = 0;

        /*
         * Buscar las aristas que intersectan la scanline.
         */
        for (i = 0; i < edge_count; i++) {
            if (scan_y >= edges[i].y_min &&
                scan_y < edges[i].y_max) {

                active[active_count] = edges[i];

                active[active_count].x_at_y_min =
                    edges[i].x_at_y_min +
                    edges[i].dx_dy *
                    (scan_y - edges[i].y_min);

                active_count++;
            }
        }

        /*
         * Ordenar intersecciones de izquierda a derecha.
         */
        for (i = 0; i < active_count - 1; i++) {
            int j;

            for (j = i + 1; j < active_count; j++) {
                if (active[j].x_at_y_min <
                    active[i].x_at_y_min) {

                    TextureEdge temp = active[i];
                    active[i] = active[j];
                    active[j] = temp;
                }
            }
        }

        /*
         * Cada par de intersecciones define un segmento interior.
         */
        for (i = 0; i + 1 < active_count; i += 2) {
            int x_left;
            int x_right;

            x_left = (int)ceilf(
                active[i].x_at_y_min
            );

            x_right = (int)floorf(
                active[i + 1].x_at_y_min
            );

            if (x_left < 0)
                x_left = 0;

            if (x_right >= fb->width)
                x_right = fb->width - 1;

            if (x_left > x_right)
                continue;

            /*
             * Pintar todos los píxeles del segmento.
             */
            for (x = x_left; x <= x_right; x++) {
                float u;
                float v;
                Color color;

                if (max_x > min_x) {
                    u = ((float)x - min_x) /
                        (max_x - min_x);
                } else {
                    u = 0.0f;
                }

                if (max_y > min_y) {
                    v = ((float)y - min_y) /
                        (max_y - min_y);
                } else {
                    v = 0.0f;
                }

                color = texture_sample(tex, u, v);

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