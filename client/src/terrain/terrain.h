#ifndef TERRAIN_H
#define TERRAIN_H

#include "../context/context.h"
#include "../entity/entity.h"
#include "../util/util.h"

/*
 * implemented by block.h
 * abstracts away internal format of block types
 */

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

/*
 * implemented by chunk.h
 * exposes terrain manipulation as globally positioned and chunk-agnostic (abstracting away chunks basically)
 */

void initialize_terrain();
void draw_terrain(const Transform *camera);

// block info
block_t get_block_at(int x, int y, int z);

// block setting/remeshing
void set_block_at(int x, int y, int z, block_t block);
void set_delay_remesh_block_at(int x, int y, int z, block_t block);
void remesh_delayed_chunks();

// collision
int does_point_intersect_blocks(int x, int y, int z);
int does_aabb_intersect_blocks(AABB *aabb);
int would_aabb_intersect_block_at(int x, int y, int z, block_t block, AABB *aabb);
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z);

#endif