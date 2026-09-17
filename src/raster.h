/**
 * @file raster.h
 * @brief Algoritmos de rasterización: Bresenham y Scanline fill.
 *
 * Prototipos para dibujo de líneas (Bresenham) y relleno sólido de
 * polígonos (Scanline) directamente sobre el Framebuffer.
 *
 * Responsable de implementación: Persona 3
 * Responsable de definición:     Persona 1 (Líder)
 */
#ifndef RASTER_H
#define RASTER_H

#include "geometry.h"
#include "framebuffer.h"

/**
 * @brief Dibuja una línea entre dos puntos usando el algoritmo de Bresenham.
 *
 * Enciende los píxeles entre (x0, y0) y (x1, y1) en el framebuffer
 * con el color indicado.  Funciona correctamente en todos los octantes.
 *
 * @param fb     Puntero al framebuffer destino.
 * @param x0     Coordenada X del punto inicial (píxeles de pantalla).
 * @param y0     Coordenada Y del punto inicial.
 * @param x1     Coordenada X del punto final.
 * @param y1     Coordenada Y del punto final.
 * @param color  Color de la línea.
 */
void draw_line_bresenham(Framebuffer *fb,
                         int x0, int y0, int x1, int y1,
                         Color color);

/**
 * @brief Dibuja el contorno (wireframe) de un polígono usando Bresenham.
 *
 * Dibuja una línea entre cada par de vértices consecutivos y cierra
 * el polígono conectando el último vértice con el primero.
 *
 * @param fb     Puntero al framebuffer destino.
 * @param poly   Polígono con vértices en coordenadas de pantalla (int).
 * @param color  Color del contorno.
 */
void draw_polygon_wireframe(Framebuffer *fb, const Polygon *poly, Color color);

/**
 * @brief Rellena un polígono con color sólido usando Scanline fill.
 *
 * Implementa el algoritmo de scanline con tabla de aristas activas
 * para rellenar el interior del polígono.
 *
 * @param fb     Puntero al framebuffer destino.
 * @param poly   Polígono con vértices en coordenadas de pantalla (int).
 * @param color  Color de relleno.
 */
void fill_polygon_solid(Framebuffer *fb, const Polygon *poly, Color color);

#endif /* RASTER_H */

