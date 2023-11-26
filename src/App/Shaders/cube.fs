#version 330 core

in vec3 normal;
in vec3 position;
in vec2 texcoord;

uniform sampler2D tex;
uniform vec3 sun_coord;
uniform bool directional;

out vec4 color;

void main() {
    if (directional) {
        // TODO: fix sun position
        float cos = max(dot(normal, normalize(sun_coord)), 0.0);
        color = texture(tex, texcoord) * cos;
    } else {
        color = texture(tex, texcoord);
    }
}