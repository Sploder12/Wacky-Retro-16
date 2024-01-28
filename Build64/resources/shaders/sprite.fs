#version 330 core

in vec2 texCoords;

uniform sampler2D atlas;

in vec4 Tint;

out vec4 FragColor;

void main() {
	FragColor = texture(atlas, texCoords) * Tint;
}