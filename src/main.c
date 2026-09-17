/**
 * @file main.c
 * @brief Punto de entrada y bucle principal GLUT.
 *
 * Inicializa la ventana gráfica con FreeGLUT, configura la proyección
 * ortográfica 1:1, registra callbacks de teclado y redibujado, y
 * orquesta el pipeline de renderizado del framebuffer de software.
 *
 * Responsable: Persona 1 (Líder)
 */

#include "framebuffer.h"
#include "app_state.h"
#include "geometry.h"
#include "transform.h"
#include "clipping.h"
#include "raster.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

/* ========================================================================= */
/*  Variables Globales                                                        */
/* ========================================================================= */

/** Framebuffer principal (búfer de píxeles en RAM). */
static Framebuffer *g_fb = NULL;

/** Estado global de la aplicación. */
static AppState g_state;

/* Declarado en stubs.c — polígono de prueba. */
extern Polygon *get_test_polygon(void);

/* ========================================================================= */
/*  Renderizado                                                              */
/* ========================================================================= */

/**
 * @brief Dibuja la escena completa en el framebuffer.
 *
 * Limpia el búfer, dibuja las provincias según el modo activo y
 * finalmente dibuja el polígono de prueba si no hay datos reales.
 */
static void render_scene(void)
{
    Color bg     = {20, 20, 30};       /* Fondo oscuro (casi negro). */
    Color white  = {255, 255, 255};
    Color green  = {0, 200, 80};
    Polygon *test_poly;
    int i;

    framebuffer_clear(g_fb, bg);

    /* --- Dibujar provincias cargadas ------------------------------------ */
    if (g_state.provinces_loaded > 0) {
        for (i = 0; i < g_state.provinces_loaded; i++) {
            Province *p = &g_state.provinces[i];
            int j;

            for (j = 0; j < p->polygon_count; j++) {
                /* Transformar a coordenadas de pantalla. */
                Polygon transformed = transform_polygon(
                    &p->polygons[j], &g_state.view,
                    g_fb->width, g_fb->height
                );

                switch (g_state.mode) {
                case MODE_WIREFRAME:
                    /* draw_polygon_wireframe aplica clipping de línea internamente */
                    draw_polygon_wireframe(g_fb, &transformed, p->color);
                    break;

                case MODE_SOLID: {
                    /* Clipping de polígono contra los bordes de la ventana */
                    Polygon clipped = clip_polygon(&transformed, 0.0f, 0.0f,
                                                   (float)(g_fb->width - 1),
                                                   (float)(g_fb->height - 1));
                    if (clipped.count >= 3) {
                        fill_polygon_solid(g_fb, &clipped, p->color);
                        draw_polygon_wireframe(g_fb, &clipped, white);
                    }
                    polygon_free(&clipped);
                    break;
                }

                case MODE_TEXTURE: {
                    /* Clipping de polígono y mapeo de textura .avs */
                    Polygon clipped = clip_polygon(&transformed, 0.0f, 0.0f,
                                                   (float)(g_fb->width - 1),
                                                   (float)(g_fb->height - 1));
                    if (clipped.count >= 3) {
                        fill_polygon_texture(g_fb, &clipped, &g_state.textures[i], &p->bbox);
                        draw_polygon_wireframe(g_fb, &clipped, white);
                    }
                    polygon_free(&clipped);
                    break;
                }
                }

                polygon_free(&transformed);
            }
        }
    } else {
        /* --- Polígono de prueba (estrella) ------------------------------ */
        test_poly = get_test_polygon();
        if (test_poly) {
            switch (g_state.mode) {
            case MODE_WIREFRAME:
                draw_polygon_wireframe(g_fb, test_poly, green);
                break;
            case MODE_SOLID: {
                Polygon clipped = clip_polygon(test_poly, 0.0f, 0.0f,
                                               (float)(g_fb->width - 1),
                                               (float)(g_fb->height - 1));
                if (clipped.count >= 3) {
                    fill_polygon_solid(g_fb, &clipped, green);
                    draw_polygon_wireframe(g_fb, &clipped, white);
                }
                polygon_free(&clipped);
                break;
            }
            case MODE_TEXTURE: {
                Polygon clipped = clip_polygon(test_poly, 0.0f, 0.0f,
                                               (float)(g_fb->width - 1),
                                               (float)(g_fb->height - 1));
                if (clipped.count >= 3) {
                    fill_polygon_texture(g_fb, &clipped, &g_state.textures[0], NULL);
                    draw_polygon_wireframe(g_fb, &clipped, white);
                }
                polygon_free(&clipped);
                break;
            }
            }
        }
    }
}

/* ========================================================================= */
/*  Callbacks de GLUT                                                        */
/* ========================================================================= */

/**
 * @brief Callback de display: renderiza la escena y la transfiere a pantalla.
 */
static void display_callback(void)
{
    render_scene();
    framebuffer_render(g_fb);
    glutSwapBuffers();
}

/**
 * @brief Callback de redimensionado de ventana.
 *
 * En esta versión se mantiene resolución fija; este callback ajusta
 * el viewport y la proyección ortográfica para reflejar el tamaño
 * actual de la ventana sin redimensionar el framebuffer.
 */
static void reshape_callback(int width, int height)
{
    if (height == 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)g_fb->width, 0.0, (double)g_fb->height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
 * @brief Callback de teclado ASCII.
 *
 * Controla cambio de modo (1/2/3), reset (0) y salida (q/ESC).
 */
static void keyboard_callback(unsigned char key, int x, int y)
{
    (void)x; (void)y;

    switch (key) {
    /* Cambio de modo de renderizado. */
    case '1':
        g_state.mode = MODE_WIREFRAME;
        printf("Modo: Wireframe (bordes)\n");
        break;
    case '2':
        g_state.mode = MODE_SOLID;
        printf("Modo: Solido (relleno)\n");
        break;
    case '3':
        g_state.mode = MODE_TEXTURE;
        printf("Modo: Textura\n");
        break;

    /* Reset de vista. */
    case '0':
        app_state_reset_view(&g_state);
        printf("Vista restaurada a valores por defecto.\n");
        break;

    /* Zoom con + y - */
    case '+':
    case '=':
        g_state.view.zoom += ZOOM_STEP;
        if (g_state.view.zoom > 10.0f) g_state.view.zoom = 10.0f;
        printf("Zoom: %.2f\n", g_state.view.zoom);
        break;
    case '-':
    case '_':
        g_state.view.zoom -= ZOOM_STEP;
        if (g_state.view.zoom < 0.1f) g_state.view.zoom = 0.1f;
        printf("Zoom: %.2f\n", g_state.view.zoom);
        break;

    /* Rotación con r/R. */
    case 'r':
        g_state.view.angle += ROTATE_STEP;
        printf("Rotacion: %.2f rad\n", g_state.view.angle);
        break;
    case 'R':
        g_state.view.angle -= ROTATE_STEP;
        printf("Rotacion: %.2f rad\n", g_state.view.angle);
        break;

    /* Salida limpia. */
    case 'q':
    case 'Q':
    case 27:   /* ESC */
        printf("Saliendo... Liberando memoria.\n");
        app_state_cleanup(&g_state);
        framebuffer_destroy(g_fb);
        g_fb = NULL;
        exit(0);
        break;

    default:
        break;
    }

    glutPostRedisplay();
}

/**
 * @brief Callback de teclas especiales (flechas).
 *
 * Controla el paneo del mapa con las teclas de dirección.
 * La velocidad varía según SHIFT (rápido) o CTRL (lento).
 */
static void special_callback(int key, int x, int y)
{
    float speed;

    (void)x; (void)y;

    speed = app_state_get_speed();

    switch (key) {
    case GLUT_KEY_UP:
        g_state.view.pan_y += speed;
        break;
    case GLUT_KEY_DOWN:
        g_state.view.pan_y -= speed;
        break;
    case GLUT_KEY_LEFT:
        g_state.view.pan_x -= speed;
        break;
    case GLUT_KEY_RIGHT:
        g_state.view.pan_x += speed;
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

/* ========================================================================= */
/*  Punto de Entrada                                                         */
/* ========================================================================= */

int main(int argc, char **argv)
{
    /* Inicializar estado de la aplicación. */
    app_state_init(&g_state);

    /* --- Inicialización de GLUT ----------------------------------------- */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Proyecto 1 - Mapa de Costa Rica (CG)");

    /* --- Crear framebuffer ---------------------------------------------- */
    g_fb = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g_fb) {
        fprintf(stderr, "Error: no se pudo crear el framebuffer.\n");
        return EXIT_FAILURE;
    }

    /* --- Configurar proyección ortográfica ------------------------------ */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)WINDOW_WIDTH, 0.0, (double)WINDOW_HEIGHT, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Fondo negro de OpenGL (el real se maneja en el framebuffer). */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* --- Cargar datos de provincias y texturas --------------------------- */
    app_state_load_data(&g_state);
    if (g_state.provinces_loaded > 0) {
        printf("[INFO] Se cargaron %d/%d provincias.\n",
               g_state.provinces_loaded, NUM_PROVINCES);
    } else {
        printf("[INFO] Modo de prueba activo (estrella). Coloque los datos en data/provincias/\n");
    }

    /* --- Registrar callbacks -------------------------------------------- */
    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutKeyboardFunc(keyboard_callback);
    glutSpecialFunc(special_callback);

    /* --- Información al usuario ----------------------------------------- */
    printf("===========================================\n");
    printf("  Proyecto 1 - Mapa de Costa Rica\n");
    printf("  Computacion Grafica\n");
    printf("===========================================\n");
    printf("  Controles:\n");
    printf("    1/2/3     - Modo wireframe/solido/textura\n");
    printf("    Flechas   - Mover (pan)\n");
    printf("    +/-       - Zoom in/out\n");
    printf("    r/R       - Rotar CW/CCW\n");
    printf("    0         - Restablecer vista\n");
    printf("    q / ESC   - Salir\n");
    printf("    SHIFT     - Velocidad rapida\n");
    printf("    CTRL      - Velocidad lenta\n");
    printf("===========================================\n");

    /* --- Entrar al bucle principal -------------------------------------- */
    glutMainLoop();

    /* glutMainLoop() nunca retorna, pero por buenas prácticas: */
    app_state_cleanup(&g_state);
    framebuffer_destroy(g_fb);
    return EXIT_SUCCESS;
}

