#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_stq;
layout(location = 2) in uvec4 a_color;
layout(location = 3) in uvec4 a_flat;
layout(location = 4) in float a_fog;

layout(location = 0) out vec4 v_color;
layout(location = 1) flat out vec4 v_flat;
layout(location = 2) out vec3 v_stq;
layout(location = 3) out float v_fog;

#include "state.glsl"

out float gl_ClipDistance[1];

void main() {
    gl_Position = clip_position(a_pos);
    gl_ClipDistance[0] = near_distance(a_pos);
    v_color = vec4(a_color) / 128.0;
    v_flat = vec4(a_flat) / 128.0;
    v_stq = a_stq;
    v_fog = a_fog / 255.0;
}
