#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aColor;
layout (location = 2) in vec2 aTexcoord;

out vec2 outColor;
out vec2 outTexcoord;

void main() {
    outColor = aColor;
    outTexcoord = aTexcoord;
    gl_Position = vec4(aPos, 1.0);
}
