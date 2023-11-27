#version 330 core

in vec3 normal;
in vec3 position;
in vec2 texcoord;
in vec3 sun;

uniform sampler2D tex;
uniform bool directional;
uniform bool spot;
uniform vec3 spot_position;
uniform vec3 spot_direction; 

out vec4 color;

void main() {

    if (!spot && ! directional) {
        color = texture(tex, texcoord);
        return;
    }

    if (directional) {
        float cos = dot(normal, normalize(sun));
        color = cos > 0.0 ? texture(tex, texcoord) * cos : texture(tex, texcoord) * 0.08;    // TODO: ambient
    } 
    
    if (spot) {
        // как считает косинус??
        vec3 lightDirection = position - spot_position;
        float cos = dot(normalize(spot_direction), normalize(position - spot_position));
        float angle = acos(cos);
        color = angle > radians(20.0) ? color = texture(tex, texcoord) * 0.08 : color = texture(tex, texcoord) * cos;   // TODO: ambient
    }   
}