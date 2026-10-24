#ifndef UTIL_H
#define UTIL_H

/*
 * literally just all the configurable constants. all of them. that I want to be easily configured :)
 */
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define GUI_SCALE 4

#define WORLD_CHUNK_DIM 8

#define FOV 90

// #define DEBUG_NOCLIP

/*
 * implemented by ez_array.c
 */
typedef struct {

	unsigned char *data;
	int datasize;
	int bytecount;

} EZArray;

void append_ezarray(EZArray *array, void *data, int data_length);
int index_of_ezarray(EZArray *array, void *data, int data_length);
void clear_ezarray(EZArray *array);

#define INDEX_EZARRAY(array, type, i) (*(((type*) (array).data) + i))

#endif