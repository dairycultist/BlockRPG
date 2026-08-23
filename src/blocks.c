#include "blocks.h"

Texture blockmap_texture;

// block type registry
BlockType block_types[256] = {

	[ BLOCK_AIR ]	= (BlockType) { 0 },
	[ BLOCK_GRASS ]	= (BlockType) { 1, 1, 20, { MESH_DIRT_GRASS, 0, 1, 2 } },
	[ BLOCK_STONE ]	= (BlockType) { 1, 1, 60, { MESH_TOP_AND_BOTTOM, 3 } }
};

// stole this from nash so I don't have to use rand(). it's deterministic!
static unsigned int r_hash(unsigned int seed) {
	
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
}

void initialize_blocks() {

    blockmap_texture = load_texture("res/blockmap.png");
}

unsigned short get_block_ticks_to_break(block_t block) {

	return block_types[block].ticks_to_break;
}

unsigned char get_block_mesh_data(block_t block, int i) {

	return block_types[block].mesh_data[i];
}

//                                                                                              [ -x, +x, -z, +z, -y (bottom), +y (top) ]
static void helper_append_fullblock(EZArray *mesh_data, int *vertex_count, int x, int y, int z, unsigned char faces[6], unsigned char is_occluded[6]) {

	float u_sml, v_sml, u_big, v_big;

	// -x face
	if (!is_occluded[0]) {

		GET_SPRITEMAP_UV(faces[0], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			x, y, z,			-1, 0, 0,	u_big, v_sml,
			x, y, z + 1,		-1, 0, 0,	u_sml, v_sml,
			x, y + 1, z,		-1, 0, 0,	u_big, v_big,
			x, y + 1, z + 1,	-1, 0, 0,	u_sml, v_big,
			x, y + 1, z,		-1, 0, 0,	u_big, v_big,
			x, y, z + 1,		-1, 0, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +x face
	if (!is_occluded[1]) {

		GET_SPRITEMAP_UV(faces[1], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			x + 1, y, z,			1, 0, 0,	u_sml, v_sml,
			x + 1, y + 1, z,		1, 0, 0,	u_sml, v_big,
			x + 1, y, z + 1,		1, 0, 0,	u_big, v_sml,
			x + 1, y + 1, z + 1,	1, 0, 0,	u_big, v_big,
			x + 1, y, z + 1,		1, 0, 0,	u_big, v_sml,
			x + 1, y + 1, z,		1, 0, 0,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -z face
	if (!is_occluded[2]) {

		GET_SPRITEMAP_UV(faces[2], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			x, y, z,			0, 0, -1,	u_sml, v_sml,
			x, y + 1, z,		0, 0, -1,	u_sml, v_big,
			x + 1, y, z,		0, 0, -1,	u_big, v_sml,
			x + 1, y + 1, z,	0, 0, -1,	u_big, v_big,
			x + 1, y, z,		0, 0, -1,	u_big, v_sml,
			x, y + 1, z,		0, 0, -1,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +z face
	if (!is_occluded[3]) {

		GET_SPRITEMAP_UV(faces[3], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			x, y, z + 1,			0, 0, 1,	u_big, v_sml,
			x + 1, y, z + 1,		0, 0, 1,	u_sml, v_sml,
			x, y + 1, z + 1,		0, 0, 1,	u_big, v_big,
			x + 1, y + 1, z + 1,	0, 0, 1,	u_sml, v_big,
			x, y + 1, z + 1,		0, 0, 1,	u_big, v_big,
			x + 1, y, z + 1,		0, 0, 1,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// -y face
	if (!is_occluded[4]) {

		GET_SPRITEMAP_UV(faces[4], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			x, y, z,			0, -1, 0,	u_sml, v_sml,
			x + 1, y, z,		0, -1, 0,	u_sml, v_big,
			x, y, z + 1,		0, -1, 0,	u_big, v_sml,
			x + 1, y, z + 1,	0, -1, 0,	u_big, v_big,
			x, y, z + 1,		0, -1, 0,	u_big, v_sml,
			x + 1, y, z,		0, -1, 0,	u_sml, v_big,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}

	// +y face
	if (!is_occluded[5]) {

		GET_SPRITEMAP_UV(faces[5], u_sml, v_sml, u_big, v_big);

		float full_block_data[] = {
			x, 		y + 1, z,			0, 1, 0,	u_big, v_sml,
			x, 		y + 1, z + 1,		0, 1, 0,	u_sml, v_sml,
			x + 1, 	y + 1, z,			0, 1, 0,	u_big, v_big,
			x + 1, 	y + 1, z + 1,		0, 1, 0,	u_sml, v_big,
			x + 1, 	y + 1, z,			0, 1, 0,	u_big, v_big,
			x, 		y + 1, z + 1,		0, 1, 0,	u_sml, v_sml,
		};

		append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 6);
		*vertex_count += 6;
	}
}

static void helper_append_crossmodel(EZArray *mesh_data, int *vertex_count, int x, int y, int z, unsigned char face) {

	float offset_x = x + (r_hash(x * 51 + z * 12) % 30) * 0.01;
	float offset_y = y - (r_hash(x * 7 + z * 5) % 30) * 0.01;
	float offset_z = z + (r_hash(x * 19 + z * 154) % 30) * 0.01;

	float u_sml, v_sml, u_big, v_big;

	GET_SPRITEMAP_UV(face, u_sml, v_sml, u_big, v_big);

	float full_block_data[] = {
		offset_x,	    offset_y + 1, 	offset_z,		0, 1, 0,	u_big, v_sml,
		offset_x + .7,  offset_y + 1, 	offset_z + .7,	0, 1, 0,	u_sml, v_sml,
		offset_x,	    offset_y + 2, 	offset_z,		0, 1, 0,	u_big, v_big,
		offset_x + .7,  offset_y + 2, 	offset_z + .7,	0, 1, 0,	u_sml, v_big,
		offset_x,       offset_y + 2, 	offset_z,		0, 1, 0,	u_big, v_big,
		offset_x + .7,  offset_y + 1, 	offset_z + .7,	0, 1, 0,	u_sml, v_sml,

		offset_x,	    offset_y + 1, 	offset_z,		0, 1, 0,	u_big, v_sml,
		offset_x,	    offset_y + 2, 	offset_z,		0, 1, 0,	u_big, v_big,
		offset_x + .7,  offset_y + 1, 	offset_z + .7,	0, 1, 0,	u_sml, v_sml,
		offset_x + .7,  offset_y + 2, 	offset_z + .7,	0, 1, 0,	u_sml, v_big,
		offset_x + .7,  offset_y + 1, 	offset_z + .7,	0, 1, 0,	u_sml, v_sml,
		offset_x,	    offset_y + 2, 	offset_z,		0, 1, 0,	u_big, v_big,

		offset_x,		offset_y + 1, 	offset_z + .7,	0, 1, 0,	u_big, v_sml,
		offset_x + .7,	offset_y + 1, 	offset_z,		0, 1, 0,	u_sml, v_sml,
		offset_x,		offset_y + 2, 	offset_z + .7,	0, 1, 0,	u_big, v_big,
		offset_x + .7,	offset_y + 2, 	offset_z,		0, 1, 0,	u_sml, v_big,
		offset_x,		offset_y + 2, 	offset_z + .7,	0, 1, 0,	u_big, v_big,
		offset_x + .7,	offset_y + 1, 	offset_z,		0, 1, 0,	u_sml, v_sml,

		offset_x,		offset_y + 1, 	offset_z + .7,	0, 1, 0,	u_big, v_sml,
		offset_x,		offset_y + 2, 	offset_z + .7,	0, 1, 0,	u_big, v_big,
		offset_x + .7,	offset_y + 1, 	offset_z,		0, 1, 0,	u_sml, v_sml,
		offset_x + .7,	offset_y + 2, 	offset_z,		0, 1, 0,	u_sml, v_big,
		offset_x + .7,	offset_y + 1, 	offset_z,		0, 1, 0,	u_sml, v_sml,
		offset_x,		offset_y + 2, 	offset_z + .7,	0, 1, 0,	u_big, v_big,
	};

	append_ezarray(mesh_data, full_block_data, sizeof(float) * 8 * 24);
	*vertex_count += 24;
}

void append_block_to_mesh(EZArray *mesh_data, int *vertex_count, block_t block, int x, int y, int z, unsigned char is_occluded[6]) {

	switch (get_block_mesh_data(block, 0)) {

		case MESH_EMPTY: return;

		case MESH_TOP_AND_BOTTOM:

			helper_append_fullblock(mesh_data, vertex_count, x, y, z, (unsigned char[6]) {
				get_block_mesh_data(block, 1),
				get_block_mesh_data(block, 1),
				get_block_mesh_data(block, 1),
				get_block_mesh_data(block, 1),
				get_block_mesh_data(block, 1),
				get_block_mesh_data(block, 1)
			}, is_occluded);
			return;
		
		case MESH_DIRT_GRASS:

			if (is_occluded[5]) {

				helper_append_fullblock(mesh_data, vertex_count, x, y, z, (unsigned char[6]) {
					get_block_mesh_data(block, 3),
					get_block_mesh_data(block, 3),
					get_block_mesh_data(block, 3),
					get_block_mesh_data(block, 3),
					get_block_mesh_data(block, 3),
					get_block_mesh_data(block, 3)
				}, is_occluded);

			} else {

				helper_append_fullblock(mesh_data, vertex_count, x, y, z, (unsigned char[6]) {
					get_block_mesh_data(block, 2),
					get_block_mesh_data(block, 2),
					get_block_mesh_data(block, 2),
					get_block_mesh_data(block, 2),
					get_block_mesh_data(block, 3),
					get_block_mesh_data(block, 1)
				}, is_occluded);

				// randomly place tall grass
				if (r_hash(x * 108 + z * 4878) % 2 != 0)
					helper_append_crossmodel(mesh_data, vertex_count, x, y, z, 4 + (r_hash(x * y * z) % 17) / 16);
			}
			return;
	}
}