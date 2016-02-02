#version 330 core

in vec3 f_texCoords;

out vec4 o_color;

uniform samplerCube s_skybox;

const vec4 color = vec4(0.9922, 1., 0.8196, 1.);

void main() {
	o_color = color * texture(s_skybox, f_texCoords);
}
