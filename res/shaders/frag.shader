#version 410 core

out vec4 FragColor;
in vec2 outColor;
in vec2 outTexcoord;

uniform sampler2D image;

void main() {
    FragColor = texture(image, outTexcoord) * vec4(outColor, 0, 1);
}