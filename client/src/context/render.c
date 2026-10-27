#include "context.h"
#include "render.h"

/*
 * implemented for context.h
 */
void initialize_render() {

	initialize_render_3D();
	initialize_render_sky();
	initialize_render_2D();
}

Texture load_texture(const char *path) {

	TextureGL *texture_gl = malloc(sizeof(TextureGL));

	SDL_Surface *surface = IMG_Load(path);
	
	texture_gl->w = surface->w;
	texture_gl->h = surface->h;

	// flip texture vertically to match OpenGL spec
	unsigned char *pixels = (unsigned char *) surface->pixels;
	int pixelBytes = surface->format->BytesPerPixel;

	for (int y = 0; y < surface->h / 2; y++) {

		for (int x = 0; x < surface->w; x++) {

			int top = x + y * surface->h;
			int bottom = x + (surface->w * surface->h - (y + 1) * surface->h);

			for (int i=0; i<pixelBytes; i++) {

				unsigned char hold = pixels[top * pixelBytes + i];
				pixels[top * pixelBytes + i] = pixels[bottom * pixelBytes + i];
				pixels[bottom * pixelBytes + i] = hold;
			}
		}
	}

	// create texture object
	glGenTextures(1, &texture_gl->obj);

	// bind texture (to active texture 2D)
	glBindTexture(GL_TEXTURE_2D, texture_gl->obj);

	// wrap repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// filter linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// write texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	// clean up
	SDL_FreeSurface(surface);

	return texture_gl;
}

void free_texture(Texture texture) {

	glDeleteTextures(1, &((TextureGL *) texture)->obj);
	free(texture);
}

void mesh_set_texture(Mesh mesh, Texture texture) {

	((MeshGL *) mesh)->texture_gl = (TextureGL *) texture;
}

void free_mesh(Mesh mesh) {

	glDeleteVertexArrays(1, &((MeshGL *) mesh)->vertex_array);
	free(mesh);
}

/*
 * implemented for render.h
 * helpers for the various render_*.c files
 */
void create_shader_program(GLuint *const program_out, const char *vertex_code, const char *fragment_code) {

	*program_out = glCreateProgram();

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char *const *) &vertex_code, NULL);
	glCompileShader(vertex_shader);
	glAttachShader(*program_out, vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char *const *) &fragment_code, NULL);
	glCompileShader(fragment_shader);
	glAttachShader(*program_out, fragment_shader);

	glLinkProgram(*program_out);
}

void mat4_mult(const GLfloat b[4][4], const GLfloat a[4][4], GLfloat out[4][4]) {

	// a (rightmost) is applied first, then b

	GLfloat matrix[4][4] = {
		{
			a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0],
			a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1],
			a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2],
			a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3],
		},
		{
			a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0],
			a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1],
			a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2],
			a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3],
		},
		{
			a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0],
			a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1],
			a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2],
			a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3],
		},
		{
			a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0],
			a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1],
			a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2],
			a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3],
		},
	};

	int x, y;

	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			out[x][y] = matrix[x][y];
		}
	}
}

void mat4_translate(GLfloat mat[4][4], float dx, float dy, float dz) {

	for (int i = 0; i < 4; i++) {

		mat[i][0] += mat[i][3] * dx;
		mat[i][1] += mat[i][3] * dy;
		mat[i][2] += mat[i][3] * dz;
	}
}

void generate_rotation_matrices(GLfloat pitch_matrix[4][4], float pitch, GLfloat yaw_matrix[4][4], float yaw) {

	pitch_matrix[0][0] = 1;
	pitch_matrix[0][1] = 0;
	pitch_matrix[0][2] = 0;
	pitch_matrix[0][3] = 0;

	pitch_matrix[1][0] = 0;
	pitch_matrix[1][1] = cos(pitch);
	pitch_matrix[1][2] = -sin(pitch);
	pitch_matrix[1][3] = 0;

	pitch_matrix[2][0] = 0;
	pitch_matrix[2][1] = sin(pitch);
	pitch_matrix[2][2] = cos(pitch);
	pitch_matrix[2][3] = 0;

	pitch_matrix[3][0] = 0;
	pitch_matrix[3][1] = 0;
	pitch_matrix[3][2] = 0;
	pitch_matrix[3][3] = 1;

	yaw_matrix[0][0] = cos(yaw);
	yaw_matrix[0][1] = 0;
	yaw_matrix[0][2] = sin(yaw);
	yaw_matrix[0][3] = 0;

	yaw_matrix[1][0] = 0;
	yaw_matrix[1][1] = 1;
	yaw_matrix[1][2] = 0;
	yaw_matrix[1][3] = 0;

	yaw_matrix[2][0] = -sin(yaw);
	yaw_matrix[2][1] = 0;
	yaw_matrix[2][2] = cos(yaw);
	yaw_matrix[2][3] = 0;

	yaw_matrix[3][0] = 0;
	yaw_matrix[3][1] = 0;
	yaw_matrix[3][2] = 0;
	yaw_matrix[3][3] = 1;
}