#version 330 core

layout(location = 0) in vec3 position; // Vertex position
layout(location = 1) in vec3 normal;   // Vertex normal
layout(location = 2) in vec2 texCoord; // Vertex texture coordinates (for noise input)

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 frag_position; // Position in world space
out vec3 frag_normal;   // Normal vector
out vec2 frag_texCoord; // Texture coordinates for noise sampling

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
    // Apply noise to the Y position to create terrain height
    float frequency = 0.5; // Adjust for the level of detail
    float amplitude = 1.1; // Adjust for the height scale
    float height = noise(position.xz * frequency) * amplitude;

    // Modify the vertex position by adding the height to the Y coordinate
    vec3 updated_position = position;
    updated_position.y += height;

    // Pass updated position, normal, and texture coordinates to the fragment shader
    frag_position = vec3(Model * vec4(updated_position, 1.0));
    frag_normal = normalize(mat3(transpose(inverse(Model))) * normal);
    frag_texCoord = updated_position.xz; // Use updated xz as 2D coordinates for noise

    // Final position in clip space
    gl_Position = Projection * View * Model * vec4(updated_position, 1.0);
}