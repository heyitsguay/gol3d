#version 330 core

in vec3 f_texCoords;

layout(location = 0) out vec4 o_color;

uniform samplerCube s_skybox;

// Thanks to sam at http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl (May 19, 2015).
const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
vec3 hsv2rgb(vec3 c){
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    vec3 rgb = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    return rgb;
}

// Color of the skybox in HSV coordinates.
const vec3 color = vec3(0.5, 0.25, 0.8);
vec4 colorRGB = vec4(hsv2rgb(color), 1.);

void main() {
	o_color = colorRGB * texture(s_skybox, f_texCoords);
}
