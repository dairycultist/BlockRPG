#include "context.h"
#include "render.h"

static const char *vertex2D =
"#version 150 core\n"
"uniform mat4 position_matrix;\n"
"in vec2 position;\n"
"in vec2 UV;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
    "gl_Position = position_matrix * vec4(position.xy, -1.0, 1.0);\n" // get final position
    "frag_UV = UV;\n" // pass along UV
"}";

static const char *fragment2D =
"#version 150 core\n"
"uniform sampler2D tex;\n"
"in vec2 frag_UV;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"outColor = texture(tex, frag_UV);\n" // we allow full transparency because 2D sprites are rendered painterly
"}";

static GLuint shader2D_program;

void initialize_render_2D() {

    create_shader_program(&shader2D_program, vertex2D, fragment2D);
}

Mesh create_sprite_mesh_from_matrix(int sample_x, int sample_y, int sample_w, int sample_h, float anchor_u, float anchor_v, Texture texture) {

	const float sample_u1 = sample_x              / (float) ((TextureGL *) texture)->w;
	const float sample_v1 = sample_y              / (float) ((TextureGL *) texture)->h;
	const float sample_u2 = (sample_x + sample_w) / (float) ((TextureGL *) texture)->w;
	const float sample_v2 = (sample_y + sample_h) / (float) ((TextureGL *) texture)->h;

	// sprite mesh position data is in PIXEL space, with 0,0 being the anchor
	const float mesh_data[] = {
		sample_w *      - anchor_u , sample_h *      - anchor_v , sample_u1, sample_v1,
		sample_w * (1.0 - anchor_u), sample_h *      - anchor_v , sample_u2, sample_v1,
		sample_w *      - anchor_u , sample_h * (1.0 - anchor_v), sample_u1, sample_v2,
		sample_w * (1.0 - anchor_u), sample_h *      - anchor_v , sample_u2, sample_v1,
		sample_w * (1.0 - anchor_u), sample_h * (1.0 - anchor_v), sample_u2, sample_v2,
		sample_w *      - anchor_u , sample_h * (1.0 - anchor_v), sample_u1, sample_v2
	};

	static const int mesh_vertcount = 6;
	static const int mesh_bytecount = sizeof(float) * 4 * mesh_vertcount;

	// make vertex array
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// make vertex buffer (stored by vertex_array)
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);								// make it the active buffer
	glBufferData(GL_ARRAY_BUFFER, mesh_bytecount, mesh_data, GL_STATIC_DRAW);	// copy vertex data into the active buffer

	// link active vertex data and shader attributes
	GLint pos_attrib = glGetAttribLocation(shader2D_program, "position");
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glEnableVertexAttribArray(pos_attrib); // requires a VAO to be bound

	GLint uv_attrib = glGetAttribLocation(shader2D_program, "UV");
	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *) (sizeof(float) * 2));
	glEnableVertexAttribArray(uv_attrib);

	// debind vertex array
	glBindVertexArray(0);

	// create final mesh object to return
	MeshGL *mesh_gl = malloc(sizeof(MeshGL));
	mesh_gl->vertex_array = vertex_array;
	mesh_gl->vertex_count = mesh_vertcount;
	mesh_gl->texture_gl = (TextureGL *) texture;

	return mesh_gl;
}

Mesh create_sprite_mesh(float anchor_u, float anchor_v, Texture texture) {
	
	create_sprite_mesh_from_matrix(0, 0, ((TextureGL *) texture)->w, ((TextureGL *) texture)->h, anchor_u, anchor_v, texture);
}

void draw_sprite_mesh(const Mesh mesh, float screen_u, float screen_v, int pixel_dx, int pixel_dy) {

	/*
	* before drawing, a position matrix is calculated that scales the pixel
	* space to UV space (using the GUI_SCALE and current window size) and
	* applies the UV and pixel anchor translations.
	*/
	GLfloat pixel_translation_matrix[4][4] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{pixel_dx, pixel_dy, 0, 1}
	};

	GLfloat scale_matrix[4][4] = {
		{GUI_SCALE * 2.0 / get_window_width(), 0, 0, 0},
		{0, GUI_SCALE * 2.0 / get_window_height(), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};

	GLfloat screen_translation_matrix[4][4] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{screen_u, screen_v, 0, 1}
	};

	// final position matrix
	GLfloat position_matrix[4][4];

	mat4_mult(screen_translation_matrix, scale_matrix, position_matrix);
	mat4_mult(position_matrix, pixel_translation_matrix, position_matrix);

	// bind the mesh's vertex mesh and texture
	glBindVertexArray(((MeshGL *) mesh)->vertex_array);
	glBindTexture(GL_TEXTURE_2D, ((MeshGL *) mesh)->texture_gl->obj);

	// load the 2D shader program with the uniform we just calculated
	glUseProgram(shader2D_program);
	glUniformMatrix4fv(glGetUniformLocation(shader2D_program, "position_matrix"), 1, GL_FALSE, &position_matrix[0][0]);

	// draw
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, ((MeshGL *) mesh)->vertex_count);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}