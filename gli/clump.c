#include <stdlib.h>
#include <string.h>

#include "clump.h"
#include "rwstream.h"

static void *take_copy(Reader *r, size_t n) {
    if (n > 0xFFFFFFFFu) return NULL;
    const u8 *p = take(r, (u32)n);
    if (!p) return NULL;
    void *copy = malloc(n ? n : 1);
    memcpy(copy, p, n);
    return copy;
}

static bool take_floats(Reader *r, float *out, int n) {
    const u8 *p = take(r, 4u * (u32)n);
    if (p) memcpy(out, p, 4u * (size_t)n);
    return p != NULL;
}

static bool texture(Reader *r, ClumpMaterial *m) {
    Reader body, head;
    if (!chunk(r, CHUNK_TEXTURE, &body) || !chunk(&body, CHUNK_STRUCT, &head) || !take_u32(&head, &m->filter_addressing))
        return false;
    return string_chunk(&body, m->texture) && string_chunk(&body, m->mask);
}

static bool material(Reader *r, ClumpMaterial *m) {
    Reader body, head;
    if (!chunk(r, CHUNK_MATERIAL, &body) || !chunk(&body, CHUNK_STRUCT, &head)) return false;
    u32 flags, color, unused, textured;
    if (!take_u32(&head, &flags) || !take_u32(&head, &color) || !take_u32(&head, &unused) || !take_u32(&head, &textured))
        return false;
    memcpy(m->color, &color, 4);
    m->ambient = m->specular = m->diffuse = 1.0f;
    float surface[3];
    if (take_floats(&head, surface, 3)) {
        m->ambient = surface[0];
        m->specular = surface[1];
        m->diffuse = surface[2];
    }
    m->textured = textured != 0;
    return !m->textured || texture(&body, m);
}

static bool materials(Reader *r, ClumpGeometry *g) {
    Reader body, head;
    if (!chunk(r, CHUNK_MATERIAL_LIST, &body) || !chunk(&body, CHUNK_STRUCT, &head)) return false;
    u32 count;
    if (!take_u32(&head, &count) || count > 0x10000) return false;
    g->material_count = (int)count;
    g->materials = calloc(count ? count : 1, sizeof *g->materials);
    for (u32 i = 0; i < count; i++) {
        u32 index;
        if (!take_u32(&head, &index)) return false;
        if (index == 0xFFFFFFFFu) {
            if (!material(&body, &g->materials[i])) return false;
        } else {
            if (index >= i) return false;
            g->materials[i] = g->materials[index];
        }
    }
    return true;
}

static bool bin_mesh(Reader *r, ClumpGeometry *g) {
    u32 flags, count, total;
    if (!take_u32(r, &flags) || !take_u32(r, &count) || !take_u32(r, &total) || count > 0x10000) return false;
    g->strips = flags & 1;
    g->mesh_count = (int)count;
    g->mesh_indices = total;
    g->meshes = calloc(count ? count : 1, sizeof *g->meshes);
    for (u32 i = 0; i < count; i++) {
        ClumpMesh *m = &g->meshes[i];
        if (!take_u32(r, &m->count) || !take_u32(r, &m->material)) return false;
        m->indices = take_copy(r, (size_t)m->count * 4);
        if (!m->indices) return false;
    }
    return true;
}

static bool geometry_extension(Reader *r, ClumpGeometry *g) {
    Reader ext, mesh;
    if (!chunk(r, CHUNK_EXTENSION, &ext)) return false;
    Reader at = ext;
    if (find_chunk(&at, CHUNK_BIN_MESH, &mesh)) return bin_mesh(&mesh, g);
    return true;
}

static bool geometry_data(Reader *s, ClumpGeometry *g) {
    u32 flags, triangles, vertices, morph_targets;
    if (!take_u32(s, &flags) || !take_u32(s, &triangles) || !take_u32(s, &vertices) || !take_u32(s, &morph_targets))
        return false;
    if (triangles > 0x100000 || vertices > 0x10000 || morph_targets < 1) return false;
    g->flags = flags;
    g->triangle_count = (int)triangles;
    g->vertex_count = (int)vertices;
    g->morph_target_count = (int)morph_targets;
    float surface[3];
    if (!take_floats(s, surface, 3)) return false;
    g->ambient = surface[0];
    g->specular = surface[1];
    g->diffuse = surface[2];
    if (flags & GEOMETRY_PRELIT) {
        if (!(g->prelit = take_copy(s, (size_t)vertices * 4))) return false;
    }
    if (flags & (GEOMETRY_TEXTURED | GEOMETRY_TEXTURED2)) {
        if (!(g->uv = take_copy(s, (size_t)vertices * 8))) return false;
        if ((flags & GEOMETRY_TEXTURED2) && !take(s, vertices * 8)) return false;
    }
    g->triangles = calloc(triangles ? triangles : 1, sizeof *g->triangles);
    for (u32 i = 0; i < triangles; i++) {
        const u8 *p = take(s, 8);
        if (!p) return false;
        u32 a = read_u32_at(p), b = read_u32_at(p + 4);
        g->triangles[i] = (ClumpTriangle){{(u16)(a >> 16), (u16)a, (u16)(b >> 16)}, (u16)b};
    }
    for (u32 k = 0; k < morph_targets; k++) {
        float sphere[4];
        u32 has_positions, has_normals;
        if (!take_floats(s, sphere, 4) || !take_u32(s, &has_positions) || !take_u32(s, &has_normals)) return false;
        float (*positions)[3] = has_positions ? take_copy(s, (size_t)vertices * 12) : NULL;
        float (*normals)[3] = has_normals ? take_copy(s, (size_t)vertices * 12) : NULL;
        if ((has_positions && !positions) || (has_normals && !normals)) return false;
        if (k == 0) {
            memcpy(g->bounding_sphere, sphere, sizeof sphere);
            g->positions = positions;
            g->normals = normals;
        } else {
            free(positions);
            free(normals);
        }
    }
    return true;
}

static bool geometry(Reader *r, ClumpGeometry *g) {
    Reader body, s;
    if (!chunk(r, CHUNK_GEOMETRY, &body) || !chunk(&body, CHUNK_STRUCT, &s)) return false;
    return geometry_data(&s, g) && materials(&body, g) && geometry_extension(&body, g);
}

static bool atomic(Reader *r, ClumpAtomic *a) {
    Reader body, s;
    u32 frame, geometry_index, flags;
    if (!chunk(r, CHUNK_ATOMIC, &body) || !chunk(&body, CHUNK_STRUCT, &s)) return false;
    if (!take_u32(&s, &frame) || !take_u32(&s, &geometry_index) || !take_u32(&s, &flags)) return false;
    *a = (ClumpAtomic){(int)frame, (int)geometry_index, flags};
    return true;
}

static bool clump_body(Reader *body, Reader *rest, Clump *c) {
    Reader head, frames, frames_head, list, list_head;
    u32 atomics, frame_count, geometries;
    if (!chunk(body, CHUNK_STRUCT, &head) || !take_u32(&head, &atomics) || atomics > 0x10000) return false;
    if (!chunk(body, CHUNK_FRAME_LIST, &frames) || !chunk(&frames, CHUNK_STRUCT, &frames_head) ||
        !take_u32(&frames_head, &frame_count))
        return false;
    c->frame_count = (int)frame_count;
    if (!chunk(body, CHUNK_GEOMETRY_LIST, &list) || !chunk(&list, CHUNK_STRUCT, &list_head) ||
        !take_u32(&list_head, &geometries) || geometries > 0x10000)
        return false;
    c->geometry_count = (int)geometries;
    c->geometries = calloc(geometries ? geometries : 1, sizeof *c->geometries);
    for (u32 i = 0; i < geometries; i++)
        if (!geometry(&list, &c->geometries[i])) return false;
    c->atomic_count = (int)atomics;
    c->atomics = calloc(atomics ? atomics : 1, sizeof *c->atomics);
    rest->at = body->at;
    for (u32 i = 0; i < atomics; i++)
        if (!atomic(rest, &c->atomics[i]) || c->atomics[i].geometry < 0 || c->atomics[i].geometry >= c->geometry_count)
            return false;
    return true;
}

Clump *clump_read(const u8 *bytes, u32 length) {
    Reader file = {bytes, 0, length}, body;
    if (!find_chunk(&file, CHUNK_CLUMP, &body)) return NULL;
    Reader rest = {bytes, body.at, length};
    Clump *c = calloc(1, sizeof *c);
    if (!clump_body(&body, &rest, c)) {
        clump_free(c);
        return NULL;
    }
    return c;
}

void clump_free(Clump *c) {
    if (!c) return;
    for (int i = 0; i < c->geometry_count; i++) {
        ClumpGeometry *g = &c->geometries[i];
        free(g->prelit);
        free(g->uv);
        free(g->triangles);
        free(g->positions);
        free(g->normals);
        free(g->materials);
        for (int k = 0; k < g->mesh_count; k++) free(g->meshes[k].indices);
        free(g->meshes);
    }
    free(c->geometries);
    free(c->atomics);
    free(c);
}
