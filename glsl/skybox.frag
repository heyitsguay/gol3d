#version 330 core

in vec3 f_texCoords;

out vec4 o_color;

uniform samplerCube s_skybox;

void main() {
	o_color = texture(s_skybox, f_texCoords);
}
