/**
 * @file framebuffer.c
 * @brief Implementación del framebuffer de software.
 *
 * Todas las operaciones de escritura/lectura de píxeles se realizan
 * sobre un búfer lineal en RAM.  La única llamada OpenGL es
 * glDrawPixels() en framebuffer_render().
 *
 * Responsable: Persona 1 (Líder)
 */

#include "framebuffer.h"

#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

/* ========================================================================= */
/*  Creación y Destrucción                                                   */
/* ========================================================================= */

Framebuffer *framebuffer_create(int width, int height)
{
    Framebuffer *fb;

    if (width <= 0 || height <= 0) {
        return NULL;
    }

    fb = (Framebuffer *)malloc(sizeof(Framebuffer));
    if (!fb) {
        return NULL;
    }

    fb->width  = width;
    fb->height = height;
    fb->pixels = (unsigned char *)calloc((size_t)width * (size_t)height * 3,
                                         sizeof(unsigned char));
    if (!fb->pixels) {
        free(fb);
        return NULL;
    }

    return fb;
}

void framebuffer_destroy(Framebuffer *fb)
{
    if (fb) {
        free(fb->pixels);
        fb->pixels = NULL;
        free(fb);
    }
}

/* ========================================================================= */
/*  Limpieza                                                                 */
/* ========================================================================= */

void framebuffer_clear(Framebuffer *fb, Color color)
{
    int total;
    int i;

    if (!fb || !fb->pixels) {
        return;
    }

    total = fb->width * fb->height;

    /* Caso especial: negro puro — memset es más rápido. */
    if (color.r == 0 && color.g == 0 && color.b == 0) {
        memset(fb->pixels, 0, (size_t)total * 3);
        return;
    }

    for (i = 0; i < total; i++) {
        fb->pixels[i * 3 + 0] = color.r;
        fb->pixels[i * 3 + 1] = color.g;
        fb->pixels[i * 3 + 2] = color.b;
    }
}

/* ========================================================================= */
/*  Operaciones de Píxel                                                     */
/* ========================================================================= */

void framebuffer_put_pixel(Framebuffer *fb, int x, int y, Color color)
{
    int offset;

    if (!fb || !fb->pixels) {
        return;
    }

    /* Clipping a nivel de búfer: descartar silenciosamente. */
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        return;
    }

    /*
     * Origen inferior izquierdo: y=0 corresponde a la primera fila
     * del búfer lineal (la fila inferior en pantalla), que es también
     * la primera fila que glDrawPixels() dibuja.
     */
    offset = (y * fb->width + x) * 3;
    fb->pixels[offset + 0] = color.r;
    fb->pixels[offset + 1] = color.g;
    fb->pixels[offset + 2] = color.b;
}

Color framebuffer_get_pixel(const Framebuffer *fb, int x, int y)
{
    Color black = {0, 0, 0};
    int   offset;

    if (!fb || !fb->pixels) {
        return black;
    }
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        return black;
    }

    offset = (y * fb->width + x) * 3;
    black.r = fb->pixels[offset + 0];
    black.g = fb->pixels[offset + 1];
    black.b = fb->pixels[offset + 2];
    return black;
}

/* ========================================================================= */
/*  Transferencia a Pantalla                                                 */
/* ========================================================================= */

void framebuffer_render(const Framebuffer *fb)
{
    if (!fb || !fb->pixels) {
        return;
    }

    /*
     * glRasterPos2i(0, 0)  — posición inferior izquierda (ya configurada
     * por la proyección ortográfica en main.c).
     *
     * glDrawPixels transfiere el búfer de RAM a la ventana.
     * GL_RGB + GL_UNSIGNED_BYTE: 3 bytes por píxel sin canal alfa.
     */
    glRasterPos2i(0, 0);
    glDrawPixels(fb->width, fb->height, GL_RGB, GL_UNSIGNED_BYTE, fb->pixels);
}

