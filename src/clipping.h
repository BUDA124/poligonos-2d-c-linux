/**
 * @file clipping.h
 * @brief Algoritmos de recorte (clipping) 2D.
 *
 * Cohen-Sutherland para recorte de líneas y Sutherland-Hodgman para
 * recorte de polígonos contra el rectángulo de la ventana.
 *
 * Responsable de implementación: Persona 2
 * Responsable de definición:     Persona 1 (Líder)
 */
#ifndef CLIPPING_H
#define CLIPPING_H

#include "geometry.h"

/**
 * @brief Recorta una línea contra un rectángulo usando Cohen-Sutherland.
 *
 * Modifica los puntos p0 y p1 in-place para que queden dentro del
 * rectángulo definido por (xmin, ymin) - (xmax, ymax).
 *
 * @param p0    Primer extremo de la línea (se modifica in-place).
 * @param p1    Segundo extremo de la línea (se modifica in-place).
 * @param xmin  Límite izquierdo del rectángulo de recorte.
 * @param ymin  Límite inferior del rectángulo de recorte.
 * @param xmax  Límite derecho del rectángulo de recorte.
 * @param ymax  Límite superior del rectángulo de recorte.
 * @return 1 si la línea recortada es visible, 0 si fue descartada.
 */
int clip_line(Vertex *p0, Vertex *p1,
              float xmin, float ymin, float xmax, float ymax);

/**
 * @brief Recorta un polígono contra un rectángulo usando Sutherland-Hodgman.
 *
 * Retorna un nuevo polígono con los vértices resultantes del recorte.
 * El llamador debe liberar la memoria del polígono retornado
 * (result.vertices) con free().
 *
 * @param poly  Polígono a recortar.
 * @param xmin  Límite izquierdo del rectángulo de recorte.
 * @param ymin  Límite inferior del rectángulo de recorte.
 * @param xmax  Límite derecho del rectángulo de recorte.
 * @param ymax  Límite superior del rectángulo de recorte.
 * @return Nuevo polígono recortado (puede tener count == 0 si es invisible).
 */
Polygon clip_polygon(const Polygon *poly,
                     float xmin, float ymin, float xmax, float ymax);

#endif /* CLIPPING_H */

