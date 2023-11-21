#version 330 core

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 ViewProjMat;
uniform mat4 ModelMat;
uniform mat4 normalMV;

out vec3 normal;
out vec3 position;
out vec2 texcoord;

void main() {
    vec4 vertex;
    vertex = vec4(in_vertex, 1);

	float prev_x = vertex.x;
	float prev_y = vertex.y;
	float prev_z = vertex.z;

	float prev_x_square = prev_x * prev_x;
	float prev_y_square = prev_y * prev_y;
	float prev_z_square = prev_z * prev_z;

  	vertex.x = prev_x * sqrt(1 - prev_y_square / 2 - prev_z_square / 2 + prev_y_square * prev_z_square / 3);
	vertex.y = prev_y * sqrt(1 - prev_z_square / 2 - prev_x_square / 2 + prev_x_square * prev_z_square / 3);
	vertex.z = prev_z * sqrt(1 - prev_x_square / 2 - prev_y_square / 2 + prev_x_square * prev_y_square / 3);

    gl_Position = ViewProjMat * ModelMat * vertex;
    normal = normalize(mat3(normalMV) * in_normal);
    // normal = in_normal;
    position = in_vertex;
    texcoord = in_texcoord;
}