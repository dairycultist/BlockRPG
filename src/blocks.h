#ifndef BLOCKS_H
#define BLOCKS_H

#include "io.h"
#include "ez_array.h"

// TODO abstracts away internal format of block types (well not yet since I'd need to rewrite a bit of terrain.c, but it will!)

// block mesh types
#define MESH_EMPTY 0
#define MESH_TOP_AND_BOTTOM 1
#define MESH_DIRT_GRASS 2

typedef struct { // TODO make this "static"

	// TODO stuff like:
	// - mining level
	// - dropped item

	unsigned char is_fullblock; // adjacent blocks will cull the faces that touch it
	unsigned char is_collidable; // has full-block collision

	unsigned short ticks_to_break;

	// first element is always the mesh type
	// the use of latter elements is determined by the mesh type but is usually atlas indices (can also be orientation!)
	unsigned char mesh_data[4];

} BlockType;

#define GET_SPRITEMAP_UV(index, u_sml, v_sml, u_big, v_big) u_sml = ((index) % 16) / 16.; v_sml = ((index) / 16) / 16.; u_big = (((index) + 1) % 16) / 16.; v_big = ((index) / 16 + 1) / 16.

// only 256 block types can exist
typedef unsigned char block_t;

extern BlockType block_types[256]; // TODO make this static
extern Texture blockmap_texture;

#define BLOCK_AIR 0
#define BLOCK_GRASS 1
#define BLOCK_STONE 2

void initialize_blocks();

unsigned short get_block_ticks_to_break(block_t block);
unsigned char get_block_mesh_data(block_t block, int i);

void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, block_t block, int x, int y, int z, unsigned char is_occluded[6]);

#endif