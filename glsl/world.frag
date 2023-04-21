#version 330 core

in vec3 f_normal;
in vec3 f_position;
in float f_scale;
flat in ivec2 f_typeCoords;
in vec2 f_texCoords;

layout(location = 0) out vec3 o_color;

uniform float u_vary_color;
uniform float u_h_base;
uniform float u_t;
uniform vec3 u_camera_pos;
uniform sampler2D s_atlas;

const vec2 dTex = vec2(0.0625, 0.0625);

//const float h_base = 0.7;

// Thanks to sam at http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl (May 19, 2015).
const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
vec3 hsv2rgb(vec3 c){
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    vec3 rgb = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    return rgb;
}

void main() {
    float scale = 8e-3 / f_scale;
    vec3 scaled_position = f_position * vec3(scale, scale, scale);
//    float other = dot(scaled_position, f_normal);
//    vec3 dcenter = f_position - u_center;
//    float ddcenter2 = dot(dcenter, dcenter);
//    float h = fract(dot(scaled_position, vec3(1., 1., 1.)) * u_t + 0.95 * sin(0.5 * (scaled_position.x + abs(dot(scaled_position, vec3(1., 1., 1.)))) * u_t)) + 0.25 * sin(2 * dcenter.y + 0.5 * ddcenter2 * dcenter.x * u_t);
    float h = fract(0.0 + u_h_base + u_vary_color * (0.06 * sin(u_t) + sqrt(dot(scaled_position, scaled_position))));
    float s = 0.7;

    vec2 tc = vec2(f_typeCoords) * dTex + f_texCoords;

    // Compute brightness.
    vec3 dposition = u_camera_pos - f_position;
    float costheta = clamp(dot(f_normal, normalize(dposition)), 0., 1.);
    float v = 0.9*(0.1 + 0.9 * costheta);
    float i;
	o_color = hsv2rgb(vec3(modf(h, i), s, v)) * texture(s_atlas, tc).rgb;
}
