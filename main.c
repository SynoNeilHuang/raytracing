#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "primitives.h"
#include "raytracing.h"

#define OUT_FILENAME "out.ppm"

#define ROWS 512
#define COLS 512

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

void thread_data_init(THREAD_DATA *input, uint8_t *pixels,
	   color background, rectangular_node rectangulars,
	   sphere_node spheres, light_node lights, const viewpoint *view,
	   int width, int height, int tid)
{
    input->pixels = pixels;
    input->background_color = background;
    input->rectangulars = rectangulars;
    input->spheres = spheres;
    input->lights = lights;
    input->view = view;
    input->width = width;
    input->height = height;
    input->tid = tid;
}

int main()
{
    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    color background = { 0.0, 0.1, 0.1 };
    struct timespec start, end;

#ifdef THREAD
    THREAD_DATA input[THREAD_NUM];
    pthread_t t_handler[THREAD_NUM];
#endif

#include "use-models.h"

    /* allocate by the given resolution */
    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    if (!pixels) exit(-1);

    printf("# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);
#ifdef THREAD
    for (int i = 0; i < THREAD_NUM; ++i) {
	thread_data_init(&input[i], pixels, background, rectangulars, spheres, lights, &view, ROWS, COLS, i);
	pthread_create(&t_handler[i], NULL, raytracing, (void *)&input[i]);
    }
    for (int i = 0; i < THREAD_NUM; ++i)
	pthread_join(t_handler[i], NULL);
#else
    raytracing(pixels, background,
               rectangulars, spheres, lights, &view, ROWS, COLS);
#endif /* THREAD */
    clock_gettime(CLOCK_REALTIME, &end);
    {
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }

    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
    free(pixels);
    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
