#version 330 core

// Input attribute arrays.
layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec3 inst_translation;
layout(location = 3) in float inst_scale;
layout(location = 4) in ivec2 inst_typeCoords;
layout(location = 5) in vec2 i_texCoords;

// Output data.
out vec3 f_normal;
out vec3 f_position;
out float f_scale;
flat out ivec2 f_typeCoords;
out vec2 f_texCoords;

//uniform mat4 u_M;
uniform mat4 u_MVP;

void main() {
//    f_position = (u_M * vec4(i_position, 1)).xyz;
    f_position = inst_scale * i_position + inst_translation;
    gl_Position = u_MVP * vec4(f_position, 1);
    f_normal = i_normal;
    f_scale = inst_scale;
    f_typeCoords = inst_typeCoords;
    f_texCoords = i_texCoords;
}