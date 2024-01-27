#version 330 core

layout(location = 0) in vec2 vertex;

layout(location = 1) in mat4 transform;

layout(location = 5) in vec4 texoff;
layout(location = 6) in vec4 tint;

out vec2 coords;

out vec4 Tint;

uniform float time;

uniform bool Shake;
uniform bool Invert;

void main() {
	vec2 adjVertex = vertex + vec2(0.5, 0.5);

	gl_Position = transform * vec4(vertex, 0.0, 1.0);

	if (Shake) {
		gl_Position.x += cos(time * 21) * 0.02;
		gl_Position.y += cos(time * 23) * 0.02;
	}

	coords = (adjVertex * texoff.zw) + texoff.xy;

	if (Invert) {
		coords = vec2(1.0 - coords.x, 1.0 - coords.y);
	}

	Tint = tint;
}