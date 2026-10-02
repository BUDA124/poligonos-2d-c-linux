#include "transform.h"

#include <math.h>
#include <stdlib.h>

Vertex transform_vertex(Vertex v, const View *view, int screen_w, int screen_h)
{
    Vertex out;
    float  dx, dy;
    float  rx, ry;
    float  cos_a, sin_a;
    float  half_w, half_h;

    dx = v.x - view->center_x;
    dy = v.y - view->center_y;

    cos_a = cosf(view->angle);
    sin_a = sinf(view->angle);
    rx = dx * cos_a - dy * sin_a;
    ry = dx * sin_a + dy * cos_a;

    rx *= view->zoom;
    ry *= view->zoom;

    rx += view->pan_x;
    ry += view->pan_y;

    half_w = (float)screen_w * 0.5f;
    half_h = (float)screen_h * 0.5f;

    out.x = half_w + rx;
    out.y = half_h + ry;

    return out;
}

Polygon transform_polygon(const Polygon *poly, const View *view,
                           int screen_w, int screen_h)
{
    Polygon out;
    int i;

    out.count    = poly->count;
    out.vertices = (Vertex *)malloc(sizeof(Vertex) * (size_t)out.count);
    if (!out.vertices) {
        out.count = 0;
        return out;
    }

    for (i = 0; i < out.count; i++) {
        out.vertices[i] = transform_vertex(poly->vertices[i], view,
                                           screen_w, screen_h);
    }

    return out;
}
