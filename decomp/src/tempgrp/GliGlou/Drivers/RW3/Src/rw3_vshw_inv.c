#include "common.h"

#include "GLI/GLD.h"
#include "GLI/GLI.h"

#include "renderware.h"
#include "MTH/MTH.h"
#include "GAM/GAM.h"
#include "SCT/SCT.h"

#define QUAD_BATCH_COUNT 5
#define QUAD_BATCH_VIEW_SPRITES 0
#define QUAD_BATCH_FLAT_SPRITES 1
#define QUAD_BATCH_ORIENTED_SPRITES 2
#define QUAD_BATCH_CORNER_QUADS 3

#define VIEW_SPRITE_CAPACITY 400
#define FLAT_SPRITE_CAPACITY 300
#define ORIENTED_SPRITE_CAPACITY 350
#define CORNER_QUAD_CAPACITY 250

#define QUAD_RENDER_FLAGS (~0x408)

extern u32 inv_GLI_quadBatchCounts[QUAD_BATCH_COUNT];
extern GliTexture** inv_GLI_quadBatchTextures[QUAD_BATCH_COUNT];
extern Quad* inv_GLI_quadBatchQuads[QUAD_BATCH_COUNT];
extern RwMatrix inv_RW3_stWorldOriginMatrix;
extern Quad inv_GLI_viewSpriteQuads[];
extern GliTexture* inv_GLI_viewSpriteTextures[];
extern s16 inv_RW3_a_wIm3DIndices[];

s32 inv_RW3_IM3D_fn_bTransform(Quad* vertices, s32 vertexCount, void* matrix, s32 flags);
void inv_RW3_IM3D_fn_bBuildIndicesForPrimType(s32 primitive, s16* indices, s32 count);
void inv_RW3_IM3D_fn_bEnd(void);
void inv_RW3_fn_vFlushQuadBatch(GLD_tdstViewportAttributes_* viewport, s32 batch);
void inv_RW3_fn_vRenderWorldQuads(Quad* quads, s32 quadCount);
void inv_RW3_fn_vRenderQuadRun(GLD_tdstViewportAttributes_* viewport, Quad* quads, s32 quadCount);
void inv_RW3_fn_vRenderWorldPlanarSprites(Quad* quads, s32 quadCount);
void inv_RW3_fn_vRenderWorldOrthoPlanarSprites(Quad* quads, s32 quadCount);
void inv_RW3_fn_vClearQuadBatchCounts(void);
RwV3d* RpWorldGetOrigin(RpWorld* world);

typedef void (*SpriteMaterialSelection)(void* material, SpriteElement* sprite);

#define QUAD_BATCH_4_CAPACITY 400
#define GLI_LIGHT_TYPE_1 1
#define GLI_LIGHT_TYPE_4 4

#define SET_MATRIX_IDENTITY(matrix)                                                                \
    do {                                                                                           \
        (matrix)->right.x = (matrix)->up.y = (matrix)->at.z = 1.0f;                                \
        (matrix)->right.y = (matrix)->right.z = (matrix)->up.x = 0.0f;                             \
        (matrix)->up.z = (matrix)->at.x = (matrix)->at.y = 0.0f;                                   \
        (matrix)->pos.x = (matrix)->pos.y = (matrix)->pos.z = 0.0f;                                \
        (matrix)->flags |= 0x20003;                                                                \
    } while (0)

static inline POS_tdstMatrix_* inv_GLI_fn_p_stViewportCameraMatrix(GLD_tdstViewportAttributes_* viewport)
{
    GLI_tdstSpecificAttributesFor3D_* attributes3D = viewport->p_vSpecificToXD;

    return &attributes3D->p_stCam->stMatrix;
}

static inline void inv_GLI_fn_vSetQuadColor(Quad* quad, GliVertexColor* color)
{
    quad->v[0].color.r = color->channels.red;
    quad->v[0].color.g = color->channels.green;
    quad->v[0].color.b = color->channels.blue;
    quad->v[0].color.a = color->channels.alpha;
    quad->v[1].color = quad->v[0].color;
    quad->v[2].color = quad->v[0].color;
    quad->v[3].color = quad->v[0].color;
}

static inline GLI_tdstRenderContext* inv_GLI_fn_p_stReloadRenderContext(void)
{
    return *(GLI_tdstRenderContext* volatile*) &inv_GLI_renderContext;
}

static inline void inv_GLI_fn_vRestoreRenderFlags(s32 flags)
{
    inv_GLI_fn_p_stReloadRenderContext()->lHierachDrawMask = flags;
}

static inline s32 inv_GLI_fn_lMatrixTypeIs1(POS_tdstMatrix_* matrix)
{
    return matrix->ucType == MAT_eTransform_Identity;
}

static inline void inv_GLI_fn_vSetSpriteCorners(Quad* quad, MTH3D_tdstVector_* center, f32 halfWidth,
                                    f32 halfHeight)
{
    f32 left = center->xX - halfWidth;
    f32 bottom = center->xY - halfHeight;
    f32 right = center->xX + halfWidth;
    f32 top = center->xY + halfHeight;
    f32 z = center->xZ;

    MTH3D_M_vSetXYZ_inv(&quad->v[0].position, left, bottom, z);
    MTH3D_M_vSetXYZ_inv(&quad->v[1].position, right, bottom, z);
    MTH3D_M_vSetXYZ_inv(&quad->v[2].position, right, top, z);
    MTH3D_M_vSetXYZ_inv(&quad->v[3].position, left, top, z);
}

void inv_RW3_fn_vInitQuadBatches(void)
{
    QuadVertex vertex;
    s16* index;
    u32 i;
    s32 first;

    MTH3D_M_vZeroXYZ_inv(&vertex.position);
    vertex.color.r = 0xFF;
    vertex.color.g = 0xFF;
    vertex.color.b = 0xFF;
    vertex.color.a = 0xFF;
    MTH3D_M_vSetXYZ_inv(&vertex.normal, 0.0f, 0.0f, -1.0f);

    vertex.u = 1.0f;
    vertex.v = 0.0f;
    inv_GLI_singleQuad.v[0] = vertex;
    vertex.u = 0.0f;
    vertex.v = 0.0f;
    inv_GLI_singleQuad.v[1] = vertex;
    vertex.u = 0.0f;
    vertex.v = 1.0f;
    inv_GLI_singleQuad.v[2] = vertex;
    vertex.u = 1.0f;
    vertex.v = 1.0f;
    inv_GLI_singleQuad.v[3] = vertex;

    inv_GLI_quadBatchQuads[QUAD_BATCH_VIEW_SPRITES] = inv_GLI_viewSpriteQuads;
    inv_GLI_quadBatchQuads[QUAD_BATCH_FLAT_SPRITES] =
        inv_GLI_quadBatchQuads[QUAD_BATCH_VIEW_SPRITES] + VIEW_SPRITE_CAPACITY;
    inv_GLI_quadBatchQuads[QUAD_BATCH_ORIENTED_SPRITES] =
        inv_GLI_quadBatchQuads[QUAD_BATCH_FLAT_SPRITES] + FLAT_SPRITE_CAPACITY;
    inv_GLI_quadBatchQuads[QUAD_BATCH_CORNER_QUADS] =
        inv_GLI_quadBatchQuads[QUAD_BATCH_ORIENTED_SPRITES] + ORIENTED_SPRITE_CAPACITY;
    inv_GLI_quadBatchQuads[4] =
        inv_GLI_quadBatchQuads[QUAD_BATCH_CORNER_QUADS] + CORNER_QUAD_CAPACITY;
    inv_GLI_quadBatchTextures[QUAD_BATCH_VIEW_SPRITES] = inv_GLI_viewSpriteTextures;
    inv_GLI_quadBatchTextures[QUAD_BATCH_FLAT_SPRITES] =
        inv_GLI_quadBatchTextures[QUAD_BATCH_VIEW_SPRITES] + VIEW_SPRITE_CAPACITY;
    inv_GLI_quadBatchTextures[QUAD_BATCH_ORIENTED_SPRITES] =
        inv_GLI_quadBatchTextures[QUAD_BATCH_FLAT_SPRITES] + FLAT_SPRITE_CAPACITY;
    inv_GLI_quadBatchTextures[QUAD_BATCH_CORNER_QUADS] =
        inv_GLI_quadBatchTextures[QUAD_BATCH_ORIENTED_SPRITES] + ORIENTED_SPRITE_CAPACITY;
    inv_GLI_quadBatchTextures[4] =
        inv_GLI_quadBatchTextures[QUAD_BATCH_CORNER_QUADS] + CORNER_QUAD_CAPACITY;

    index = inv_RW3_a_wIm3DIndices;
    index[0] = 1;
    index[1] = 0;
    index[2] = 2;
    index[3] = 3;
    index[4] = 3;
    index += 5;
    for (i = 1; i < 399; i++) {
        first = i * 4 + 1;
        index[0] = first;
        index[1] = first;
        index[2] = i * 4;
        index[3] = first + 1;
        index[4] = first + 2;
        index[5] = first + 2;
        index += 6;
    }
    index[0] = 0x63D;
    index[1] = 0x63C;
    index[2] = 0x63C;
    index[3] = 0x63E;
    index[4] = 0x63F;

    for (i = 0; i < 1700; i++) {
        inv_GLI_viewSpriteQuads[i] = inv_GLI_singleQuad;
    }

    SET_MATRIX_IDENTITY(&inv_RW3_stWorldOriginMatrix);

    RwMatrixTranslate(&inv_RW3_stWorldOriginMatrix, RpWorldGetOrigin(RW3_g_pWorld), 1);
    inv_RW3_fn_vClearQuadBatchCounts();
}

void inv_RW3_fn_vClearQuadBatchCounts(void)
{
    memset(inv_GLI_quadBatchCounts, 0, sizeof(inv_GLI_quadBatchCounts));
}

void inv_RW3_fn_vFlushAllQuadBatches(GLD_tdstViewportAttributes_* viewport)
{
    inv_GLI_renderContext->lCurrentDrawMask = -1;

    GLI_RW_vFogOff();
    inv_RW3_fn_vFlushQuadBatch(viewport, QUAD_BATCH_VIEW_SPRITES);
    inv_RW3_fn_vFlushQuadBatch(viewport, QUAD_BATCH_FLAT_SPRITES);
    inv_RW3_fn_vFlushQuadBatch(viewport, QUAD_BATCH_ORIENTED_SPRITES);
    inv_RW3_fn_vFlushQuadBatch(viewport, QUAD_BATCH_CORNER_QUADS);
    inv_RW3_fn_vFlushQuadBatch(viewport, 4);
    inv_RW3_fn_vClearQuadBatchCounts();
}

void inv_RW3_fn_vFlushQuadBatch(GLD_tdstViewportAttributes_* viewport, s32 batch)
{
    Quad* quads;
    Quad* group;
    GliTexture** textures;
    GliTexture* texture;
    u32 count;
    u32 done;
    u32 next;
    s32 run;

    textures = inv_GLI_quadBatchTextures[batch];
    quads = inv_GLI_quadBatchQuads[batch];
    count = inv_GLI_quadBatchCounts[batch];
    run = 0;
    group = quads;
    texture = *textures;
    for (done = 0; done < count; done = next) {
        if (run + done < count) {
            do {
                textures++;
                run++;
                quads++;
                if (texture != *textures) {
                    break;
                }
            } while (run + done < count);
        }
        next = run + done;
        inv_GLI_renderContext->currentTexture = texture;
        GLI_RW_vDoTextureSelection();
        switch (batch) {
        case QUAD_BATCH_VIEW_SPRITES:
            inv_RW3_fn_vRenderQuadRun(viewport, group, run);
            break;
        case QUAD_BATCH_FLAT_SPRITES:
            inv_RW3_fn_vRenderWorldPlanarSprites(group, run);
            break;
        case QUAD_BATCH_ORIENTED_SPRITES:
            inv_RW3_fn_vRenderWorldOrthoPlanarSprites(group, run);
            break;
        case QUAD_BATCH_CORNER_QUADS:
            inv_RW3_fn_vRenderWorldQuads(group, run);
            break;
        case 4:
            inv_RW3_fn_vRenderQuadRun(viewport, group, run);
            break;
        }
        texture = *textures;
        group = quads;
        run = 0;
    }
}

void inv_RW3_fn_vDrawTexturedQuads(GLD_tdstViewportAttributes_* viewport, Quad* quads, GliTexture* texture)
{
    inv_GLI_renderContext->lCurrentDrawMask = -1;
    GLI_RW_vFogOff();
    inv_GLI_renderContext->currentTexture = texture;
    GLI_RW_vDoTextureSelection();
    inv_RW3_fn_vRenderQuadRun(viewport, quads, 1);
}

void inv_RW3_fn_vDrawSpriteList(GLD_tdstViewportAttributes_* viewport, SpriteElementList* list)
{
    SpriteElement* first;
    SpriteElement* sprite;
    s32 i;
    s32 savedFlags;
    MTH3D_tdstVector_* vertices;
    void* object;
    MTH3D_tdstVector_* directVertices;

    first = list->sprites;
    if (first->flags & 0x20) {
        SpriteElement* deferred = first;

        for (i = 0; i < list->count; i++) {
            inv_RW3_fn_vQueueHalo(inv_GLI_renderContext->geometricObject, deferred);
            deferred++;
        }
        return;
    }

    object = first->u.material;
    savedFlags = inv_GLI_renderContext->lHierachDrawMask;
    inv_GLI_renderContext->lHierachDrawMask = savedFlags & ~0x400;
    inv_GLI_renderContext->currentMaterial = object;
    ((SpriteMaterialSelection) GLI_vDoMaterialSelection)(object, first);

    if (inv_GLI_renderContext->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) {
        vertices = inv_GLI_renderContext->geometricObject->vertices;
        sprite = list->sprites;
        if (inv_SCT_bSendingSectorToViewport == 1) {
            if (inv_GLI_renderContext->geometricObject->flags & 8) {
                GliTexture* texture = inv_GLI_renderContext->currentMaterial->layers[0].texture;

                if (texture != NULL && !(texture->flags & 0x20)) {
                    GEO_tdstColor_ ambient;
                    GEO_tdstColor_ parallel;
                    GLI_tdstLight_* ambientLight;
                    GLI_tdstLight_* parallelLight;
                    GEO_tdstColor_* color;
                    s32 red, green, blue;
                    s32 j;
                    GLI_tdstLight_** lights;

                    memset(&ambient, 0, sizeof(ambient));
                    ambient.xR = 1.0f;
                    memset(&parallel, 0, sizeof(parallel));
                    ambientLight = NULL;
                    parallelLight = NULL;
                    lights = gs_aDEF_p_CurrentListOfLight;
                    for (j = 0; j < gs_CurrentNumberOfLightInViewport &&
                                (ambientLight == NULL || parallelLight == NULL);
                         j++, lights++) {
                        if ((inv_RW3_ulViewportLightMask >> j) & 1) {
                            GLI_tdstLight_* light = *lights;

                            if (light->lTypeOfLight == GLI_LIGHT_TYPE_4) {
                                ambientLight = light;
                            } else if (light->lTypeOfLight == GLI_LIGHT_TYPE_1) {
                                parallelLight = light;
                            }
                        }
                    }
                    if (ambientLight != NULL) {
                        ambient = ambientLight->stColor;
                    }
                    if (parallelLight != NULL) {
                        parallel = parallelLight->stColor;
                    }
                    color = &inv_GLI_renderContext->currentMaterial->color;
                    red = (s32) inv_MTH_fn_xMinimum(
                        color->xR * 255.0f * (ambient.xR + parallel.xR * 0.5f), 255.0f);
                    green = (s32) inv_MTH_fn_xMinimum(
                        color->xG * 255.0f * (ambient.xG + parallel.xG * 0.5f), 255.0f);
                    blue = (s32) inv_MTH_fn_xMinimum(
                        color->xB * 255.0f * (ambient.xB + parallel.xB * 0.5f), 255.0f);
                    inv_GLI_renderContext->vertexColor.packed =
                        (inv_GLI_renderContext->vertexColor.packed & 0xFF000000) |
                        ((red << 16) | (green << 8) | blue);
                }
            } else {
                GEO_tdstColor_* color = &inv_GLI_renderContext->currentMaterial->color;
                s32 red, green, blue;

                red = (s32) inv_MTH_fn_xMinimum(color->xR * 255.0f, 255.0f);
                green = (s32) inv_MTH_fn_xMinimum(color->xG * 255.0f, 255.0f);
                blue = (s32) inv_MTH_fn_xMinimum(color->xB * 255.0f, 255.0f);
                inv_GLI_renderContext->vertexColor.packed =
                    (inv_GLI_renderContext->vertexColor.packed & 0xFF000000) |
                    ((red << 16) | (green << 8) | blue);
            }

            for (i = 0; i < list->count; i++, sprite++) {
                TexturedDrawRequest* reserved;

                if (inv_RW3_fn_bReserveTexturedRequest(&reserved,
                                  inv_GLI_renderContext->currentMaterial->layers[0].texture) == 1) {
                    Quad* quad;
                    s32 index = sprite->vertexIndex;
                    f32 halfWidth = POS_fn_xGetXScale(g_p_stCurrentMatrix) * sprite->halfWidth;
                    f32 halfHeight = POS_fn_xGetXScale(g_p_stCurrentMatrix) * sprite->halfHeight;
                    MTH3D_tdstVector_* vertex;
                    GliVertexColor color;
                    POS_tdstMatrix_* matrix;
                    MTH3D_tdstVector_ center;

                    quad = &reserved->quad;
                    vertex = &vertices[index];
                    object = inv_GLI_renderContext->object;
                    color = inv_GLI_renderContext->vertexColor;
                    matrix = inv_GLI_fn_p_stViewportCameraMatrix(object);
                    if (!inv_GLI_fn_lMatrixTypeIs1(g_p_stCurrentMatrix)) {
                        POS_fn_vMulMatrixVertex(&center, g_p_stCurrentMatrix, vertex);
                    } else {
                        center = *vertex;
                    }
                    POS_fn_vApplyMatrixToVector(&center, matrix, &center);
                    inv_GLI_fn_vSetSpriteCorners(quad, &center, halfWidth, halfHeight);
                    inv_GLI_fn_vSetQuadColor(quad, &color);
                }
            }
        } else {
            for (i = 0; i < list->count; i++, sprite++) {
                GliMaterialData* current = inv_GLI_renderContext->currentMaterial;
                s32 index = sprite->vertexIndex;
                f32 halfWidth = POS_fn_xGetXScale(g_p_stCurrentMatrix) * sprite->halfWidth;
                f32 halfHeight = POS_fn_xGetXScale(g_p_stCurrentMatrix) * sprite->halfHeight;
                MTH3D_tdstVector_* vertex = &vertices[index];
                u32 count = inv_GLI_quadBatchCounts[4];
                GliTexture* texture = current->layers[0].texture;
                GliVertexColor color = inv_GLI_renderContext->vertexColor;
                GLD_tdstViewportAttributes_* target = inv_GLI_renderContext->object;

                if (count != QUAD_BATCH_4_CAPACITY) {
                    Quad* quad;
                    POS_tdstMatrix_* matrix;
                    MTH3D_tdstVector_ center;
                    GliVertexColor batchColor;

                    quad = &inv_GLI_quadBatchQuads[4][count];
                    inv_GLI_quadBatchTextures[4][count] = texture;
                    batchColor = color;
                    inv_GLI_quadBatchCounts[4]++;
                    matrix = inv_GLI_fn_p_stViewportCameraMatrix(target);
                    if (!inv_GLI_fn_lMatrixTypeIs1(g_p_stCurrentMatrix)) {
                        POS_fn_vMulMatrixVertex(&center, g_p_stCurrentMatrix, vertex);
                    } else {
                        center = *vertex;
                    }
                    POS_fn_vApplyMatrixToVector(&center, matrix, &center);
                    inv_GLI_fn_vSetSpriteCorners(quad, &center, halfWidth, halfHeight);
                    inv_GLI_fn_vSetQuadColor(quad, &batchColor);
                }
            }
            inv_GLI_renderContext->lHierachDrawMask = savedFlags;
            return;
        }
    } else {
        directVertices = inv_GLI_renderContext->geometricObject->vertices;
        sprite = list->sprites;
        for (i = 0; i < list->count; i++, sprite++) {
            Quad* quad = &inv_GLI_singleQuad;
            s32 index = sprite->vertexIndex;
            f32 halfWidth = POS_fn_xGetXScale(g_p_stCurrentMatrix) * sprite->halfWidth;
            f32 halfHeight = POS_fn_xGetXScale(g_p_stCurrentMatrix) * sprite->halfHeight;
            MTH3D_tdstVector_* vertex = &directVertices[index];
            GliVertexColor color;
            POS_tdstMatrix_* matrix;
            MTH3D_tdstVector_ center;

            object = inv_GLI_renderContext->object;
            color = inv_GLI_renderContext->vertexColor;
            matrix = inv_GLI_fn_p_stViewportCameraMatrix(object);
            if (!inv_GLI_fn_lMatrixTypeIs1(g_p_stCurrentMatrix)) {
                POS_fn_vMulMatrixVertex(&center, g_p_stCurrentMatrix, vertex);
            } else {
                center = *vertex;
            }
            POS_fn_vApplyMatrixToVector(&center, matrix, &center);
            inv_GLI_fn_vSetSpriteCorners(quad, &center, halfWidth, halfHeight);
            inv_GLI_fn_vSetQuadColor(quad, &color);
            inv_RW3_fn_vRenderQuadRun(inv_GLI_renderContext->object, &inv_GLI_singleQuad, 1);
        }
    }
    inv_GLI_renderContext->lHierachDrawMask = savedFlags;
}

void GLI_vDrawWorldQuadOpt(MTH3D_tdstVector_* corner0, MTH3D_tdstVector_* corner1,
                   MTH3D_tdstVector_* corner2, MTH3D_tdstVector_* corner3,
                   GliMaterialData* material)
{
    GLI_tdstRenderContext* context;
    GliVertexColor batchColor;
    GliVertexColor directColor;

    context = inv_GLI_renderContext;
    g_BaseTimeForParticleAnimation = 1.0f;
    g_bTakeCareOfAnimationCoordination = 1;
    g_bOneShotAnim = 1;
    g_bHasTextureAnimationFinished = 0;
    context->currentMaterial = material;
    context->lHierachDrawMask = QUAD_RENDER_FLAGS;
    GLI_vDoMaterialSelection();

    if (context->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) {
        GliTexture* texture = material->layers[0].texture;
        u32 index = inv_GLI_quadBatchCounts[QUAD_BATCH_CORNER_QUADS];
        GliVertexColor color = context->vertexColor;

        if (index != CORNER_QUAD_CAPACITY) {
            Quad* quad = &inv_GLI_quadBatchQuads[QUAD_BATCH_CORNER_QUADS][index];

            inv_GLI_quadBatchTextures[QUAD_BATCH_CORNER_QUADS][index] = texture;
            batchColor = color;
            inv_GLI_quadBatchCounts[QUAD_BATCH_CORNER_QUADS]++;
            quad->v[0].position = *corner0;
            quad->v[1].position = *corner1;
            quad->v[2].position = *corner2;
            quad->v[3].position = *corner3;
            inv_GLI_fn_vSetQuadColor(quad, &batchColor);
        }
    } else {
        Quad* quad;

        directColor = context->vertexColor;
        inv_GLI_singleQuad.v[0].position = *corner0;
        quad = &inv_GLI_singleQuad;
        quad->v[1].position = *corner1;
        quad->v[2].position = *corner2;
        quad->v[3].position = *corner3;
        inv_GLI_fn_vSetQuadColor(quad, &directColor);
        inv_RW3_fn_vRenderWorldQuads(quad, 1);
    }

    g_bTakeCareOfAnimationCoordination = 0;
    g_bOneShotAnim = 0;
}

void inv_RW3_fn_vRenderWorldQuads(Quad* quads, s32 quadCount)
{
    s32 indicesPerQuad;

    inv_RW3_IM3D_fn_bTransform(quads, quadCount << 2, &inv_RW3_stWorldOriginMatrix, 1);
    indicesPerQuad = 6;
    quadCount = quadCount * indicesPerQuad;
    inv_RW3_IM3D_fn_bBuildIndicesForPrimType(4, inv_RW3_a_wIm3DIndices, quadCount - 2);
    inv_RW3_IM3D_fn_bEnd();
}

static inline void inv_GLI_fn_vSetViewSpriteCorners(Quad* quad, MTH3D_tdstVector_* center, f32 halfWidth,
                                        f32 halfHeight)
{
    f32 x = center->xX;
    f32 y = center->xY;
    f32 right = x + halfWidth;
    f32 z = center->xZ;
    f32 top = y + halfHeight;

    x -= halfWidth;
    y -= halfHeight;
    MTH3D_M_vSetXYZ_inv(&quad->v[0].position, x, y, z);
    MTH3D_M_vSetXYZ_inv(&quad->v[1].position, right, y, z);
    MTH3D_M_vSetXYZ_inv(&quad->v[2].position, right, top, z);
    MTH3D_M_vSetXYZ_inv(&quad->v[3].position, x, top, z);
}

u8 GLI_vDrawWorldSpriteOpt(MTH3D_tdstVector_* center, GliMaterialData* material, f32 halfWidth, f32 halfHeight, GLD_tdstViewportAttributes_* viewport, u32 count)
{
    GLI_tdstRenderContext* context;
    GLI_tdstRenderContext* selected;
    s32 savedFlags;
    MTH3D_tdstVector_ viewCenter;
    GliVertexColor batchColor;
    GliVertexColor directColor;

    context = inv_GLI_renderContext;
    savedFlags = context->lHierachDrawMask;
    g_bTakeCareOfAnimationCoordination = 1;
    g_BaseTimeForParticleAnimation = (f32) count;
    g_bOneShotAnim = 1;
    g_bHasTextureAnimationFinished = 0;
    context->lHierachDrawMask = QUAD_RENDER_FLAGS;
    context->currentMaterial = material;
    GLI_vDoMaterialSelection();

    selected = inv_GLI_renderContext;
    selected->unk20AC = 1;

    if (selected->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) {
        GliTexture* texture = material->layers[0].texture;
        u32 index = inv_GLI_quadBatchCounts[QUAD_BATCH_VIEW_SPRITES];
        GliVertexColor color = selected->vertexColor;

        if (index != VIEW_SPRITE_CAPACITY) {
            Quad* quad = &inv_GLI_quadBatchQuads[QUAD_BATCH_VIEW_SPRITES][index];

            inv_GLI_quadBatchTextures[QUAD_BATCH_VIEW_SPRITES][index] = texture;
            batchColor = color;
            inv_GLI_quadBatchCounts[QUAD_BATCH_VIEW_SPRITES]++;
            POS_fn_vApplyMatrixToVector(&viewCenter, inv_GLI_fn_p_stViewportCameraMatrix(viewport), center);
            inv_GLI_fn_vSetViewSpriteCorners(quad, &viewCenter, halfWidth, halfHeight);
            inv_GLI_fn_vSetQuadColor(quad, &batchColor);
        }
    } else {
        POS_tdstMatrix_* matrix = inv_GLI_fn_p_stViewportCameraMatrix(viewport);
        Quad* quad;

        directColor = selected->vertexColor;
        quad = &inv_GLI_singleQuad;
        POS_fn_vApplyMatrixToVector(&viewCenter, matrix, center);
        inv_GLI_fn_vSetViewSpriteCorners(quad, &viewCenter, halfWidth, halfHeight);
        inv_GLI_fn_vSetQuadColor(quad, &directColor);
        inv_RW3_fn_vRenderQuadRun(viewport, quad, 1);
    }

    inv_GLI_renderContext->unk203C = 0;
    inv_GLI_fn_vRestoreRenderFlags(savedFlags);
    g_bTakeCareOfAnimationCoordination = 0;
    g_bOneShotAnim = 0;
    return g_bHasTextureAnimationFinished;
}

void RwMatrixUpdate(MTH4D_tdstMatrix_* matrix);

void inv_RW3_fn_vRenderQuadRun(GLD_tdstViewportAttributes_* viewport, Quad* quads, s32 quadCount)
{
    MTH4D_tdstMatrix_* matrix = &inv_GLI_renderContext->im3dMatrix;
    MTH4D_tdstVector_* right = &matrix->stCol_0;
    MTH4D_tdstVector_* up = &matrix->stCol_1;
    MTH4D_tdstVector_* at = &matrix->stCol_2;
    MTH4D_tdstVector_* position = &matrix->stCol_3;
    MTH4D_tdstMatrix_* camera = &inv_GLI_fn_p_stViewportCameraMatrix(viewport)->stMatrix;

    MTH3D_M_vSetXYZ_inv(right, camera->stCol_0.xX, camera->stCol_1.xX, camera->stCol_2.xX);
    MTH3D_M_vSetXYZ_inv(up, camera->stCol_0.xY, camera->stCol_1.xY, camera->stCol_2.xY);
    MTH3D_M_vSetXYZ_inv(at, camera->stCol_0.xZ, camera->stCol_1.xZ, camera->stCol_2.xZ);
    MTH3D_M_vSetXYZ_inv(position, inv_RW3_stWorldOrigin.x, inv_RW3_stWorldOrigin.y,
                        inv_RW3_stWorldOrigin.z);
    RwMatrixUpdate(matrix);
    inv_RW3_IM3D_fn_bTransform(quads, quadCount * 4, matrix, 1);
    inv_RW3_IM3D_fn_bBuildIndicesForPrimType(4, inv_RW3_a_wIm3DIndices, quadCount * 6 - 2);
    inv_RW3_IM3D_fn_bEnd();
}

static inline void inv_GLI_fn_vSetFlatSpriteCorners(Quad* quad, MTH3D_tdstVector_* center, f32 halfWidth,
                                        f32 halfHeight)
{
    f32 x = center->xX;
    f32 y = center->xY;
    f32 right = x + halfWidth;
    f32 top = y + halfHeight;
    f32 left = x - halfWidth;
    f32 bottom = y - halfHeight;
    f32 z;

    quad->v[0].position.xX = left;
    quad->v[0].position.xY = bottom;
    quad->v[1].position.xX = left;
    quad->v[1].position.xY = top;
    quad->v[2].position.xX = right;
    quad->v[2].position.xY = top;
    quad->v[3].position.xX = right;
    quad->v[3].position.xY = bottom;
    z = center->xZ;
    quad->v[3].position.xZ = z;
    quad->v[2].position.xZ = z;
    quad->v[1].position.xZ = z;
    quad->v[0].position.xZ = z;
}

u8 GLI_vDrawWorldPlanarSprite(MTH3D_tdstVector_* center, GliMaterialData* material, f32 halfWidth, f32 halfHeight, GLD_tdstViewportAttributes_* viewport, u32 count)
{
    s32 savedFlags;
    s32 flags;

    savedFlags = inv_GLI_renderContext->lHierachDrawMask;
    g_bTakeCareOfAnimationCoordination = 1;
    g_BaseTimeForParticleAnimation = (f32) count;
    g_bHasTextureAnimationFinished = 0;
    g_bOneShotAnim = 1;
    flags = inv_GLI_renderContext->lHierachDrawMask & QUAD_RENDER_FLAGS;
    inv_GLI_renderContext->currentMaterial = material;
    inv_GLI_renderContext->lHierachDrawMask = flags;
    inv_GLI_renderContext->lCurrentDrawMask = flags;
    inv_GLI_renderContext->unk20AC = 1;
    inv_GLI_renderContext->unk203C = 1;
    GLI_vDoMaterialSelection();

    if (inv_GLI_renderContext->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) {
        GliTexture* texture = material->layers[0].texture;
        u32 index = inv_GLI_quadBatchCounts[QUAD_BATCH_FLAT_SPRITES];
        GliVertexColor color = inv_GLI_renderContext->vertexColor;
        GliVertexColor batchColor;

        if (index != FLAT_SPRITE_CAPACITY) {
            Quad* quad = &inv_GLI_quadBatchQuads[QUAD_BATCH_FLAT_SPRITES][index];

            inv_GLI_quadBatchTextures[QUAD_BATCH_FLAT_SPRITES][index] = texture;
            batchColor = color;
            inv_GLI_quadBatchCounts[QUAD_BATCH_FLAT_SPRITES]++;
            inv_GLI_fn_vSetFlatSpriteCorners(quad, center, halfWidth, halfHeight);
            inv_GLI_fn_vSetQuadColor(quad, &batchColor);
        }
    } else {
        GliVertexColor directColor = inv_GLI_renderContext->vertexColor;
        Quad* quad = &inv_GLI_singleQuad;

        inv_GLI_fn_vSetFlatSpriteCorners(quad, center, halfWidth, halfHeight);
        inv_GLI_fn_vSetQuadColor(quad, &directColor);
        inv_RW3_fn_vRenderWorldPlanarSprites(quad, 1);
    }

    inv_GLI_renderContext->unk203C = 0;
    inv_GLI_fn_vRestoreRenderFlags(savedFlags);
    g_bTakeCareOfAnimationCoordination = 0;
    g_bOneShotAnim = 0;
    return g_bHasTextureAnimationFinished;
}

void inv_RW3_fn_vRenderWorldPlanarSprites(Quad* quads, s32 quadCount)
{
    s32 indicesPerQuad;

    inv_RW3_IM3D_fn_bTransform(quads, quadCount << 2, &inv_RW3_stWorldOriginMatrix, 1);
    indicesPerQuad = 6;
    quadCount = quadCount * indicesPerQuad;
    inv_RW3_IM3D_fn_bBuildIndicesForPrimType(4, inv_RW3_a_wIm3DIndices, quadCount - 2);
    inv_RW3_IM3D_fn_bEnd();
}

static inline void inv_GLI_fn_vSetOrientedSpriteCorners(Quad* quad, MTH3D_tdstVector_* center,
                                            MTH3D_tdstVector_* axis0, MTH3D_tdstVector_* axis1,
                                            f32 scale0, f32 scale1)
{
    f32 x0 = scale0 * axis0->xX;
    f32 x1 = scale1 * axis1->xX;
    f32 y0 = scale0 * axis0->xY;
    f32 y1 = scale1 * axis1->xY;
    f32 z0 = scale0 * axis0->xZ;
    f32 z1 = scale1 * axis1->xZ;
    f32 difference = x0 - x1;
    f32 sum = x0 + x1;

    quad->v[0].position.xX = center->xX - difference;
    quad->v[1].position.xX = center->xX + sum;
    quad->v[2].position.xX = center->xX + difference;
    difference = y0 - y1;
    quad->v[3].position.xX = center->xX - sum;
    sum = y0 + y1;
    quad->v[0].position.xY = center->xY - difference;
    quad->v[1].position.xY = center->xY + sum;
    quad->v[2].position.xY = center->xY + difference;
    difference = z0 - z1;
    quad->v[3].position.xY = center->xY - sum;
    sum = z0 + z1;
    quad->v[0].position.xZ = center->xZ - difference;
    quad->v[1].position.xZ = center->xZ + sum;
    quad->v[2].position.xZ = center->xZ + difference;
    quad->v[3].position.xZ = center->xZ - sum;
}

u8 GLI_vDrawWorldOrthoPlanarSprite(MTH3D_tdstVector_* center, GliMaterialData* material, f32 scale0, f32 scale1, GLD_tdstViewportAttributes_* viewport, u32 count, MTH3D_tdstVector_* axis0, MTH3D_tdstVector_* axis1)
{
    GLI_tdstRenderContext* context;
    GLI_tdstRenderContext* selected;
    s32 savedFlags;
    s32 flags;

    context = inv_GLI_renderContext;
    g_bTakeCareOfAnimationCoordination = 1;
    g_BaseTimeForParticleAnimation = (f32) count;
    g_bHasTextureAnimationFinished = 0;
    g_bOneShotAnim = 1;
    savedFlags = context->lHierachDrawMask;
    context->currentMaterial = material;
    flags = savedFlags & QUAD_RENDER_FLAGS;
    context->lHierachDrawMask = flags;
    context->lCurrentDrawMask = flags;
    GLI_vDoMaterialSelection();

    selected = inv_GLI_renderContext;
    selected->unk20AC = 1;
    selected->unk203C = 1;
    selected = inv_GLI_fn_p_stReloadRenderContext();

    if (selected->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) {
        GliTexture* texture = material->layers[0].texture;
        u32 index = inv_GLI_quadBatchCounts[QUAD_BATCH_ORIENTED_SPRITES];
        GliVertexColor color = selected->vertexColor;
        GliVertexColor batchColor;

        if (index != ORIENTED_SPRITE_CAPACITY) {
            Quad* quad = &inv_GLI_quadBatchQuads[QUAD_BATCH_ORIENTED_SPRITES][index];

            inv_GLI_quadBatchTextures[QUAD_BATCH_ORIENTED_SPRITES][index] = texture;
            batchColor = color;
            inv_GLI_quadBatchCounts[QUAD_BATCH_ORIENTED_SPRITES]++;
            inv_GLI_fn_vSetOrientedSpriteCorners(quad, center, axis0, axis1, scale0, scale1);
            inv_GLI_fn_vSetQuadColor(quad, &batchColor);
        }
        selected = inv_GLI_renderContext;
    } else {
        GliVertexColor directColor = selected->vertexColor;
        Quad* quad = &inv_GLI_singleQuad;

        inv_GLI_fn_vSetOrientedSpriteCorners(quad, center, axis0, axis1, scale0, scale1);
        inv_GLI_fn_vSetQuadColor(quad, &directColor);
        inv_RW3_fn_vRenderWorldOrthoPlanarSprites(quad, 1);
        selected = inv_GLI_renderContext;
    }

    selected->unk203C = 0;
    inv_GLI_fn_vRestoreRenderFlags(savedFlags);
    g_bTakeCareOfAnimationCoordination = 0;
    g_bOneShotAnim = 0;
    return g_bHasTextureAnimationFinished;
}

void inv_RW3_fn_vRenderWorldOrthoPlanarSprites(Quad* quads, s32 quadCount)
{
    s32 indicesPerQuad;

    inv_RW3_IM3D_fn_bTransform(quads, quadCount << 2, &inv_RW3_stWorldOriginMatrix, 1);
    indicesPerQuad = 6;
    quadCount = quadCount * indicesPerQuad;
    inv_RW3_IM3D_fn_bBuildIndicesForPrimType(4, inv_RW3_a_wIm3DIndices, quadCount - 2);
    inv_RW3_IM3D_fn_bEnd();
}
