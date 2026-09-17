/**
 * @file geometry.h
 * @brief Tipos geométricos compartidos y funciones de carga de datos.
 *
 * Este archivo define las estructuras de datos fundamentales del proyecto
 * (Color, Vertex, Polygon, Province, BoundingBox) y los prototipos para
 * la carga y liberación de la información geográfica de las provincias.
 *
 * Responsable de implementación: Persona 2
 * Responsable de definición:     Persona 1 (Líder)
 */
#ifndef GEOMETRY_H
#define GEOMETRY_H

/* ========================================================================= */
/*  Tipos Básicos                                                            */
/* ========================================================================= */

/** Color en formato RGB (8 bits por canal). */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

/** Vértice 2D en coordenadas de mundo (punto flotante). */
typedef struct {
    float x;
    float y;
} Vertex;

/** Polígono definido como lista de vértices. */
typedef struct {
    Vertex *vertices;   /**< Arreglo dinámico de vértices. */
    int     count;      /**< Cantidad de vértices en el arreglo. */
} Polygon;

/** Caja envolvente alineada a ejes (AABB). */
typedef struct {
    float min_x;
    float min_y;
    float max_x;
    float max_y;
} BoundingBox;

/** Provincia: colección de polígonos con metadatos de color y textura. */
typedef struct {
    char     name[64];          /**< Nombre de la provincia (UTF-8). */
    Polygon *polygons;          /**< Arreglo dinámico de polígonos. */
    int      polygon_count;     /**< Cantidad de polígonos. */
    Color    color;             /**< Color sólido para modo MODE_SOLID. */
    char     texture_path[256]; /**< Ruta al archivo .avs de textura. */
    BoundingBox bbox;           /**< Caja envolvente calculada. */
} Province;

/* ========================================================================= */
/*  Prototipos — Persona 2                                                   */
/* ========================================================================= */

/**
 * @brief Carga los datos de una provincia desde un archivo de texto.
 *
 * El archivo debe contener los polígonos de la provincia en el formato
 * acordado por el equipo.  La función asigna memoria dinámica que debe
 * liberarse con province_free().
 *
 * @param filepath Ruta al archivo de la provincia.
 * @param out      Puntero a la estructura Province a poblar.
 * @return  0 en éxito, -1 en error (el llamador debe verificar).
 */
int province_load(const char *filepath, Province *out);

/**
 * @brief Libera la memoria dinámica de un polígono individual.
 * @param poly Puntero al polígono cuyos vértices se liberarán.
 */
void polygon_free(Polygon *poly);

/**
 * @brief Libera toda la memoria dinámica asociada a una provincia.
 * @param prov Puntero a la provincia a liberar.
 */
void province_free(Province *prov);

/**
 * @brief Calcula la caja envolvente de una provincia.
 *
 * Recorre todos los polígonos y vértices de la provincia y almacena
 * los extremos en prov->bbox.
 *
 * @param prov Puntero a la provincia (debe tener polígonos cargados).
 */
void province_compute_bbox(Province *prov);

/**
 * @brief Calcula la caja envolvente global de un arreglo de provincias.
 * @param provinces Arreglo de provincias.
 * @param count     Cantidad de provincias.
 * @return BoundingBox que contiene a todas las provincias.
 */
BoundingBox provinces_global_bbox(const Province *provinces, int count);

#endif /* GEOMETRY_H */

