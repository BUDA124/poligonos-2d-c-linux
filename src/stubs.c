/**
 * @file stubs.c
 * @brief Implementaciones provisionales (stubs) para compilación limpia.
 *
 * Contiene implementaciones mínimas de las funciones declaradas en
 * los headers de los módulos del proyecto, así como las
 * funciones auxiliares del estado de la aplicación.
 *
 * Estas funciones serán reemplazadas progresivamente por las
 * implementaciones reales de cada compañero.  Mientras tanto, permiten
 * que el proyecto compile y enlace correctamente.
 *
 * También incluye un polígono de prueba hardcodeado para verificar
 * el pipeline de renderizado.
 *
 * Responsable: Persona 1 (Líder)
 */

#include "geometry.h"
#include "app_state.h"
#include "raster.h"
#include "texture.h"
#include "framebuffer.h"
#include "clipping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>

/* ========================================================================= */
/*  Stubs — raster.h (Persona 3)                                            */
/* ========================================================================= */

void draw_line_bresenham(Framebuffer *fb,
                         int x0, int y0, int x1, int y1,
                         Color color)
{
    /*
     * Implementación simplificada de Bresenham para el arnés de prueba.
     * La Persona 3 la reemplazará con la versión optimizada para todos
     * los octantes.
     */
    int dx  = abs(x1 - x0);
    int dy  = abs(y1 - y0);
    int sx  = (x0 < x1) ? 1 : -1;
    int sy  = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int e2;

    for (;;) {
        framebuffer_put_pixel(fb, x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0  += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0  += sy;
        }
    }
}

void draw_polygon_wireframe(Framebuffer *fb, const Polygon *poly, Color color)
{
    int i;

    if (!fb || !poly || poly->count < 2) {
        return;
    }

    for (i = 0; i < poly->count; i++) {
        Vertex p0 = poly->vertices[i];
        Vertex p1 = poly->vertices[(i + 1) % poly->count];

        /* Recortar segmento contra los bordes del framebuffer */
        if (clip_line(&p0, &p1, 0.0f, 0.0f,
                      (float)(fb->width - 1), (float)(fb->height - 1))) {
            draw_line_bresenham(fb,
                                (int)p0.x, (int)p0.y,
                                (int)p1.x, (int)p1.y,
                                color);
        }
    }
}

void fill_polygon_solid(Framebuffer *fb, const Polygon *poly, Color color)
{
    (void)fb; (void)poly; (void)color;
    fprintf(stderr, "[STUB] fill_polygon_solid: no implementado aun.\n");
}

/* ========================================================================= */
/*  Stubs — texture.h (Persona 4)                                           */
/* ========================================================================= */

int texture_load_avs(const char *filepath, Texture *out)
{
    (void)filepath; (void)out;
    fprintf(stderr, "[STUB] texture_load_avs: no implementado aun.\n");
    return -1;
}

void texture_free(Texture *tex)
{
    if (tex && tex->pixels) {
        free(tex->pixels);
        tex->pixels = NULL;
        tex->width  = 0;
        tex->height = 0;
    }
}

Color texture_sample(const Texture *tex, float u, float v)
{
    Color black = {0, 0, 0};
    (void)tex; (void)u; (void)v;
    return black;
}

void fill_polygon_texture(Framebuffer *fb, const Polygon *poly,
                          const Texture *tex, const BoundingBox *bbox)
{
    (void)fb; (void)poly; (void)tex; (void)bbox;
    fprintf(stderr, "[STUB] fill_polygon_texture: no implementado aun.\n");
}

/* ========================================================================= */
/*  app_state.h — Implementación                                            */
/* ========================================================================= */

/* Nombres, archivos y colores por defecto para las 7 provincias de Costa Rica */
static const char *const PROVINCE_NAMES[NUM_PROVINCES] = {
    "San Jose", "Alajuela", "Cartago", "Heredia", "Guanacaste", "Puntarenas", "Limon"
};

static const char *const PROVINCE_FILES[NUM_PROVINCES] = {
    "data/provincias/san_jose.txt",
    "data/provincias/alajuela.txt",
    "data/provincias/cartago.txt",
    "data/provincias/heredia.txt",
    "data/provincias/guanacaste.txt",
    "data/provincias/puntarenas.txt",
    "data/provincias/limon.txt"
};

static const char *const TEXTURE_FILES[NUM_PROVINCES] = {
    "data/texturas/san_jose.avs",
    "data/texturas/alajuela.avs",
    "data/texturas/cartago.avs",
    "data/texturas/heredia.avs",
    "data/texturas/guanacaste.avs",
    "data/texturas/puntarenas.avs",
    "data/texturas/limon.avs"
};

/* 7 colores bien diferenciados para modo sólido */
static const Color PROVINCE_COLORS[NUM_PROVINCES] = {
    {65, 105, 225},  /* San Jose: Azul real */
    {220, 20, 60},   /* Alajuela: Rojo carmesí */
    {34, 139, 34},   /* Cartago: Verde bosque */
    {255, 215, 0},   /* Heredia: Oro / Amarillo */
    {255, 140, 0},   /* Guanacaste: Naranja oscuro */
    {148, 0, 211},   /* Puntarenas: Violeta */
    {0, 206, 209}    /* Limon: Turquesa */
};

void app_state_init(AppState *state)
{
    memset(state, 0, sizeof(AppState));

    state->view.center_x = 0.0f;
    state->view.center_y = 0.0f;
    state->view.zoom     = 1.0f;
    state->view.angle    = 0.0f;
    state->view.pan_x    = 0.0f;
    state->view.pan_y    = 0.0f;

    state->default_view  = state->view;
    state->mode          = MODE_WIREFRAME;
}

void app_state_load_data(AppState *state)
{
    int i;

    if (!state) return;

    for (i = 0; i < NUM_PROVINCES; i++) {
        strncpy(state->provinces[i].name, PROVINCE_NAMES[i],
                sizeof(state->provinces[i].name) - 1);
        strncpy(state->provinces[i].texture_path, TEXTURE_FILES[i],
                sizeof(state->provinces[i].texture_path) - 1);
        state->provinces[i].color = PROVINCE_COLORS[i];

        /* Intenta cargar geometría (Persona 2) */
        if (province_load(PROVINCE_FILES[i], &state->provinces[i]) == 0) {
            province_compute_bbox(&state->provinces[i]);
            state->provinces_loaded++;
        }

        /* Intenta cargar textura .avs (Persona 4) */
        if (texture_load_avs(TEXTURE_FILES[i], &state->textures[i]) == 0) {
            state->textures_loaded++;
        }
    }

    if (state->provinces_loaded > 0) {
        float map_w, map_h, scale;

        state->global_bbox = provinces_global_bbox(state->provinces,
                                                   NUM_PROVINCES);
        /* Centrar vista en el mapa cargado */
        state->view.center_x = (state->global_bbox.min_x + state->global_bbox.max_x) * 0.5f;
        state->view.center_y = (state->global_bbox.min_y + state->global_bbox.max_y) * 0.5f;
        map_w = state->global_bbox.max_x - state->global_bbox.min_x;
        map_h = state->global_bbox.max_y - state->global_bbox.min_y;

        if (map_w > 0.0f && map_h > 0.0f) {
            float scale_x = (float)WINDOW_WIDTH  * 0.95f / map_w;
            float scale_y = (float)WINDOW_HEIGHT * 0.95f / map_h;
            scale = (scale_x < scale_y) ? scale_x : scale_y;
            state->view.zoom = scale;
        }

        state->default_view  = state->view;
    }
}

void app_state_cleanup(AppState *state)
{
    int i;

    if (!state) return;

    for (i = 0; i < NUM_PROVINCES; i++) {
        province_free(&state->provinces[i]);
        texture_free(&state->textures[i]);
    }

    state->provinces_loaded = 0;
    state->textures_loaded  = 0;
}

void app_state_reset_view(AppState *state)
{
    if (state) {
        state->view = state->default_view;
    }
}

float app_state_get_speed(void)
{
    int mods = glutGetModifiers();

    if (mods & GLUT_ACTIVE_SHIFT) {
        return SPEED_FAST;
    }
    if (mods & GLUT_ACTIVE_CTRL) {
        return SPEED_SLOW;
    }
    return SPEED_NORMAL;
}

/* ========================================================================= */
/*  Datos de Prueba — Polígono Hardcodeado                                   */
/* ========================================================================= */

/**
 * @brief Carga un polígono de prueba (estrella de 5 puntas) para validar
 *        el pipeline de renderizado.
 *
 * Los vértices están en coordenadas de pantalla directas (píxeles).
 * Esta función se usa desde main.c cuando no hay provincias cargadas.
 */
static Vertex test_star_vertices[] = {
    /* Puntas externas de la estrella */
    {512.0f, 700.0f},   /* Punta superior */
    {440.0f, 530.0f},
    {300.0f, 530.0f},   /* Punta izquierda */
    {410.0f, 420.0f},
    {360.0f, 260.0f},   /* Punta inferior izquierda */
    {512.0f, 350.0f},
    {664.0f, 260.0f},   /* Punta inferior derecha */
    {614.0f, 420.0f},
    {724.0f, 530.0f},   /* Punta derecha */
    {584.0f, 530.0f}
};

static Polygon test_star_polygon = {
    test_star_vertices,
    10  /* sizeof(test_star_vertices) / sizeof(Vertex) */
};

/** Polígono de prueba accesible desde main.c. */
Polygon *get_test_polygon(void)
{
    return &test_star_polygon;
}
