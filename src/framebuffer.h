/**
 * @file framebuffer.h
 * @brief Framebuffer de software para renderizado por CPU.
 *
 * Gestiona un búfer lineal de píxeles RGB en RAM que se transfiere
 * a la pantalla mediante glDrawPixels().
 *
 * Responsable: Persona 1 (Líder)
 */
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "geometry.h"   /* Color */

/** Búfer de píxeles en RAM (formato RGB, 3 bytes por píxel). */
typedef struct {
    int            width;    /**< Ancho en píxeles. */
    int            height;   /**< Alto en píxeles. */
    unsigned char *pixels;   /**< Datos RGB lineales (width * height * 3). */
} Framebuffer;

/**
 * @brief Crea un framebuffer con la resolución indicada.
 *
 * Asigna memoria dinámica para width * height * 3 bytes (RGB).
 * El contenido inicial no está definido; usar framebuffer_clear()
 * para inicializarlo.
 *
 * @param width  Ancho en píxeles.
 * @param height Alto en píxeles.
 * @return Puntero al framebuffer creado, o NULL en error de memoria.
 */
Framebuffer *framebuffer_create(int width, int height);

/**
 * @brief Libera toda la memoria del framebuffer.
 * @param fb Puntero al framebuffer (puede ser NULL, en cuyo caso no hace nada).
 */
void framebuffer_destroy(Framebuffer *fb);

/**
 * @brief Limpia el framebuffer estableciendo todos los píxeles al color dado.
 * @param fb    Puntero al framebuffer.
 * @param color Color de fondo.
 */
void framebuffer_clear(Framebuffer *fb, Color color);

/**
 * @brief Enciende un píxel individual en el framebuffer.
 *
 * Incluye chequeo de límites: si (x, y) está fuera del rango válido,
 * la llamada no tiene efecto (no crash, no escritura fuera de límites).
 *
 * @param fb    Puntero al framebuffer.
 * @param x     Coordenada X (0 = izquierda).
 * @param y     Coordenada Y (0 = abajo, origen inferior izquierdo).
 * @param color Color del píxel.
 */
void framebuffer_put_pixel(Framebuffer *fb, int x, int y, Color color);

/**
 * @brief Lee el color de un píxel del framebuffer.
 *
 * Si (x, y) está fuera de rango retorna negro {0, 0, 0}.
 *
 * @param fb Puntero al framebuffer (constante).
 * @param x  Coordenada X.
 * @param y  Coordenada Y.
 * @return Color del píxel en la posición dada.
 */
Color framebuffer_get_pixel(const Framebuffer *fb, int x, int y);

/**
 * @brief Transfiere el contenido del framebuffer a la ventana GLUT.
 *
 * Ejecuta glDrawPixels() con los datos del búfer.  Debe llamarse
 * dentro del callback de display de GLUT.
 *
 * @param fb Puntero al framebuffer (constante).
 */
void framebuffer_render(const Framebuffer *fb);

#endif /* FRAMEBUFFER_H */

