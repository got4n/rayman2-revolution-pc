#version 450

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform sampler2D u_frame;

layout(push_constant) uniform Present {
    int mode;
    float view_w;
    float view_h;
    int post;
} u;

ivec2 size;
vec2 view_size() { return vec2(u.view_w, u.view_h); }

vec3 texel(ivec2 p) {
    p = clamp(p, ivec2(0), size - 1);
    p.y = size.y - 1 - p.y;
    return texelFetch(u_frame, p, 0).rgb * 255.0;
}

vec3 bilinear(vec2 at) {
    vec2 q = at - 0.5;
    ivec2 i = ivec2(floor(q));
    vec2 f = q - floor(q);
    return mix(mix(texel(i), texel(i + ivec2(1, 0)), f.x), mix(texel(i + ivec2(0, 1)), texel(i + ivec2(1, 1)), f.x), f.y);
}
vec4 catmull(float t) {
    float t2 = t * t, t3 = t2 * t;
    return vec4(-0.5 * t3 + t2 - 0.5 * t, 1.5 * t3 - 2.5 * t2 + 1.0, -1.5 * t3 + 2.0 * t2 + 0.5 * t, 0.5 * t3 - 0.5 * t2);
}
vec3 bicubic(vec2 at) {
    vec2 q = at - 0.5;
    ivec2 i = ivec2(floor(q));
    vec2 f = q - floor(q);
    vec4 wx = catmull(f.x), wy = catmull(f.y);
    vec3 c = vec3(0.0);
    for (int y = 0; y < 4; y++) {
        vec3 row = vec3(0.0);
        for (int x = 0; x < 4; x++) row += texel(i + ivec2(x - 1, y - 1)) * wx[x];
        c += row * wy[y];
    }
    return clamp(c, 0.0, 255.0);
}
vec3 area(vec2 at, vec2 foot) {
    vec2 lo = at - max(foot, vec2(1.0)) * 0.5, hi = at + max(foot, vec2(1.0)) * 0.5;
    ivec2 a = ivec2(floor(lo)), b = min(ivec2(ceil(hi)) - 1, a + 15);
    vec3 sum = vec3(0.0);
    float weight = 0.0;
    for (int y = a.y; y <= b.y; y++)
        for (int x = a.x; x <= b.x; x++) {
            vec2 w = max(min(vec2(x + 1, y + 1), hi) - max(vec2(x, y), lo), vec2(0.0));
            sum += texel(ivec2(x, y)) * (w.x * w.y);
            weight += w.x * w.y;
        }
    return sum / max(weight, 1e-6);
}
vec3 scaled(vec2 at, vec2 foot, int mode) {
    if (mode == 0) return texel(ivec2(at));
    if (foot.x > 1.0 || foot.y > 1.0) return area(at, foot);
    return mode == 2 ? bicubic(at) : bilinear(at);
}
float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

/* FXAA: along the edges the 2x2 around finds, a blend of the scene. */
vec3 fxaa(vec3 c, vec2 at, vec2 foot) {
    vec2 d = max(foot, vec2(1.0));
    float nw = luma(bilinear(at + vec2(-d.x, -d.y))), ne = luma(bilinear(at + vec2(d.x, -d.y)));
    float sw = luma(bilinear(at + vec2(-d.x, d.y))), se = luma(bilinear(at + d)), m = luma(c);
    float lo = min(m, min(min(nw, ne), min(sw, se))), hi = max(m, max(max(nw, ne), max(sw, se)));
    if (hi - lo < max(8.0, hi * 0.125)) return c;
    vec2 dir = vec2(-((nw + ne) - (sw + se)), (nw + sw) - (ne + se));
    float reduce = max((nw + ne + sw + se) * 0.25 * 0.125, 2.0);
    dir = clamp(dir / (min(abs(dir.x), abs(dir.y)) + reduce), vec2(-8.0), vec2(8.0)) * d;
    vec3 a = 0.5 * (bilinear(at + dir * (1.0 / 3.0 - 0.5)) + bilinear(at + dir * (2.0 / 3.0 - 0.5)));
    vec3 b = a * 0.5 + 0.25 * (bilinear(at - dir * 0.5) + bilinear(at + dir * 0.5));
    float l = luma(b);
    return l < lo || l > hi ? a : b;
}

/* Vivid colours: more saturation and a little contrast. */
vec3 vivid(vec3 c) {
    vec3 x = c / 255.0;
    x = clamp(mix(vec3(luma(x)), x, 1.3), 0.0, 1.0);
    return mix(x, x * x * (3.0 - 2.0 * x), 0.35) * 255.0;
}

/* Composite TV: the colour bleeds sideways much more than the brightness, as on a composite cable. */
const mat3 to_yiq = mat3(0.299, 0.596, 0.211, 0.587, -0.274, -0.523, 0.114, -0.322, 0.312);
const mat3 to_rgb = mat3(1.0, 1.0, 1.0, 0.956, -0.272, -1.106, 0.621, -0.647, 1.703);
vec3 composite(vec2 at) {
    float pitch = max(float(size.x) / 640.0, 1.0);
    float y = 0.0;
    vec2 iq = vec2(0.0);
    const float luma_w[3] = float[](0.25, 0.5, 0.25);
    for (int k = -1; k <= 1; k++) y += (to_yiq * bilinear(at + vec2(float(k) * pitch, 0.0))).x * luma_w[k + 1];
    const float chroma_w[7] = float[](0.06, 0.1, 0.18, 0.32, 0.18, 0.1, 0.06);
    for (int k = -3; k <= 3; k++) iq += (to_yiq * bilinear(at + vec2(float(k) * pitch * 1.5, 0.0))).yz * chroma_w[k + 3];
    return clamp(to_rgb * vec3(y, iq * 0.9), 0.0, 255.0);
}

/* The curved CRT's screen: a barrel, outside which is black. */
vec2 curve(vec2 uv) {
    vec2 c = uv * 2.0 - 1.0;
    c *= 1.0 + vec2(c.y * c.y * 0.045, c.x * c.x * 0.06);
    return c * 0.5 + 0.5;
}

vec3 crt(vec3 c, vec2 uv, float depth) {
    float line = fract(uv.y * 448.0);
    float scan = (1.0 - depth) + depth * sin(line * 3.14159265);
    float column = mod(floor(uv.x * view_size().x), 3.0);
    vec3 mask = column < 1.0 ? vec3(1.0, 0.85, 0.85) : column < 2.0 ? vec3(0.85, 1.0, 0.85) : vec3(0.85, 0.85, 1.0);
    vec2 e = uv * (1.0 - uv);
    float vignette = clamp(pow(e.x * e.y * 16.0, 0.18), 0.0, 1.0);
    return clamp(c * scan * mask * vignette * (1.0 + depth * 0.47), 0.0, 255.0);
}

vec3 post_effect(vec3 c, vec2 at, vec2 foot, vec2 uv, int post) {
    if (post == 1) {
        vec2 d = max(foot, vec2(1.0));
        vec3 around = bilinear(at + vec2(d.x, 0.0)) + bilinear(at - vec2(d.x, 0.0)) + bilinear(at + vec2(0.0, d.y)) +
                      bilinear(at - vec2(0.0, d.y));
        return clamp(c + (c * 4.0 - around) * 0.25, 0.0, 255.0);
    }
    if (post == 2 || post == 5) return crt(c, uv, 0.38);
    if (post == 3) return fxaa(c, at, foot);
    if (post == 4) return vivid(c);
    if (post == 6) return crt(composite(at), uv, 0.2);
    return c;
}

void main() {
    size = textureSize(u_frame, 0);
    vec2 uv = v_uv;
    if (u.post == 5) {
        uv = curve(v_uv);
        if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
            o_color = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
    }
    vec2 at = uv * vec2(size);
    vec2 foot = vec2(size) / view_size();
    vec3 c = scaled(at, foot, u.mode);
    c = post_effect(c, at, foot, vec2(uv.x, 1.0 - uv.y), u.post);
    o_color = vec4(c / 255.0, 1.0);
}
