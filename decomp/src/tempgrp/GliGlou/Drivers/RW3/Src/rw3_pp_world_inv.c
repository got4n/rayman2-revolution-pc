#include "common.h"

#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "renderware.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

typedef struct WorldVertex {
    RwV3d position;
    RwRGBA color;
    RwV3d normal;
    f32 u;
    f32 v;
} WorldVertex;

typedef struct MeshOrigin {
    MeshView* mesh;
    MTH3D_tdstVector_ position;
} MeshOrigin;

extern MeshOrigin inv_RW3_stMeshOrigin;
extern RwMatrix inv_RW3_stMeshWorldMatrix;
extern s16 inv_RW3_a_wMeshIndices[];

s32 inv_RW3_IM3D_fn_bTransform(WorldVertex* vertices, u32 count, RwMatrix* matrix, s32 flags);
void inv_RW3_IM3D_fn_bBuildIndicesForPrimType(s32 type, s16* indices, s32 count);
void inv_RW3_IM3D_fn_bEnd(void);

#define SET_MATRIX_IDENTITY(matrix)                                                                \
    do {                                                                                           \
        (matrix)->right.x = (matrix)->up.y = (matrix)->at.z = 1.0f;                                \
        (matrix)->right.y = (matrix)->right.z = (matrix)->up.x = 0.0f;                             \
        (matrix)->up.z = (matrix)->at.x = (matrix)->at.y = 0.0f;                                   \
        (matrix)->pos.x = (matrix)->pos.y = (matrix)->pos.z = 0.0f;                                \
        (matrix)->flags |= 0x20003;                                                                \
    } while (0)

RwV3d* RpWorldGetOrigin(RpWorld* world);

void inv_RW3_fn_vInitMeshIndices(void)
{
    s16* index = inv_RW3_a_wMeshIndices;
    u32 i;

    index[0] = 0;
    index[2] = 2;
    index[1] = 1;
    index[3] = 2;
    index += 4;
    for (i = 1; i < 99; i++) {
        index[0] = i * 3;
        index[1] = i * 3;
        index[2] = i * 3 + 1;
        index[3] = i * 3 + 2;
        index[4] = i * 3 + 2;
        index += 5;
    }
    index[0] = 0x129;
    index[1] = 0x129;
    index[2] = 0x12A;
    index[3] = 0x12B;
    SET_MATRIX_IDENTITY(&inv_RW3_stMeshWorldMatrix);
    RwMatrixTranslate(&inv_RW3_stMeshWorldMatrix, RpWorldGetOrigin(RW3_g_pWorld), 1);
}

s32 inv_RW3_fn_bAllocMeshVertices(MeshView* mesh, s32 capacity)
{
    s32 size = capacity * sizeof(WorldVertex);
    WorldVertex* vertices = AI_fn_p_vTrueAlloc(gp_6F74, size + 0xF);

    mesh->vertices = vertices;
    if (vertices == NULL) {
        return 0;
    }
    if ((u32) vertices & 0xF) {
        mesh->vertices = (WorldVertex*) (((u32) vertices + 0x10) & 0xFFFFFFF0);
    }
    memset(mesh->vertices, 0, size);
    mesh->capacity = capacity;
    mesh->vertexCount = 0;
    mesh->material = NULL;
    return 1;
}

s32 inv_RW3_fn_bCreateMesh(MeshView** out, s32 count)
{
    MeshView* mesh;

    *out = NULL;
    mesh = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(MeshView));
    if (mesh == NULL) {
        return 0;
    }
    *out = mesh;
    return inv_RW3_fn_bAllocMeshVertices(mesh, count * 3);
}

void fn_vMSSoundSetMechaCharacter(MeshView* mesh, GliMaterialData* material)
{
    mesh->material = material;
}

void inv_RW3_fn_vBeginMesh(MeshView* mesh, MTH3D_tdstVector_* position)
{
    MeshOrigin* origin = &inv_RW3_stMeshOrigin;
    MTH3D_tdstVector_* scaled = &origin->position;

    inv_RW3_stMeshOrigin.mesh = mesh;
    mesh->vertexCount = 0;
    origin->position.xX = position->xX * 0.01f;
    scaled->xY = position->xY * 0.01f;
    scaled->xZ = position->xZ * 0.01f;
}

void inv_RW3_fn_vEndMesh(MeshView* mesh)
{
    inv_RW3_stMeshOrigin.mesh = NULL;
}

void inv_RW3_fn_vAddMeshVertex(MTH3D_tdstVector_* position, f32* uv, f32 alpha)
{
    MeshView* mesh = inv_RW3_stMeshOrigin.mesh;

    if (mesh->vertexCount < mesh->capacity) {
        WorldVertex* vertex = &mesh->vertices[mesh->vertexCount++];
        RwV3d offset;

        offset.x = position->xX - inv_RW3_stMeshOrigin.position.xX;
        offset.y = position->xY - inv_RW3_stMeshOrigin.position.xY;
        offset.z = position->xZ - inv_RW3_stMeshOrigin.position.xZ;
        do {
            RwV3d copy;

            MTH3D_M_vCopyLowerXYZ_inv(&copy, &offset);
            vertex->position = copy;
        } while (0);
        {
            RwRGBA color;

            color.red = 0xFF;
            color.green = 0xFF;
            color.blue = 0xFF;
            color.alpha = alpha * 255.0f;
            vertex->color = color;
        }
        vertex->u = uv[0];
        vertex->v = uv[1];
    }
}

void inv_RW3_fn_vSetRenderObject(void* object, u32 renderFlags)
{
    GLI_tdstRenderContext* context = inv_GLI_renderContext;

    context->object = object;
    context->lHierachDrawMask = renderFlags & 0xF7FBFBF7;
    context->inv_lDrawFlags = GLI_DRAW_DEFERRED_inv;
    context->geometricObject = NULL;
    context->currentMatrix = NULL;
    context->geometryMatrix = NULL;
    context->camera = NULL;
    context->currentMaterial = NULL;
    context->resetWord2034 = 0;
    if (inv_GLI_blendModeState != 0) {
        inv_GLI_blendModeState = 0;
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        inv_GLI_zTestState = 1;
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
        inv_GLI_blendModeState = 0;
    }
    inv_GLI_renderContext->lCurrentDrawMask = -1;
    GLI_RW_vFogOff();
}

void inv_RW3_fn_vDrawMesh(MeshView* mesh, f32 weight, tdstFogParams_* fogParameters)
{
    GLI_tdstRenderContext* context = inv_GLI_renderContext;

    context->fogEnabled = fogParameters != NULL;
    context->fogParameters = fogParameters;
    context->currentMaterial = mesh->material;
    context->currentTexture = mesh->material->layers[0].texture;
    inv_GLI_textureAddressState = 0xFF;
    GLI_RW_vDoTextureSelection();
    if (inv_RW3_IM3D_fn_bTransform(mesh->vertices, mesh->vertexCount, &inv_RW3_stMeshWorldMatrix,
                                   1) != 0) {
        inv_RW3_IM3D_fn_bBuildIndicesForPrimType(4, inv_RW3_a_wMeshIndices,
                                                 mesh->vertexCount * 5 / 3 - 2);
        inv_RW3_IM3D_fn_bEnd();
    }
}
