#version 300 es

precision mediump float;
precision mediump int;

layout(location = 0) in vec2 vertex;

layout(location = 1) in mat4 transform;

layout(location = 5) in vec4 texoff;
layout(location = 6) in vec4 tint;

out vec2 texCoords;

out vec4 Tint;

uniform float time;

void main() {
	vec2 adjVertex = vertex + vec2(0.5, 0.5);

	gl_Position = transform * vec4(vertex, 0.0, 1.0);

	texCoords = (adjVertex * texoff.zw) + texoff.xy;
	Tint = tint;
}