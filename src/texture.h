/**
 * @file texture.h
 * @brief Carga de texturas .avs y relleno de polígonos con textura.
 *
 * Prototipos para lectura de archivos de textura en formato .avs,
 * muestreo de píxeles y relleno texturizado de polígonos.
 *
 * Responsable de implementación: Persona 4
 * Responsable de definición:     Persona 1 (Líder)
 */
#ifndef TEXTURE_H
#define TEXTURE_H

#include "geometry.h"
#include "framebuffer.h"

/** Textura cargada en memoria desde un archivo .avs. */
typedef struct {
    int            width;      /**< Ancho de la textura en píxeles. */
    int            height;     /**< Alto de la textura en píxeles. */
    int            channels;   /**< Cantidad de canales (3 para RGB, 4 para RGBA). */
    unsigned char *pixels;     /**< Datos de píxeles lineales. */
} Texture;

/**
 * @brief Carga una textura desde un archivo en formato .avs.
 *
 * Asigna memoria dinámica para los píxeles.  Debe liberarse con
 * texture_free().
 *
 * @param filepath  Ruta al archivo .avs.
 * @param out       Puntero a la estructura Texture a poblar.
 * @return 0 en éxito, -1 en error.
 */
int texture_load_avs(const char *filepath, Texture *out);

/**
 * @brief Libera la memoria asociada a una textura.
 * @param tex  Puntero a la textura a liberar.
 */
void texture_free(Texture *tex);

/**
 * @brief Muestrea un color de la textura en coordenadas normalizadas [0,1].
 *
 * Aplica wrapping (repetición) si las coordenadas exceden [0,1].
 *
 * @param tex  Puntero a la textura cargada.
 * @param u    Coordenada U normalizada [0.0, 1.0].
 * @param v    Coordenada V normalizada [0.0, 1.0].
 * @return Color del texel correspondiente.
 */
Color texture_sample(const Texture *tex, float u, float v);

/**
 * @brief Rellena un polígono con textura usando scanline modificado.
 *
 * Combina el algoritmo de scanline con interpolación de coordenadas UV
 * para mapear la textura sobre el polígono.
 *
 * @param fb    Puntero al framebuffer destino.
 * @param poly  Polígono con vértices en coordenadas de pantalla.
 * @param tex   Textura a aplicar.
 * @param bbox  Bounding box del polígono en coordenadas de mundo
 *              (para calcular UVs).
 */
void fill_polygon_texture(Framebuffer *fb, const Polygon *poly,
                          const Texture *tex, const BoundingBox *bbox);

#endif /* TEXTURE_H */

