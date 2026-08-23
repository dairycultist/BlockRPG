#ifndef BLOCKS_H
#define BLOCKS_H

// abstracts away internal format of block types

#include "io.h"
#include "ez_array.h"

// block mesh types
#define MESH_EMPTY 0
#define MESH_TOP_AND_BOTTOM 1
#define MESH_DIRT_GRASS 2

// only 256 block types can exist
typedef unsigned char block_t;

extern Texture blockmap_texture;

#define BLOCK_AIR 0
#define BLOCK_GRASS 1
#define BLOCK_STONE 2

void initialize_blocks();

unsigned short get_block_ticks_to_break(block_t block);
unsigned char get_block_is_fullblock(block_t block);
unsigned char get_block_is_collidable(block_t block);
unsigned char get_block_mesh_data(block_t block, int i);

// is_occluded takes arguments in this order: [ -x, +x, -z, +z, -y (bottom), +y (top) ]
void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, block_t block, int x, int y, int z, unsigned char is_occluded[6]);

#endif