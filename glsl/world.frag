#version 330 core

in vec3 f_normal;
in vec3 f_position;
in float f_scale;

layout(location = 0) out vec3 o_color;

uniform float u_t;
uniform vec3 u_camera_pos;

//const float h_base = 0.4;

const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);

// Thanks to sam at http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl (May 19, 2015).
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
    float h = fract(0.7 + 1.2 * u_t + sqrt(dot(scaled_position, scaled_position)));
//    float h = 0.5 * (1. + tanh(scaled_position.y));
    float s = 0.7;

    // Compute brightness.
    vec3 dposition = u_camera_pos - f_position;
    float costheta = clamp(dot(f_normal, normalize(dposition)), 0., 1.);
    float v = 0.8*(0.15 + 0.85 * costheta);
	o_color = hsv2rgb(vec3(h, s, v));
}
