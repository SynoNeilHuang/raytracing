#ifndef __RAYTRACING_H
#define __RAYTRACING_H

#include "objects.h"
#include <stdint.h>
#ifdef THREAD
#include <pthread.h>
#define THREAD_NUM 32

typedef struct __THREAD_DATA__ {
    uint8_t *pixels;
    double *background_color;
    rectangular_node rectangulars;
    sphere_node spheres;
    light_node lights;
    const viewpoint *view;
    int width;
    int height;
    int tid;
} THREAD_DATA;

void *raytracing(void *thread_data);
#else
void raytracing(uint8_t *pixels, color background_color,
                rectangular_node rectangulars, sphere_node spheres,
                light_node lights, const viewpoint *view,
                int width, int height);
#endif
#endif
