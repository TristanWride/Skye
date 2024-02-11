#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 colour;

out vec3 outColour;

void main() {
    gl_Position = vec4(pos, 1.0);
    outColour = colour;
}