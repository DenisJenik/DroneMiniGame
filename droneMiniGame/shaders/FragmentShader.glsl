#version 330 core

// Input from Vertex Shader

in vec3 frag_normal;     // Normal vector
in vec3 frag_position;   // Position in Object Space
in vec2 frag_texCoord;   // Texture coordinates for noise sampling

// Output
layout(location = 0) out vec4 out_color; // Final fragment color

// Random function
float random(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// Noise function
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    // Four corners of the grid cell
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smoothstep interpolation
    vec2 u = f * f * (3.0 - 2.0 * f);

    // Bilinear interpolation
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}


void main() {
    // Frecvență și amplitudine pentru noise
    float frequency = 0.15; // Ajustează detaliile noise-ului
    float noise_value = noise(frag_texCoord * frequency);

    // Culori mai naturale pentru teren
    vec3 color1 = vec3(0.2, 0.5, 0.2); // Verde deschis
    vec3 color2 = vec3(0.5, 0.3, 0.1); // Maro


    // Factor de amestec bazat pe poziție și noise
    float blend_factor = clamp((frag_position.y - 0.0) / 10.0, 0.0, 1.0);
    vec3 final_color = mix(color1, color2, noise_value + blend_factor * 0.5);

    vec3 ambient = 0.2 * color1; // Lumină ambientală
vec3 result = ambient + final_color;
    // Output final
    out_color = vec4(final_color, 1.0);
}
