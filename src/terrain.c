#include "main.h"
#include "terrain.h"
#include "ez_array.h"

#include <stdlib.h>
#include <math.h>

// technically we don't need this since blocks are never at negative positions, but whatever
#define SAFEMOD(a, b) ((((a) % (b)) + b) % b)

typedef struct { // only this file knows Chunks even exist

	Mesh mesh;
	block_t blocks[16][128][16]; // array of bytes indexing into block_types

	int chunk_x, chunk_z; // we're storing this for when you're accessing this chunk through a pointer instead of indexing the chunks matrix

} Chunk;

static Chunk chunks[WORLD_CHUNK_DIM][WORLD_CHUNK_DIM];

static EZArray delayed_remesh_chunks; // stores Chunk *; when you want to set a bunch of blocks, remeshing after each is slow and redundant, so you save them to remesh once at the end

// remeshes based on the chunk's internal blocks
static void remesh_chunk(Chunk *chunk) {

	EZArray mesh_data = {0};

	int vertex_count = 0;

	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 128; y++) {
			for (int z = 0; z < 16; z++) {

				int global_x = x + chunk->chunk_x * 16;
				int global_z = z + chunk->chunk_z * 16;

				append_block_to_mesh(
					&mesh_data,
					&vertex_count,
					get_block_at(x + chunk->chunk_x * 16, y, z + chunk->chunk_z * 16),
					x,
					y,
					z,
					(unsigned char[6]) {
						block_types[(get_block_at(global_x - 1, y, 		global_z	))].is_fullblock,
						block_types[(get_block_at(global_x + 1, y, 		global_z	))].is_fullblock,
						block_types[(get_block_at(global_x, 	y, 		global_z - 1))].is_fullblock,
						block_types[(get_block_at(global_x, 	y, 		global_z + 1))].is_fullblock,
						block_types[(get_block_at(global_x, 	y - 1, 	global_z	))].is_fullblock,
						block_types[(get_block_at(global_x, 	y + 1, 	global_z	))].is_fullblock
					}
				);
			}
		}
	}

	// create mesh
	remesh_mesh(chunk->mesh, mesh_data.data, mesh_data.bytecount, vertex_count);
}

void initialize_terrain() {

	for (int x = 0; x < WORLD_CHUNK_DIM; x++) {
		for (int z = 0; z < WORLD_CHUNK_DIM; z++) {

			chunks[x][z].mesh = create_mesh(NULL, 0, 0, blockmap_texture);

			chunks[x][z].chunk_x = x;
			chunks[x][z].chunk_z = z;
		}
	}
}

void draw_chunks(const Transform *camera) {

	Transform chunk_transform = {0};
	Chunk *chunk;

	for (int x = 0; x < WORLD_CHUNK_DIM; x++) {
		for (int z = 0; z < WORLD_CHUNK_DIM; z++) {

			chunk_transform.x = x * 16;
			chunk_transform.z = z * 16;

			draw_mesh(camera, &chunk_transform, chunks[x][z].mesh);
		}
	}
}

static Chunk *get_chunk_of_block(int x, int y, int z) {

	if (y < 0 || y >= 128) // we include y so that OOB y will return NULL
		return NULL;
	
	if (x < 0 || x >= WORLD_CHUNK_DIM * 16 || z < 0 || z >= WORLD_CHUNK_DIM * 16)
		return NULL;

	return &chunks[(int) floor(x / 16.0)][(int) floor(z / 16.0)];
}

block_t get_block_at(int x, int y, int z) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return 0;
	
	return chunk->blocks[SAFEMOD(x, 16)][y][SAFEMOD(z, 16)];
}

void set_block_at(int x, int y, int z, block_t block) {
	
	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[SAFEMOD(x, 16)][y][SAFEMOD(z, 16)] = block;

	remesh_chunk(chunk);

	// if block was at the edge of a chunk, also remesh the adjacent chunk(s)
	if (SAFEMOD(x, 16) == 0) {
		chunk = get_chunk_of_block(x - 1, y, z);
		if (chunk)
			remesh_chunk(chunk);
	} else if (SAFEMOD(x, 16) == 15) {
		chunk = get_chunk_of_block(x + 1, y, z);
		if (chunk)
			remesh_chunk(chunk);
	}

	if (SAFEMOD(z, 16) == 0) {
		chunk = get_chunk_of_block(x, y, z - 1);
		if (chunk)
			remesh_chunk(chunk);
	} else if (SAFEMOD(z, 16) == 15) {
		chunk = get_chunk_of_block(x, y, z + 1);
		if (chunk)
			remesh_chunk(chunk);
	}
}

void set_delay_remesh_block_at(int x, int y, int z, block_t block) {

	Chunk *chunk = get_chunk_of_block(x, y, z);

	if (!chunk)
		return;

	chunk->blocks[SAFEMOD(x, 16)][y][SAFEMOD(z, 16)] = block;

	// save chunk for delayed remeshing if it's not already saved
	if (index_of_ezarray(&delayed_remesh_chunks, &chunk, sizeof(Chunk *)) == -1)
		append_ezarray(&delayed_remesh_chunks, &chunk, sizeof(Chunk *));
}

void remesh_delayed_chunks() {

	for (int i = 0; i < delayed_remesh_chunks.bytecount / sizeof(Chunk *); i++)
		remesh_chunk(INDEX_EZARRAY(delayed_remesh_chunks, Chunk *, i));

	clear_ezarray(&delayed_remesh_chunks);
}

int does_point_intersect_blocks(int x, int y, int z) {

	return block_types[get_block_at(x, y, z)].is_collidable;
}

int does_aabb_intersect_blocks(AABB *aabb) {

	for (int block_x = (int) (aabb->x - aabb->r); block_x <= (int) (aabb->x + aabb->r); block_x++) {
	for (int block_z = (int) (aabb->z - aabb->r); block_z <= (int) (aabb->z + aabb->r); block_z++) {
	for (int block_y = (int) aabb->y;             block_y <= (int) (aabb->y + aabb->h); block_y++) {

		if (does_point_intersect_blocks(block_x, block_y, block_z))
			return 1;
	}}}

	return 0;
}

int would_aabb_intersect_block_at(int x, int y, int z, block_t block, AABB *aabb) {

	BlockType *block_type = &block_types[block];

	if (!(block_type->is_collidable))
		return 0;

	float aabb_x = aabb->x - x;
	float aabb_y = aabb->y - y;
	float aabb_z = aabb->z - z;

	return (aabb_x - aabb->r < 1.0) && (aabb_z - aabb->r < 1.0) && (aabb_y < 1.0)
		&& (aabb_x + aabb->r > 0.0) && (aabb_z + aabb->r > 0.0) && (aabb_y + aabb->h > 0.0);
}

// returns true if it hit a block, in which case it populates the output parameters with the position of the block
int raycast_blocks(const Transform *origin, float max_dist, int return_surface, int *out_x, int *out_y, int *out_z) {

	#define STEP_SIZE 0.1

	float x = origin->x, y = origin->y, z = origin->z;
	float dx, dy, dz;

	dx = STEP_SIZE *  sin(origin->yaw) * cos(origin->pitch);
	dz = STEP_SIZE * -cos(origin->yaw) * cos(origin->pitch);

	dy = STEP_SIZE * -sin(origin->pitch);

	for (float dist = 0.0; dist < max_dist; dist += STEP_SIZE) {
		
		if (does_point_intersect_blocks(x, y, z)) {

			if (return_surface) {

				x -= dx;
				y -= dy;
				z -= dz;
			}

			*out_x = floor(x);
			*out_y = floor(y);
			*out_z = floor(z);

			return 1;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	return 0;
}