/**
 * @file transform.h
 * @brief Transformaciones 2D: zoom, paneo y rotación.
 *
 * Prototipos para transformar vértices de coordenadas de mundo a
 * coordenadas de pantalla, aplicando la vista actual (zoom, pan, rotación).
 *
 * Responsable de implementación: Persona 2
 * Responsable de definición:     Persona 1 (Líder)
 */
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "geometry.h"
#include "app_state.h"

/**
 * @brief Transforma un vértice de coordenadas de mundo a coordenadas de
 *        pantalla, aplicando rotación, zoom y paneo según la vista actual.
 *
 * Pipeline de transformación:
 *   1. Trasladar al origen de rotación (center_x, center_y).
 *   2. Rotar por view->angle.
 *   3. Escalar por view->zoom.
 *   4. Aplicar paneo (pan_x, pan_y).
 *   5. Mapear a coordenadas de pantalla (0..screen_w, 0..screen_h).
 *
 * @param v         Vértice en coordenadas de mundo.
 * @param view      Estado actual de la vista.
 * @param screen_w  Ancho de la ventana en píxeles.
 * @param screen_h  Alto de la ventana en píxeles.
 * @return Vértice en coordenadas de pantalla.
 */
Vertex transform_vertex(Vertex v, const View *view, int screen_w, int screen_h);

/**
 * @brief Transforma todos los vértices de un polígono y retorna una copia
 *        con las coordenadas de pantalla.
 *
 * La memoria del polígono retornado debe liberarse con free().
 *
 * @param poly      Polígono original en coordenadas de mundo.
 * @param view      Estado actual de la vista.
 * @param screen_w  Ancho de la ventana en píxeles.
 * @param screen_h  Alto de la ventana en píxeles.
 * @return Polígono nuevo con vértices transformados (caller debe liberar
 *         result.vertices).
 */
Polygon transform_polygon(const Polygon *poly, const View *view,
                           int screen_w, int screen_h);

#endif /* TRANSFORM_H */

