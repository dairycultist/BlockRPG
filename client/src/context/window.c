#include "context.h"

#include "../util/util.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>

static SDL_Window *window;
static int running = 1;
static int is_cursor_enabled;

static int window_w = WINDOW_WIDTH, window_h = WINDOW_HEIGHT;

static void log_error(const char *msg) {
	
	if (strlen(SDL_GetError()) == 0) {
		fprintf(stderr, "\n%s: <No error given>\n\n", msg);
	} else {
		fprintf(stderr, "\n%s: %s\n\n", msg, SDL_GetError());
	}
}

void initialize_window() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		log_error("Could not initialize SDL");
		exit(1);
	}

	// init OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	window = SDL_CreateWindow("CinnamonCraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!window) {
        log_error("Could not create window");
		exit(1);
    }

	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glClearColor(1.0f, 0.188f, 0.647f, 1.0f); // since the sky is rendered as a mesh, set the clear color to hot pink so it's obvious
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // only sprites enable blending

	SDL_SetRelativeMouseMode(SDL_TRUE);
}

int get_window_width() {

	return window_w;
}

int get_window_height() {

	return window_h;
}

int game_is_running() {

    return running;
}

void set_cursor_enabled(int boolean) {

	is_cursor_enabled = boolean;

	SDL_SetRelativeMouseMode(!boolean);

	if (boolean)
		SDL_WarpMouseInWindow(window, window_w / 2, window_h / 2);
}

void populate_input(Input *input) {

	if (is_cursor_enabled) {

		input->left = 0;
		input->right = 0;
		input->forward = 0;
		input->backward = 0;
		input->up = 0;
		input->down = 0;
		input->attack = 0;
		input->use = 0;

	} else {

		input->cursor_x = 0;
		input->cursor_y = 0;
	}

	input->hotbar_select = -1;
	input->hotbar_delta = 0;

    input->camera_dx = 0.0;
    input->camera_dy = 0.0;

	input->menu_toggle = 0;
	input->cursor_toggle = 0;
	input->cursor_alt_toggle = 0;

    input->use = 0; // TODO replace with logical cooldown (for block placement)

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {

            running = 0;

		} else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {

			window_w = event.window.data1;
			window_h = event.window.data2;

			glViewport(0, 0, window_w, window_h);
        
        } else if (event.type == SDL_MOUSEMOTION) {

			if (is_cursor_enabled) {

				input->cursor_x = (int) event.motion.x; // Sint32
				input->cursor_y = (int) event.motion.y;

			} else {

				input->camera_dx = (int) event.motion.xrel; // Sint32
				input->camera_dy = (int) event.motion.yrel;
			}

        } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {

			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {

				input->menu_toggle = 1;
			
			} else if (!is_cursor_enabled) {

				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_A: input->left = 1; break;
					case SDL_SCANCODE_D: input->right = 1; break;
					case SDL_SCANCODE_W: input->forward = 1; break;
					case SDL_SCANCODE_S: input->backward = 1; break;
					case SDL_SCANCODE_SPACE: input->up = 1; break;
					case SDL_SCANCODE_LSHIFT: input->down = 1; break;
					case SDL_SCANCODE_MINUS: input->hotbar_delta = -1; break;
					case SDL_SCANCODE_EQUALS: input->hotbar_delta = 1; break;
					case SDL_SCANCODE_1: input->hotbar_select = 0; break;
					case SDL_SCANCODE_2: input->hotbar_select = 1; break;
					case SDL_SCANCODE_3: input->hotbar_select = 2; break;
					case SDL_SCANCODE_4: input->hotbar_select = 3; break;
					case SDL_SCANCODE_5: input->hotbar_select = 4; break;
					case SDL_SCANCODE_6: input->hotbar_select = 5; break;
					case SDL_SCANCODE_7: input->hotbar_select = 6; break;
					case SDL_SCANCODE_8: input->hotbar_select = 7; break;
					case SDL_SCANCODE_9: input->hotbar_select = 8; break;
					case SDL_SCANCODE_0: input->hotbar_select = 9; break;
				}
			}

        } else if (event.type == SDL_KEYUP) {

			if (!is_cursor_enabled) {

				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_A: input->left = 0; break;
					case SDL_SCANCODE_D: input->right = 0; break;
					case SDL_SCANCODE_W: input->forward = 0; break;
					case SDL_SCANCODE_S: input->backward = 0; break;
					case SDL_SCANCODE_SPACE: input->up = 0; break;
					case SDL_SCANCODE_LSHIFT: input->down = 0; break;
				}
			}

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {

            if (event.button.button == 1) { // LMB

				if (is_cursor_enabled)
					input->cursor_toggle = 1;
				else
                	input->attack = 1;
				
            } else if (event.button.button == 3) { // RMB

				if (is_cursor_enabled)
					input->cursor_alt_toggle = 1;
				else
                	input->use = 1;
            }
        }

        else if (event.type == SDL_MOUSEBUTTONUP) {

			if (!is_cursor_enabled) {

				if (event.button.button == 1) { // LMB
					input->attack = 0;
				} else if (event.button.button == 3) { // RMB
					input->use = 0;
				}
			}
        }
    }
}

void present() {

    SDL_GL_SwapWindow(window);
    SDL_Delay(1000 / 60);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
