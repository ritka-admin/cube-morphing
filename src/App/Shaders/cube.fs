#version 330 core

in vec3 normal;
in vec3 position;
in vec2 texcoord;

uniform sampler2D tex;
uniform vec3 sun_coord;

out vec4 color;

void main() {
    float cos = max(dot(normal, normalize(sun_coord)), 0.0);
    color = texture(tex, texcoord) * cos;
}