#version 330 core

in vec3 normal;
in vec3 position;
in vec2 texcoord;
in vec3 sun;
in vec3 lightDirection;
in vec3 spotDirection;

uniform sampler2D tex;
uniform bool directional;
uniform bool spot;
// uniform vec3 spot_position;
//uniform vec3 spot_direction; 

out vec4 color;

void main() {
    
    vec4 color_dir = vec4(0, 0, 0, 0);
    vec4 color_spot = vec4(0, 0, 0, 0);

    color = texture(tex, texcoord) * 0.08;          // TODO: ambient

    if (!spot && !directional) {
        return;
    }

    if (directional) {
        float cos = dot(normal, normalize(sun));
        color_dir = cos > 0.0 ? texture(tex, texcoord) * cos : color_dir;
    } 
    
    if (spot) {
        float cos = max(dot(normalize(lightDirection), normalize(spotDirection)), 0.0);
        float angle = acos(cos);
        color_spot = angle < radians(20.0) ? texture(tex, texcoord) * cos : color_spot;
    }

    color += color_dir;
    color += color_spot;   
}