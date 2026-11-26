#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec4 v_color;
layout(location = 1) flat in vec4 v_flat;
layout(location = 2) in vec3 v_stq;
layout(location = 3) in float v_fog;

layout(location = 0) out vec4 o_color;

layout(set = 1, binding = 0) uniform texture2D u_tex;

#include "state.glsl"

void main() {
    vec4 c = u.flat_color != 0 ? v_flat : v_color;
    if (u.textured != 0) {
        vec4 t = sample_texture(u_tex, v_stq.xy / v_stq.z);
        if (u.textured == 1) c = vec4(t.rgb * c.rgb, u.texture_alpha != 0 ? t.a * c.a : c.a);
        else c = vec4(t.rgb, u.texture_alpha != 0 ? t.a : c.a);
    } else {
        c.rgb *= 128.0 / 255.0;
    }
    if (u.alpha_mode == 1 && c.a < u.alpha_ref) discard;
    if (u.alpha_mode == 2 && c.a >= u.alpha_ref) discard;
    c.rgb = min(c.rgb, 1.0);
    if (u.fog != 0) c.rgb = mix(u.fog_color.rgb, c.rgb, v_fog);
    o_color = vec4(c.rgb, clamp(c.a, 0.0, 1.0));
}
