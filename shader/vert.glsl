#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 colour;

layout (location = 0) uniform mat4 view;
layout (location = 1) uniform mat4 proj;

out vec3 outColour;

void main() {
    gl_Position = proj * view * vec4(pos, 1.0);
    outColour = colour;
}