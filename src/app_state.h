/**
 * @file app_state.h
 * @brief Estado global de la aplicación y modos de visualización.
 *
 * Define las constantes de configuración, los modos de renderizado y la
 * estructura central AppState que encapsula todo el estado mutable del
 * programa (vista, provincias cargadas, modo activo, etc.).
 *
 * Responsable: Persona 1 (Líder)
 */
#ifndef APP_STATE_H
#define APP_STATE_H

#include "geometry.h"
#include "texture.h"

/* ========================================================================= */
/*  Constantes de Configuración                                              */
/* ========================================================================= */

/** Resolución inicial de la ventana (píxeles). */
#define WINDOW_WIDTH   1024
#define WINDOW_HEIGHT   768

/** Cantidad de provincias de Costa Rica. */
#define NUM_PROVINCES   7

/** Velocidades de transformación (píxeles o unidades por evento). */
#define SPEED_NORMAL   5.0f
#define SPEED_FAST    15.0f    /* Con SHIFT */
#define SPEED_SLOW     1.0f    /* Con CTRL  */

/** Factor de zoom por paso. */
#define ZOOM_STEP      0.1f

/** Ángulo de rotación por paso (en radianes, ~5°). */
#define ROTATE_STEP    0.0872665f

/* ========================================================================= */
/*  Modos de Visualización                                                   */
/* ========================================================================= */

/** Modos de renderizado del mapa. */
typedef enum {
    MODE_WIREFRAME = 1,   /**< Solo bordes (Bresenham). */
    MODE_SOLID     = 2,   /**< Relleno sólido (Scanline). */
    MODE_TEXTURE   = 3    /**< Textura mapeada (.avs). */
} RenderMode;

/* ========================================================================= */
/*  Estado de la Vista (Cámara 2D)                                           */
/* ========================================================================= */

/** Parámetros de la cámara / vista del mapa. */
typedef struct {
    float center_x;   /**< Centro de rotación X (coordenadas de mundo). */
    float center_y;   /**< Centro de rotación Y (coordenadas de mundo). */
    float zoom;       /**< Factor de escala (> 0.0). */
    float angle;      /**< Ángulo de rotación (radianes). */
    float pan_x;      /**< Desplazamiento horizontal acumulado. */
    float pan_y;      /**< Desplazamiento vertical acumulado. */
} View;

/* ========================================================================= */
/*  Estado Global de la Aplicación                                           */
/* ========================================================================= */

/** Estructura central que encapsula todo el estado mutable del programa. */
typedef struct {
    Province    provinces[NUM_PROVINCES]; /**< Datos de las 7 provincias. */
    Texture     textures[NUM_PROVINCES];  /**< Texturas de las 7 provincias. */
    int         provinces_loaded;         /**< Cuántas se cargaron con éxito. */
    int         textures_loaded;          /**< Cuántas texturas cargadas. */
    View        view;                     /**< Estado actual de la cámara. */
    View        default_view;             /**< Vista por defecto (para reset). */
    RenderMode  mode;                     /**< Modo de renderizado activo. */
    BoundingBox global_bbox;              /**< Caja envolvente de todo el mapa. */
} AppState;

/* ========================================================================= */
/*  Funciones de Utilidad del Estado                                         */
/* ========================================================================= */

/**
 * @brief Inicializa el AppState con valores por defecto.
 * @param state Puntero al AppState a inicializar.
 */
void app_state_init(AppState *state);

/**
 * @brief Carga las 7 provincias y sus texturas asociadas.
 *
 * Configura nombres, colores diferenciados y rutas de archivos por provincia,
 * intentando cargar los datos con province_load() y texture_load_avs().
 * Si los archivos no existen aún, mantiene provinces_loaded en 0.
 *
 * @param state Puntero al AppState.
 */
void app_state_load_data(AppState *state);

/**
 * @brief Libera toda la memoria de provincias y texturas cargadas.
 * @param state Puntero al AppState.
 */
void app_state_cleanup(AppState *state);

/**
 * @brief Reinicia la vista a los valores por defecto (tecla 0).
 * @param state Puntero al AppState.
 */
void app_state_reset_view(AppState *state);

/**
 * @brief Calcula la velocidad según los modificadores de teclado activos.
 *
 * Consulta glutGetModifiers() para determinar si SHIFT o CTRL están
 * presionados y retorna la velocidad correspondiente.
 *
 * @return Velocidad a aplicar (SPEED_NORMAL, SPEED_FAST o SPEED_SLOW).
 */
float app_state_get_speed(void);

#endif /* APP_STATE_H */

