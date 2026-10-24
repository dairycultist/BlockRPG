#ifndef UTIL_H
#define UTIL_H

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