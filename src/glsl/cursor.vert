#version 330 core

// Input attribute array.
layout(location = 0) in vec3 i_position;

// Uniform data.
uniform mat4 u_MVP;

void main() {
	gl_Position = u_MVP * vec4(i_position, 1);
}
