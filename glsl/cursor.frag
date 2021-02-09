#version 330 core

// Output color.
layout(location = 0) out vec4 o_color;

// Indicates which color to use.
uniform int u_colorState;

const vec4 color0 = vec4(0.8, 0.9, 1., 0.2);
const vec4 color1 = vec4(0.8, 0.55, 0.5, 0.2);
const vec4 color2 = vec4(0.55, 0.8, 0.5, 0.2);
const vec4 color3 = vec4(0.5, 0.55, 0.8, 0.2);

void main() {
	o_color = color0 * float(u_colorState == 0) + color1 * float(u_colorState == 1) +
	          color2 * float(u_colorState == 2) + color3 * float(u_colorState == 3);
}
