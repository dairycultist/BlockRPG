#include "context.h"
#include "render.h"

static const char *vertex2D =
"#version 150 core\n"
"in vec2 position;\n"
"in vec2 UV;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
    "gl_Position = vec4(position.xy, -1.0, 1.0);\n" // get final position
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

Mesh create_sprite_mesh_from_matrix(int x, int y, int width, int height, float u, float v, float anchor_u, float anchor_v, int h_pixels, Texture texture) {

	// I know I know, the variable names here suck fat nuts

	const float w = h_pixels / (float) WINDOW_WIDTH * width / height;
	const float h = h_pixels / (float) WINDOW_HEIGHT;

	u -= anchor_u * w;
	v -= anchor_v * h;

	float sample_u1 = x            / (float) ((TextureGL *) texture)->w;
	float sample_v1 = y            / (float) ((TextureGL *) texture)->h;
	float sample_u2 = (x + width)  / (float) ((TextureGL *) texture)->w;
	float sample_v2 = (y + height) / (float) ((TextureGL *) texture)->h;

	const float mesh_data[] = {
		u,     v,     sample_u1, sample_v1,
		u + w, v,     sample_u2, sample_v1,
		u,     v + h, sample_u1, sample_v2,
		u + w, v,     sample_u2, sample_v1,
		u + w, v + h, sample_u2, sample_v2,
		u,     v + h, sample_u1, sample_v2
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

Mesh create_sprite_mesh(float u, float v, float anchor_u, float anchor_v, int h_pixels, Texture texture) {
	
	create_sprite_mesh_from_matrix(0, 0, ((TextureGL *) texture)->w, ((TextureGL *) texture)->h, u, v, anchor_u, anchor_v, h_pixels, texture);
}

void draw_sprite_mesh(const Mesh mesh) {

	// bind the mesh's vertex mesh and texture
	glBindVertexArray(((MeshGL *) mesh)->vertex_array);
	glBindTexture(GL_TEXTURE_2D, ((MeshGL *) mesh)->texture_gl->obj);

	// load the 2D shader program
	glUseProgram(shader2D_program);

	// draw
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, ((MeshGL *) mesh)->vertex_count);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}