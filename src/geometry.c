#include "geometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

int province_load(const char *filepath, Province *out)
{
    FILE *fp;
    int   polygon_count;
    int   i, j;

    if (!filepath || !out) {
        return -1;
    }

    fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[geometry] No se pudo abrir: %s\n", filepath);
        return -1;
    }

    if (fscanf(fp, "%d", &polygon_count) != 1 || polygon_count <= 0) {
        fprintf(stderr, "[geometry] Formato invalido (polygon_count): %s\n",
                filepath);
        fclose(fp);
        return -1;
    }

    out->polygons = (Polygon *)calloc((size_t)polygon_count, sizeof(Polygon));
    if (!out->polygons) {
        fprintf(stderr, "[geometry] Error de memoria al cargar: %s\n", filepath);
        fclose(fp);
        return -1;
    }
    out->polygon_count = polygon_count;

    for (i = 0; i < polygon_count; i++) {
        int vertex_count;

        if (fscanf(fp, "%d", &vertex_count) != 1 || vertex_count < 3) {
            fprintf(stderr,
                    "[geometry] Formato invalido (vertex_count en poly %d): %s\n",
                    i, filepath);
            out->polygon_count = i;
            province_free(out);
            fclose(fp);
            return -1;
        }

        out->polygons[i].vertices =
            (Vertex *)malloc(sizeof(Vertex) * (size_t)vertex_count);
        if (!out->polygons[i].vertices) {
            fprintf(stderr, "[geometry] Error de memoria en poly %d: %s\n",
                    i, filepath);
            out->polygon_count = i;
            province_free(out);
            fclose(fp);
            return -1;
        }
        out->polygons[i].count = vertex_count;

        for (j = 0; j < vertex_count; j++) {
            float x, y;
            if (fscanf(fp, "%f %f", &x, &y) != 2) {
                fprintf(stderr,
                        "[geometry] Error leyendo vertice %d del poly %d: %s\n",
                        j, i, filepath);
                out->polygon_count = i + 1;
                province_free(out);
                fclose(fp);
                return -1;
            }
            out->polygons[i].vertices[j].x = x;
            out->polygons[i].vertices[j].y = y;
        }
    }

    fclose(fp);
    return 0;
}

void polygon_free(Polygon *poly)
{
    if (poly && poly->vertices) {
        free(poly->vertices);
        poly->vertices = NULL;
        poly->count    = 0;
    }
}

void province_free(Province *prov)
{
    if (!prov) {
        return;
    }
    if (prov->polygons) {
        int i;
        for (i = 0; i < prov->polygon_count; i++) {
            polygon_free(&prov->polygons[i]);
        }
        free(prov->polygons);
        prov->polygons      = NULL;
        prov->polygon_count = 0;
    }
}

void province_compute_bbox(Province *prov)
{
    int i, j;

    if (!prov || prov->polygon_count == 0) {
        return;
    }

    prov->bbox.min_x =  1e30f;
    prov->bbox.min_y =  1e30f;
    prov->bbox.max_x = -1e30f;
    prov->bbox.max_y = -1e30f;

    for (i = 0; i < prov->polygon_count; i++) {
        for (j = 0; j < prov->polygons[i].count; j++) {
            float x = prov->polygons[i].vertices[j].x;
            float y = prov->polygons[i].vertices[j].y;
            if (x < prov->bbox.min_x) prov->bbox.min_x = x;
            if (y < prov->bbox.min_y) prov->bbox.min_y = y;
            if (x > prov->bbox.max_x) prov->bbox.max_x = x;
            if (y > prov->bbox.max_y) prov->bbox.max_y = y;
        }
    }
}

BoundingBox provinces_global_bbox(const Province *provinces, int count)
{
    BoundingBox bb = {1e30f, 1e30f, -1e30f, -1e30f};
    int i;

    for (i = 0; i < count; i++) {
        if (provinces[i].bbox.min_x < bb.min_x) bb.min_x = provinces[i].bbox.min_x;
        if (provinces[i].bbox.min_y < bb.min_y) bb.min_y = provinces[i].bbox.min_y;
        if (provinces[i].bbox.max_x > bb.max_x) bb.max_x = provinces[i].bbox.max_x;
        if (provinces[i].bbox.max_y > bb.max_y) bb.max_y = provinces[i].bbox.max_y;
    }

    return bb;
}
