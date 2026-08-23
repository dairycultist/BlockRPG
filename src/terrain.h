#ifndef TERRAIN_H
#define TERRAIN_H

// exposes terrain manipulation as globally positioned and chunk-agnostic (abstracting away chunks basically)

#include "io.h"
#include "entities.h"
#include "blocks.h"

void initialize_terrain();
void draw_chunks(const Transform *camera);

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