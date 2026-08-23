#include "main.h"
#include "io.h"
#include "terrain.h"
#include "player.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {

	initialize_io();
	initialize_blocks();
	initialize_terrain();
	initialize_entities();
	initialize_player();

	for (int x = 0; x < WORLD_CHUNK_DIM * 16; x++) {
	for (int z = 0; z < WORLD_CHUNK_DIM * 16; z++) {

			set_delay_remesh_block_at(x, 20, z, BLOCK_GRASS);

			for (int y = 0; y < 20; y++) {
				set_delay_remesh_block_at(x, y, z, BLOCK_STONE);
			}
	}}
	remesh_delayed_chunks();

	Input input = { 0 };

	while (game_is_running()) {

		populate_input(&input);
		
		entities_process_tick();
		player_process_tick(&input);

		present();
	}

	return 0;
}