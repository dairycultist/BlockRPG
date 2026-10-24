#include "context.h"
#include "render.h"

static const char *vertexSky =
"#version 150 core\n"
"in vec2 position;\n"
"out float height;\n" // [-1, 1]
"void main() {\n"
    "gl_Position = vec4(position.xy, -1.0, 1.0);\n" // get final position
	"height = position.y;\n"
"}";

static const char *fragmentSky =
"#version 150 core\n"
"uniform vec2 rotation;\n" // pitch [-1, 1] yaw [0, 1]
"in float height;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"outColor = mix(vec4(0.4, 0.55, 1.0, 1.0), vec4(0.58, 0.73, 1.0, 1.0), clamp((height * 10.0) * 0.5 + 0.5 + 5.0 * tan(rotation.x * 1.57), 0.0, 1.0));\n"
"}";

static GLuint shaderSky_program;

void initialize_render_sky() {

    create_shader_program(&shaderSky_program, vertexSky, fragmentSky);
}

Mesh create_sky_mesh() {

	const float mesh_data[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f,
	};

	static const int mesh_vertcount = 6;
	static const int mesh_bytecount = sizeof(float) * 2 * mesh_vertcount;

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
	GLint pos_attrib = glGetAttribLocation(shaderSky_program, "position");
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
	glEnableVertexAttribArray(pos_attrib); // requires a VAO to be bound

	// debind vertex array
	glBindVertexArray(0);

	// create final mesh object to return
	MeshGL *mesh_gl = malloc(sizeof(MeshGL));
	mesh_gl->vertex_array = vertex_array;
	mesh_gl->vertex_count = mesh_vertcount;

	return mesh_gl;
}

void draw_sky_mesh(const Transform *camera, const Mesh mesh) {

	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(((MeshGL *) mesh)->vertex_array);
	glUseProgram(shaderSky_program);
	glUniform2f(glGetUniformLocation(shaderSky_program, "rotation"), (GLfloat) -camera->pitch * 2.0 / M_PI, (GLfloat) fmod(fmod(camera->yaw / 2.0 / M_PI, 1.0) + 1.0, 1.0));

	glDrawArrays(GL_TRIANGLES, 0, ((MeshGL *) mesh)->vertex_count);

	glEnable(GL_DEPTH_TEST);
}