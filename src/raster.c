#include <stdlib.h>
#include "raster.h"

//draw a line between two points with midpoint algorithm
void draw_line_bresenham(Framebuffer *fb, 
    int x0, int y0, int x1, int y1, Color color){

    int dx, dy, sx, sy, d;
    //abs distances
    dx = abs(x1 - x0);
    dy = abs(y1 - y0);

    //Determine direction for both x and y
    //1: move right (sx), move up (sy)
    //-1: move left (sx), move down (sy)
    sx = (x0 < x1) ? 1 : -1; 
    sy = (y0 < y1) ? 1 : -1;

    //variable used to choose next pixel
    d = dx - dy; 

    //continue until reaching endpoint
    while (1) {
        framebuffer_put_pixel(fb, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int d_2 = 2 * d;

        if (d_2 > -dy) {
            d -= dy;
            x0 += sx;
        }

        if (d_2 < dx) {
            d += dx;
            y0 += sy;
        }
    }
}

void draw_polygon_wireframe(Framebuffer *fb, 
    const Polygon *poly, Color color){
    int i;

    if (!fb || !poly || !poly->vertices || poly->count < 2)
        return;

    for (i = 0; i < poly->count; i++) {
        int next = (i + 1) % poly->count;

        int x0 = (int)poly->vertices[i].x;
        int y0 = (int)poly->vertices[i].y;

        int x1 = (int)poly->vertices[next].x;
        int y1 = (int)poly->vertices[next].y;

        draw_line_bresenham(fb, x0, y0, x1, y1, color);
    }
}

void fill_polygon_solid(Framebuffer *fb,
                        const Polygon *poly, Color color){
    typedef struct {
        int y_min;
        int y_max;
        float x;
        float dx_dy;
    } Edge;

    Edge *edges;
    Edge *active;
    int edge_count = 0;
    int active_count;
    int i;
    int y;

    if (!fb || !poly || !poly->vertices || poly->count < 3)
        return;

    //create the list of borders.
    //Horizontal borders are ignored, not intersection with scanline
    edges = malloc(sizeof(Edge) * poly->count);

    if (!edges)
        return;

    for (i = 0; i < poly->count; i++){
        int next = (i + 1) % poly->count;

        int x0 = (int)poly->vertices[i].x;
        int y0 = (int)poly->vertices[i].y;
        int x1 = (int)poly->vertices[next].x;
        int y1 = (int)poly->vertices[next].y;

        if (y0 == y1)
            continue;

        //Store intersection from its lower endpoint to its
        //upper endpoint
        if (y0 < y1){
            edges[edge_count].y_min = y0;
            edges[edge_count].y_max = y1;
            edges[edge_count].x = (float)x0;
            edges[edge_count].dx_dy =
                (float)(x1 - x0) / (float)(y1 - y0);
        } else{
            edges[edge_count].y_min = y1;
            edges[edge_count].y_max = y0;
            edges[edge_count].x = (float)x1;
            edges[edge_count].dx_dy =
                (float)(x0 - x1) / (float)(y0 - y1);
        }

        edge_count++;
    }

    active = malloc(sizeof(Edge) * edge_count);

    if (!active) {
        free(edges);
        return;
    }

    //Start at highest scanline of the window
    //and move downward one scanline at a time
    for (y = fb->height - 1; y >= 0; y--){
        active_count = 0;

        //Find intersections for current
        //scanline
        for (i = 0; i < edge_count; i++){
            if (y >= edges[i].y_min &&
                y < edges[i].y_max)
            {
                active[active_count] = edges[i];

                //Calculate intersection for
                //current scanline.
                active[active_count].x =
                    edges[i].x +
                    edges[i].dx_dy *
                    (float)(y - edges[i].y_min);

                active_count++;
            }
        }

        //Sort intersections from left to right.
        for (i = 0; i < active_count - 1; i++)
        {
            int j;

            for (j = i + 1; j < active_count; j++)
            {
                if (active[j].x < active[i].x)
                {
                    Edge temp = active[i];
                    active[i] = active[j];
                    active[j] = temp;
                }
            }
        }

        //Fill between pairs of intersections:
        //0 -> 1 paint
        //2 -> 3 not paint
        //4 -> 5 paint
        for (i = 0; i + 1 < active_count; i += 2)
        {
            int x_start = (int)active[i].x;
            int x_end   = (int)active[i + 1].x;
            int x;

            for (x = x_start; x <= x_end; x++)
            {
                framebuffer_put_pixel(fb, x, y, color);
            }
        }
    }

    free(active);
    free(edges);
}   
