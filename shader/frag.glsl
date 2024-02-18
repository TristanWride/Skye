#version 460 core
in vec3 outColour;

out vec4 FragColor;

void main() {
    FragColor = vec4((outColour + 1.0) / 2.0, 1.0f);
}