#include "context.h"
#include "render.h"

// hardcoded values of near plane = 0.01, far plane = 100
#define PROJ_MATRIX (GLfloat[][4]) {\
    { get_window_height() / (float) get_window_width() / tan(FOV * M_PI / 360.0), 0, 0, 0 },\
    { 0, 1.0 / tan(FOV * M_PI / 360.0), 0, 0 },\
    { 0, 0, -1.00020002, -1.0 },\
    { 0, 0, -0.0200020002, 0 }\
}

static const char *vertex3D =
"#version 150 core\n"
"uniform mat4 position_matrix;\n"
"uniform mat4 normal_matrix;\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec2 UV;\n"
"out vec3 normal_camera;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
    "gl_Position = position_matrix * vec4(position.xyz, 1.0);\n" // get final position
    "normal_camera = (normal_matrix * vec4(normal, 1.0)).xyz;\n" // get final normal
    "frag_UV = UV;\n" // pass along UV
"}";

static const char *fragment3D =
"#version 150 core\n"
"uniform sampler2D tex;\n"
"in vec3 normal_camera;\n"
"in vec2 frag_UV;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"float c = dot(normal_camera, vec3(0.436, 0.9, 0)) * 0.3 + 0.7;\n"
	"outColor = texture(tex, frag_UV) * vec4(c, c, c, 1.0);\n"
	"if (outColor.a == 0) { discard; }" // texture clip transparency
"}";

static const char *vertex3DIcon =
"#version 150 core\n"
"uniform mat4 position_matrix;\n"
"in vec3 position;\n"
"in vec3 normal;\n"
"in vec2 UV;\n"
"out vec3 normal_camera;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
    "gl_Position = position_matrix * vec4(position.xyz, 1.0);\n" // get final position
    "normal_camera = normal;\n" // pass along normal
    "frag_UV = UV;\n" // pass along UV
"}";

static const char *fragment3DIcon =
"#version 150 core\n"
"uniform sampler2D tex;\n"
"in vec3 normal_camera;\n"
"in vec2 frag_UV;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"float c = dot(normal_camera, vec3(0.436, 0.9, 0)) * 0.3 + 0.7;\n"
	"outColor = texture(tex, frag_UV) * vec4(c, c, c, 1.0);\n"
	"if (outColor.a == 0) { discard; }" // texture clip transparency
"}";

static GLuint shader3D_program;
static GLuint shader3DIcon_program;

void initialize_render_3D() {

    create_shader_program(&shader3D_program, vertex3D, fragment3D);
	create_shader_program(&shader3DIcon_program, vertex3DIcon, fragment3DIcon);
}

void remesh_mesh(Mesh mesh, const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount) {

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
	GLint pos_attrib = glGetAttribLocation(shader3D_program, "position");
	glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
	glEnableVertexAttribArray(pos_attrib); // requires a VAO to be bound

	GLint normal_attrib = glGetAttribLocation(shader3D_program, "normal");
	glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid *) (sizeof(float) * 3));
	glEnableVertexAttribArray(normal_attrib);

	GLint uv_attrib = glGetAttribLocation(shader3D_program, "UV");
	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (GLvoid *) (sizeof(float) * 6));
	glEnableVertexAttribArray(uv_attrib);

	// debind vertex array
	glBindVertexArray(0);

	// assign to mesh, freeing previous VAO if it exists
	MeshGL *mesh_gl = (MeshGL *) mesh;

	if (mesh_gl->vertex_array)
		glDeleteVertexArrays(1, &mesh_gl->vertex_array);

	mesh_gl->vertex_array = vertex_array;
	mesh_gl->vertex_count = mesh_vertcount;
}

Mesh create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, Texture texture) {

	Mesh mesh = malloc(sizeof(MeshGL));

	remesh_mesh(mesh, mesh_data, mesh_bytecount, mesh_vertcount);	
	mesh_set_texture(mesh, texture);

	return mesh;
}

Mesh create_mesh_from_obj(const char *obj_path, Texture texture) {

	// read obj file
	FILE *file = fopen(obj_path, "r");

	if (file == NULL) {
		printf("File \"%s\" does not exist!\n", obj_path);
		exit(1);
	}

	char line[1024];

	EZArray position_data  = {0};
	EZArray normal_data    = {0};
	EZArray texture_data   = {0};
	EZArray composite_data = {0}; // stores combined vertex position, normal, and texture data

	int vertex_count;

	while (fgets(line, 1024, file)) {

		char prefix[8];

		sscanf(line, "%s", prefix);

		if (!strcmp(prefix, "v")) {

			float v[3];
			
			sscanf(line, "v %f %f %f", &v[0], &v[1], &v[2]);

			// obj vertices have reverse xz
			v[0] = -v[0];
			v[2] = -v[2];

			append_ezarray(&position_data, v, sizeof(float) * 3);
		}

		else if (!strcmp(prefix, "vn")) {

			float n[3];
			
			sscanf(line, "vn %f %f %f", &n[0], &n[1], &n[2]);

			// obj vertices have reverse xz
			n[0] = -n[0];
			n[2] = -n[2];

			append_ezarray(&normal_data, n, sizeof(float) * 3);
		}

		else if (!strcmp(prefix, "vt")) {

			float n[2];
			
			sscanf(line, "vt %f %f", &n[0], &n[1]);

			append_ezarray(&texture_data, n, sizeof(float) * 2);
		}

		else if (!strcmp(prefix, "f")) {

			// only works with tris right now (no quads or ngons)

			unsigned int p[3]; // vertex position indices
			unsigned int t[3]; // vertex texture coordinate indices
			unsigned int n[3]; // vertex normal indices
			
			sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u",
				&p[0], &t[0], &n[0],
				&p[1], &t[1], &n[1],
				&p[2], &t[2], &n[2]);

			// convert vertex indices to vertex positions (indices start at 1 for some reason)
			append_ezarray(&composite_data, position_data.data + ((p[0] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, normal_data.data +   ((n[0] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, texture_data.data +  ((t[0] - 1) * sizeof(float) * 2), sizeof(float) * 2);

			append_ezarray(&composite_data, position_data.data + ((p[1] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, normal_data.data +   ((n[1] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, texture_data.data +  ((t[1] - 1) * sizeof(float) * 2), sizeof(float) * 2);

			append_ezarray(&composite_data, position_data.data + ((p[2] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, normal_data.data +   ((n[2] - 1) * sizeof(float) * 3), sizeof(float) * 3);
			append_ezarray(&composite_data, texture_data.data +  ((t[2] - 1) * sizeof(float) * 2), sizeof(float) * 2);
			
			vertex_count += 3;
		}
	}

	fclose(file);

	// create mesh
	return create_mesh(composite_data.data, composite_data.bytecount, vertex_count, texture);
}

void draw_mesh(const Transform *camera, const Transform *transform, const Mesh mesh) {

	// shared buffers
	GLfloat pitch_matrix[4][4];
	GLfloat yaw_matrix[4][4];

	GLfloat position_matrix[4][4];

	// bind the mesh's vertex mesh and texture
	glBindVertexArray(((MeshGL *) mesh)->vertex_array);
	glBindTexture(GL_TEXTURE_2D, ((MeshGL *) mesh)->texture_gl->obj);

	// model matrix (converts from model space to world space)
	generate_rotation_matrices(
		pitch_matrix, transform->pitch,
		yaw_matrix, transform->yaw
	);

	GLfloat model_matrix[4][4];

	mat4_mult(yaw_matrix, pitch_matrix, model_matrix); // rotation

	model_matrix[3][0] = transform->x; // translation
	model_matrix[3][1] = transform->y;
	model_matrix[3][2] = transform->z;

	// view matrix (converts from world space to view space, aka accounts for camera transformations)
	// must apply translations before rotations this time, unlike model matrix!
	generate_rotation_matrices(
		pitch_matrix, -camera->pitch,
		yaw_matrix, -camera->yaw
	);

	GLfloat view_matrix[4][4] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{-camera->x, -camera->y, -camera->z, 1}
	};

	mat4_mult(yaw_matrix, view_matrix, view_matrix);
	mat4_mult(pitch_matrix, view_matrix, view_matrix);

	// final position matrix (PROJ_MATRIX * view_matrix * model_matrix)
	mat4_mult(PROJ_MATRIX, view_matrix, position_matrix);
	mat4_mult(position_matrix, model_matrix, position_matrix);

	// normal matrix (applied to normals to account for mesh rotation)
	GLfloat normal_matrix[4][4];

	generate_rotation_matrices(
		pitch_matrix, -transform->pitch,
		yaw_matrix, -transform->yaw
	);

	mat4_mult(yaw_matrix, pitch_matrix, normal_matrix);

	// load the 3D shader program and the uniforms we just calculated
	glUseProgram(shader3D_program);
	glUniformMatrix4fv(glGetUniformLocation(shader3D_program, "position_matrix"), 1, GL_FALSE, &position_matrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader3D_program, "normal_matrix"), 1, GL_FALSE, &normal_matrix[0][0]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, ((MeshGL *) mesh)->vertex_count);
}

void draw_mesh_as_icon(const Mesh mesh, float screen_u, float screen_v, int pixel_dx, int pixel_dy) {

	// bind the mesh's vertex mesh and texture
	glBindVertexArray(((MeshGL *) mesh)->vertex_array);
	glBindTexture(GL_TEXTURE_2D, ((MeshGL *) mesh)->texture_gl->obj);

	// model matrix (makes the model really tiny and just a little bit away from the camera
	// because I can't disable depth testing and I don't want to create a separate buffer
	// to render to first)
	GLfloat model_matrix[4][4] = {
		{0.01, 0, 0, 0},
		{0, 0.01, 0, 0},
		{0, 0, 0.01, 0},
		{0, 0, -0.2, 1}
	};

	// scale matrix (scales the model after projection, so that scaling doesn't effect perspective)
	float scale = 1000.0 / get_window_height();
	
	GLfloat scale_matrix[4][4] = {
		{scale, 0, 0, 0},
		{0, scale, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};

	// final position matrix
	GLfloat position_matrix[4][4];

	mat4_mult(scale_matrix, PROJ_MATRIX, position_matrix);
	mat4_mult(position_matrix, model_matrix, position_matrix);
	mat4_translate(position_matrix, screen_u + pixel_dx * GUI_SCALE * 2.0 / get_window_width(), screen_v + pixel_dy * GUI_SCALE * 2.0 / get_window_height(), 0.0);

	// load the 3D shader program with the uniform we just calculated
	glUseProgram(shader3DIcon_program);
	glUniformMatrix4fv(glGetUniformLocation(shader3DIcon_program, "position_matrix"), 1, GL_FALSE, &position_matrix[0][0]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, ((MeshGL *) mesh)->vertex_count);
}