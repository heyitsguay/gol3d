#version 330 core

layout(location = 0) in vec3 i_position;

out vec3 f_texCoords;

uniform mat4 u_MVP;

void main() {
	vec4 pos = u_MVP * vec4(i_position, 1.);
	gl_Position = pos.xyww;
	f_texCoords = i_position;
}
