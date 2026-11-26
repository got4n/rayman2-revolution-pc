#define SAMPLERS 40

layout(set = 0, binding = 0) uniform sampler u_samplers[SAMPLERS];

layout(set = 2, binding = 2, std140) uniform Draw {
    vec4 view;
    vec4 fog_color;
    int textured;
    int texture_alpha;
    int flat_color;
    int fog;
    int alpha_mode;
    float alpha_ref;
    int sampler_index;
    int pad0;
    vec4 projection;
    vec4 projection2;
} u;

vec4 clip_position(vec3 p) {
    if (u.projection2.x == 0.0)
        return vec4((p.xy + u.view.zw) / u.view.xy * 2.0 - 1.0, clamp(p.z, 0.0, 65535.0) / 65535.0, 1.0);
    vec2 screen_z = vec2(320.0, 224.0) * (p.z - p.xy / u.projection.xy);
    vec2 xy = (screen_z + u.view.zw * p.z) / u.view.xy * 2.0 - p.z;
    return vec4(xy, (u.projection.z + u.projection.w * p.z) / 65535.0, p.z);
}

float near_distance(vec3 p) { return u.projection2.x == 0.0 ? 1.0 : p.z - u.projection2.y; }

vec4 sample_texture(texture2D tex, vec2 uv) {
    return texture(sampler2D(tex, u_samplers[u.sampler_index]), uv);
}
