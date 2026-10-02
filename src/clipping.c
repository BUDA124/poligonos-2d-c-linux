#include "clipping.h"

#include <stdlib.h>
#include <string.h>

#define CS_INSIDE  0
#define CS_LEFT    1
#define CS_RIGHT   2
#define CS_BOTTOM  4
#define CS_TOP     8

static int compute_outcode(float x, float y,
                           float xmin, float ymin, float xmax, float ymax)
{
    int code = CS_INSIDE;

    if (x < xmin)      code |= CS_LEFT;
    else if (x > xmax) code |= CS_RIGHT;

    if (y < ymin)      code |= CS_BOTTOM;
    else if (y > ymax) code |= CS_TOP;

    return code;
}

int clip_line(Vertex *p0, Vertex *p1,
              float xmin, float ymin, float xmax, float ymax)
{
    float x0, y0, x1, y1;
    int   code0, code1, code_out;
    float x, y;

    if (!p0 || !p1) {
        return 0;
    }

    x0 = p0->x;  y0 = p0->y;
    x1 = p1->x;  y1 = p1->y;

    code0 = compute_outcode(x0, y0, xmin, ymin, xmax, ymax);
    code1 = compute_outcode(x1, y1, xmin, ymin, xmax, ymax);

    for (;;) {
        if ((code0 | code1) == 0) {
            p0->x = x0;  p0->y = y0;
            p1->x = x1;  p1->y = y1;
            return 1;
        }

        if ((code0 & code1) != 0) {
            return 0;
        }

        code_out = (code0 != CS_INSIDE) ? code0 : code1;

        x = 0.0f;
        y = 0.0f;

        if (code_out & CS_TOP) {
            x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
            y = ymax;
        } else if (code_out & CS_BOTTOM) {
            x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
            y = ymin;
        } else if (code_out & CS_RIGHT) {
            y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
            x = xmax;
        } else if (code_out & CS_LEFT) {
            y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
            x = xmin;
        }

        if (code_out == code0) {
            x0    = x;
            y0    = y;
            code0 = compute_outcode(x0, y0, xmin, ymin, xmax, ymax);
        } else {
            x1    = x;
            y1    = y;
            code1 = compute_outcode(x1, y1, xmin, ymin, xmax, ymax);
        }
    }
}

#define SH_MAX_VERTS  8192

typedef enum {
    EDGE_LEFT,
    EDGE_RIGHT,
    EDGE_BOTTOM,
    EDGE_TOP
} ClipEdge;

static int sh_inside(float x, float y, ClipEdge edge,
                     float xmin, float ymin, float xmax, float ymax)
{
    switch (edge) {
    case EDGE_LEFT:   return x >= xmin;
    case EDGE_RIGHT:  return x <= xmax;
    case EDGE_BOTTOM: return y >= ymin;
    case EDGE_TOP:    return y <= ymax;
    }
    return 0;
}

static Vertex sh_intersect(Vertex a, Vertex b, ClipEdge edge,
                           float xmin, float ymin, float xmax, float ymax)
{
    Vertex p;
    float  dx = b.x - a.x;
    float  dy = b.y - a.y;
    float  t  = 0.0f;

    switch (edge) {
    case EDGE_LEFT:
        t = (xmin - a.x) / dx;
        break;
    case EDGE_RIGHT:
        t = (xmax - a.x) / dx;
        break;
    case EDGE_BOTTOM:
        t = (ymin - a.y) / dy;
        break;
    case EDGE_TOP:
        t = (ymax - a.y) / dy;
        break;
    }

    p.x = a.x + t * dx;
    p.y = a.y + t * dy;
    return p;
}

static int sh_clip_edge(const Vertex *input, int input_count,
                        Vertex *output, ClipEdge edge,
                        float xmin, float ymin, float xmax, float ymax)
{
    int out_count = 0;
    int i;

    if (input_count == 0) {
        return 0;
    }

    for (i = 0; i < input_count; i++) {
        Vertex current = input[i];
        Vertex prev    = input[(i + input_count - 1) % input_count];

        int cur_in  = sh_inside(current.x, current.y, edge,
                                xmin, ymin, xmax, ymax);
        int prev_in = sh_inside(prev.x, prev.y, edge,
                                xmin, ymin, xmax, ymax);

        if (cur_in) {
            if (!prev_in) {
                if (out_count < SH_MAX_VERTS) {
                    output[out_count++] = sh_intersect(prev, current, edge,
                                                      xmin, ymin, xmax, ymax);
                }
            }
            if (out_count < SH_MAX_VERTS) {
                output[out_count++] = current;
            }
        } else {
            if (prev_in) {
                if (out_count < SH_MAX_VERTS) {
                    output[out_count++] = sh_intersect(prev, current, edge,
                                                      xmin, ymin, xmax, ymax);
                }
            }
        }
    }

    return out_count;
}

Polygon clip_polygon(const Polygon *poly,
                     float xmin, float ymin, float xmax, float ymax)
{
    Polygon out;
    Vertex buf_a[SH_MAX_VERTS];
    Vertex buf_b[SH_MAX_VERTS];
    Vertex *input  = buf_a;
    Vertex *output = buf_b;
    Vertex *tmp;
    int    count;
    int    edge;

    out.vertices = NULL;
    out.count    = 0;

    if (!poly || poly->count < 3) {
        return out;
    }

    count = (poly->count < SH_MAX_VERTS) ? poly->count : SH_MAX_VERTS;
    memcpy(input, poly->vertices, sizeof(Vertex) * (size_t)count);

    for (edge = EDGE_LEFT; edge <= EDGE_TOP; edge++) {
        count = sh_clip_edge(input, count, output, (ClipEdge)edge,
                             xmin, ymin, xmax, ymax);
        if (count == 0) {
            return out;
        }
        tmp    = input;
        input  = output;
        output = tmp;
    }

    if (count < 3) {
        return out;
    }

    out.vertices = (Vertex *)malloc(sizeof(Vertex) * (size_t)count);
    if (!out.vertices) {
        out.count = 0;
        return out;
    }
    memcpy(out.vertices, input, sizeof(Vertex) * (size_t)count);
    out.count = count;

    return out;
}
