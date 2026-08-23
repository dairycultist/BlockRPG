#ifndef IO_H
#define IO_H

// exposes helpers for receiving player input, model manipulation and rendering, etc

// abstracts away all that hardware-specific stuff (graphical context/renderer, the
// file system, what physical inputs map to which game inputs, etc)

// if you want to port this game to something that doesn't support SDL/OpenGL, or
// otherwise handles rendering, input, file loading, etc differently, you should
// only have to reimplement io.c

typedef struct {

    // the following are only populated if the cursor is DISABLED, otherwise they are 0
    int camera_dx, camera_dy;
    int left, right, forward, backward, up, down;
    int attack, use; // maybe change to primary and secondary

    // the following are only populated if the cursor is ENABLED, otherwise they are 0
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

void initialize_io();

int game_is_running();
void set_cursor_enabled(int boolean);
void populate_input(Input *input);
void present();

// textures
Texture load_texture(const char *path);
void free_texture(Texture texture);

// 3D meshes
Mesh create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, Texture texture);
Mesh create_mesh_from_obj(const char *obj_path, Texture texture);
void draw_mesh(const Transform *camera, const Transform *transform, const Mesh mesh);
void draw_mesh_as_icon(const Mesh mesh, float screen_u, float screen_v);
void remesh_mesh(Mesh mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount);

// sky meshes
Mesh create_sky_mesh();
void draw_sky_mesh(const Transform *camera, const Mesh mesh);

// 2D meshes
//   x/y [0, texture w/h]          | the position in the texture to sample from
//   width/height [0, texture w/h] | the size of the area to sample from
//   u/v [-1, 1]                   | the screen UV where the anchor should be aligned
//   anchor_u/anchor_v [0, 1]      | the sprite UV of the anchor (origin in the bottom left)
//   h_pixels                      | height of the sprite mesh when rendered to the window in pixels (width is calculated automatically)
Mesh create_sprite_mesh_from_matrix(int x, int y, int width, int height, float u, float v, float anchor_u, float anchor_v, int h_pixels, Texture texture);
Mesh create_sprite_mesh(float u, float v, float anchor_u, float anchor_v, int h_pixels, Texture texture);
void draw_sprite_mesh(const Mesh mesh);

// TODO for registering where in a sprite mesh the user clicked/hovered, for buttons and stuff
void sprite_mesh_window_pos_to_texture_pos(const Mesh mesh, int window_x, int window_y, int *texture_x, int *texture_y);

// TODO for determining where to render item icons
void sprite_mesh_texture_pos_to_window_uv(const Mesh mesh, int texture_x, int texture_y, int *window_u, int *window_v);

// misc
void mesh_set_texture(Mesh mesh, Texture texture);
void free_mesh(Mesh mesh);

#endif