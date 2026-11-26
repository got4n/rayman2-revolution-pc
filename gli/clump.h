#ifndef GLI_CLUMP_H
#define GLI_CLUMP_H

#include "guest.h"

enum {
    GEOMETRY_TRISTRIP = 0x01, GEOMETRY_POSITIONS = 0x02, GEOMETRY_TEXTURED = 0x04, GEOMETRY_PRELIT = 0x08,
    GEOMETRY_NORMALS = 0x10, GEOMETRY_LIGHT = 0x20, GEOMETRY_MODULATE_MATERIAL_COLOR = 0x40,
    GEOMETRY_TEXTURED2 = 0x80,
};

typedef struct ClumpMaterial {
    u8 color[4];
    float ambient, specular, diffuse;
    bool textured;
    char texture[32], mask[32];
    u32 filter_addressing;
} ClumpMaterial;

typedef struct ClumpTriangle { u16 v[3], material; } ClumpTriangle;

typedef struct ClumpMesh {
    u32 material;
    u32 count;
    u32 *indices;
} ClumpMesh;

typedef struct ClumpGeometry {
    u32 flags;
    int vertex_count, triangle_count;
    int morph_target_count;
    float ambient, specular, diffuse;
    u8 (*prelit)[4];
    float (*uv)[2];
    ClumpTriangle *triangles;
    float bounding_sphere[4];
    float (*positions)[3];
    float (*normals)[3];
    int material_count;
    ClumpMaterial *materials;
    bool strips;
    int mesh_count;
    u32 mesh_indices;
    ClumpMesh *meshes;
} ClumpGeometry;

typedef struct ClumpAtomic {
    int frame, geometry;
    u32 flags;
} ClumpAtomic;

typedef struct Clump {
    int frame_count;
    int geometry_count;
    ClumpGeometry *geometries;
    int atomic_count;
    ClumpAtomic *atomics;
} Clump;

Clump *clump_read(const u8 *bytes, u32 length);
void clump_free(Clump *c);

#endif
