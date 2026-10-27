#ifndef CONTEXT_H
#define CONTEXT_H

/*
 * abstracts away all that hardware-specific stuff (graphical context/renderer,
 * manipulating 2D/3D models, the file system, what physical inputs map to which
 * game inputs, etc)
 * 
 * if you want to port this game to something that doesn't support SDL/OpenGL, or
 * otherwise handles IO operations differently, you should (ideally) only have to
 * reimplement io.c
 */

typedef struct {

    /*
     * the following are only populated if the cursor is DISABLED, otherwise they are 0 (unless otherwise specified)
     */
    int hotbar_select; // -1 unless one of the number keys are pressed
    int hotbar_delta; // 0 unless a "left" or "right" key are pressed, in which case it's -1 or 1, respectively

    int camera_dx, camera_dy;
    int left, right, forward, backward, up, down;
    int attack, use;

    /*
     * the following are only populated if the cursor is ENABLED, otherwise they are 0
     */
    int menu_toggle; // toggling out of a menu, toggling into inventory (which is also the pause menu), etc
    int cursor_x, cursor_y;
    int cursor_toggle, cursor_alt_toggle;

} Input;

typedef struct {

	float x, y, z;
	float pitch, yaw;

} Transform;

typedef void *Texture;
typedef void *Mesh;

/*
 * implemented by window.c
 */
void initialize_window();

int get_window_width();
int get_window_height();

int game_is_running();
void set_cursor_enabled(int boolean);
void populate_input(Input *input);
void present();

/*
 * implemented by render.c
 */
void initialize_render();

Texture load_texture(const char *path);
void free_texture(Texture texture);

void mesh_set_texture(Mesh mesh, Texture texture);
void free_mesh(Mesh mesh);

/*
 * implemented by render_3D.c
 */
Mesh create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, Texture texture);
Mesh create_mesh_from_obj(const char *obj_path, Texture texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh mesh);
void draw_mesh_as_icon(const Mesh mesh, float screen_u, float screen_v, int pixel_dx, int pixel_dy);
void remesh_mesh(Mesh mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);

/*
 * implemented by render_sky.c
 */
Mesh create_sky_mesh();
void draw_sky_mesh(const Transform *camera, const Mesh mesh);

/*
 * implemented by render_2D.c
 */
//   x/y [0, texture w/h]          | the position in the texture to sample from
//   width/height [0, texture w/h] | the size of the area to sample from
//   u/v [-1, 1]                   | the screen UV where the anchor should be aligned
//   anchor_u/anchor_v [0, 1]      | the sprite UV of the anchor (origin in the bottom left)
Mesh create_sprite_mesh_from_matrix(int sample_x, int sample_y, int sample_w, int sample_h, float anchor_u, float anchor_v, Texture texture);
Mesh create_sprite_mesh(float anchor_u, float anchor_v, Texture texture);
void draw_sprite_mesh(const Mesh mesh, float screen_u, float screen_v, int pixel_dx, int pixel_dy);

// // TODO for registering where in a sprite mesh the user clicked/hovered, for buttons and stuff
// void sprite_mesh_window_pos_to_texture_pos(const Mesh mesh, int window_x, int window_y, int *texture_x, int *texture_y);

// // TODO for determining where to render item icons
// void sprite_mesh_texture_pos_to_window_uv(const Mesh mesh, int texture_x, int texture_y, int *window_u, int *window_v);

#endif