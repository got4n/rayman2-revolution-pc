#include "common.h"

#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "ps2/r5900.h"
#include "renderware.h"
#include "COL/COL.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"

#define MAX_MORPH_VERTICES 0x400
#define HALO_FADE_FRAMES 6
#define HALO_BLEND_STATE 2
#define MAX_POINT_LIGHTS 5

#define GLI_LIGHT_TYPE_1 1
#define GLI_LIGHT_TYPE_2 2
#define GLI_LIGHT_TYPE_3 3
#define GLI_LIGHT_TYPE_4 4
#define GLI_LIGHT_HOTSPOT 5
#define GLI_LIGHT_FOG 6

#define RP_LIGHT_DIRECTIONAL 1
#define RW3_LIGHT_TYPE_SINGLE 2
#define RW3_LIGHT_TYPE_80 0x80

typedef struct LightHaloEntry {
    f32 x;
    f32 y;
    f32 halfWidth;
    f32 halfHeight;
    u32 color;
    u32 startFrame;
    u32 lastFrame;
    u32 visible;
} LightHaloEntry;

extern MTH3D_tdstVector_ inv_RW3_a_stMorphVertexBuffer[MAX_MORPH_VERTICES];
extern u8 inv_RW3_ucQueuedHaloCount;
extern s32 inv_RW3_worldLightCount;

#ifdef PC_PORT
u32 fptoui(f32 value);
#endif
MTH3D_tdstVector_* RpMorphTargetGetVertices(RpMorphTarget* morphTarget);
s32 RpGeometryGetNumVertices(RpGeometry* geometry);
RpLight* RpLightCreate(s32 type);
RpLight* RpLightSetColor(RpLight* light, GEO_tdstColor_* color);
RpLight* RpLightSetFrame(RpLight* light, RwFrame* frame);
void RpLightSetFlags(RpLight* light, s32 flags);
s32 RpLightGetFlags(RpLight* light);
void RpLightSetRadius(RpLight* light, f32 radius);
void RpLightSetConeAngle(RpLight* light, f32 angle);
RwFrame* RwFrameCreate(void);
RwMatrix* RwFrameGetMatrix(RwFrame* frame);
void inv_RW3_fn_vApplySinusEffect(MTH3D_tdstVector_* source, MTH3D_tdstVector_* destination, s32 count);
void inv_RW3_fn_vUpdateAtomicVertices(RW3ObjectView* object, MTH3D_tdstVector_* vertices);

extern MTH3D_tdstVector_ a3_xSinEffectBaseMesh;

void inv_RW3_fn_vApplySinusEffect(MTH3D_tdstVector_* source, MTH3D_tdstVector_* destination, s32 count)
{
    POS_tdstMatrix_ matrix;
    POS_tdstMatrix_ inverse;
    GLI_tdstRenderContext* context = inv_GLI_renderContext;
    MTH3D_tdstVector_* vertex;
    f32 phaseX;
    f32 phaseY;
    f32 phaseZ;
    s32 remaining;

    if (context->inv_bUseSinEffectBase != 0) {
        phaseX = a3_xSinEffectBaseMesh.xX + GLI_fSinEffectBase;
        phaseY = a3_xSinEffectBaseMesh.xY + GLI_fSinEffectBase;
        phaseZ = a3_xSinEffectBaseMesh.xZ + GLI_fSinEffectBase;
    } else {
        phaseX = a3_xSinEffectBaseMesh.xX + GLI_fF2pow24;
        phaseY = a3_xSinEffectBaseMesh.xY + GLI_fF2pow24;
        phaseZ = a3_xSinEffectBaseMesh.xZ + GLI_fF2pow24;
    }
    _POS_fn_vMulMatrixMatrix(&matrix, context->geometryMatrix, context->currentMatrix);
    POS_fn_vInvertMatrix(&inverse, &matrix);
    POS_fn_vMulMatrixVertexArray(destination, &matrix, source, count);
    remaining = count;
    vertex = destination;
    do {
        f32 y = vertex->xY;
        f32 x = vertex->xX;
        f32 z = vertex->xZ;
        FloatBits angleX;
        FloatBits angleY;
        FloatBits angleZ;
        f32 newX;
        f32 newY;
        f32 newZ;

        angleZ.value = phaseZ + y + x;
        angleX.value = phaseX + y + z;
        angleY.value = phaseY + x + z;
        newX = x + a64_fSinTable[angleX.bits & 0x3F] * stSinEffectAmp.xX;
        newY = y + a64_fSinTable[angleY.bits & 0x3F] * stSinEffectAmp.xY;
        newZ = z + a64_fSinTable[angleZ.bits & 0x3F] * stSinEffectAmp.xZ;
        MTH3D_M_vSetXYZ_inv(vertex, newX, newY, newZ);
        vertex++;
    } while (--remaining != 0);
    POS_fn_vMulMatrixVertexArray(destination, &inverse, destination, count);
}

void inv_RW3_fn_vUpdateAtomicVertices(RW3ObjectView* object, MTH3D_tdstVector_* vertices)
{
    s32 index;
    RpAtomic** atomics;
    u16** indexLists;
    RpGeometry* geometry;
    MTH3D_tdstVector_* morphVertices;
    RpMorphTarget* morphTarget;
    u16* sourceIndex;
    s32 vertexCount;
    s32 vertexIndex;

    atomics = object->atomics;
    indexLists = object->vertexIndices;
    morphVertices = NULL;
    for (index = 0; index < object->atomicCount; index++) {
        geometry = (*atomics)->geometry;
        morphTarget = RpGeometryGetMorphTarget(geometry, 0);
        if (morphTarget != NULL) {
            morphVertices = RpMorphTargetGetVertices(morphTarget);
            sourceIndex = *indexLists;
            RpGeometryLock(geometry, 6);
            vertexCount = RpGeometryGetNumVertices(geometry);
            for (vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {
                MTH3D_M_vCopyXYZ_inv(&morphVertices[vertexIndex], &vertices[*sourceIndex]);
                sourceIndex++;
            }
            RpGeometryUnlock(geometry);
        }
        atomics++;
        indexLists++;
    }
}

void inv_RW3_fn_vSendDeformedObject(GeometricObjectView* object)
{
    s16 count = object->vertexCount;

    if (count <= MAX_MORPH_VERTICES) {
        inv_RW3_fn_vApplySinusEffect(object->vertices, inv_RW3_a_stMorphVertexBuffer, count);
        inv_RW3_fn_vUpdateAtomicVertices(object->rw3Object, inv_RW3_a_stMorphVertexBuffer);
    }
}

static inline GLI_tdstCamera_* inv_GLI_fn_p_stCurrentCamera(void)
{
    return inv_GLI_renderContext->camera;
}

static inline void inv_GLI_fn_vSelectHaloMaterial(void)
{
    GLI_tdstRenderContext* context = inv_GLI_renderContext;
    GliMaterialData* material = inv_GLI_p_stHaloMaterial;
    s32 flags;

    context->currentMaterial = material;
    flags = material->type;
    context->lHierachDrawMask = flags;
    context->lCurrentDrawMask = flags;
}

static inline void inv_GLI_fn_vSelectHaloTexture(void)
{
    GLI_tdstRenderContext* context = inv_GLI_renderContext;
    GliMaterialData* material = inv_GLI_p_stHaloMaterial;

    context->currentMaterial = material;
    context->currentTexture = material->layers[0].texture;
}

void inv_RW3_fn_vDrawQueuedHalos(struct GLD_tdstViewportAttributes_* viewport)
{
    u32 frame;
    u8 i;
    s32 blendState;

    if (inv_RW3_ucQueuedHaloCount == 0) {
        return;
    }

    blendState = HALO_BLEND_STATE;
    frame = inv_fn_ulGetEngineTrameNumber();
    inv_GLI_fn_vSelectHaloMaterial();
    GLI_RW_vFogOff();

    if (inv_GLI_blendModeState != blendState) {
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDONE);
        inv_GLI_blendModeState = blendState;
    }

    i = 0;
    GLI_vFogOff();
    inv_GLI_fn_vSelectHaloTexture();
    GLI_RW_vDoTextureSelection();

    if (inv_RW3_ucQueuedHaloCount != 0) {
        do {
            LightHaloEntry* entry;
            f32 alpha;
            u32 start;

            entry = &inv_RW3_p_stLightHalos[inv_RW3_p_ucQueuedHalos[i]];

            if (entry->lastFrame + 1 != frame) {
                if (entry->visible != 0) {
                    entry->startFrame = frame;
                    entry->lastFrame = frame;
                }
                alpha = 0.0f;
            } else {
                start = entry->startFrame;
                entry->lastFrame = frame;
                if (frame < start + HALO_FADE_FRAMES) {
                    if (entry->visible != 0) {
                        f32 elapsed;

                        elapsed = (f32) (frame - start);
                        alpha = elapsed / 6.0f;
                    } else {
                        alpha = (f32) (frame - start);
                        alpha = alpha / 6.0f;
                        alpha = 1.0f - alpha;
                    }
                } else {
                    if (entry->visible != 0) {
                        alpha = 1.0f;
                    } else {
                        alpha = 0.0f;
                    }
                }
            }

            if (alpha > 0.0f) {
                f32 y;
                f32 x;
                f32 bottom;
                f32 right;
                f32 halfWidth;
                f32 halfHeight;
                u32 color;

                x = entry->x;
                halfWidth = entry->halfWidth;
                halfHeight = entry->halfHeight;
                y = entry->y;
                bottom = y + halfHeight;
                right = x + halfWidth;
                x = x - halfWidth;
                y = y - halfHeight;
#ifdef PC_PORT
                color = entry->color | (fptoui(alpha * 255.0f) << 24);
#else
                color = entry->color | ((u32) (alpha * 255.0f) << 24);
#endif
                inv_GLI_fn_vDraw2DTexturedRectWithColor(viewport, color, x, y, right, bottom);
            }

            i = (u8) (i + 1);
        } while (i < inv_RW3_ucQueuedHaloCount);
    }

    inv_RW3_ucQueuedHaloCount = 0;
    GLI_RW_vFogOn();

    if (inv_GLI_blendModeState != 0) {
        s32 one;

        inv_GLI_blendModeState = 0;
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        one = 1;
        inv_GLI_zTestState = one;
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = one;
        inv_GLI_blendModeState = 0;
    }

    inv_GLI_RW_vFlush2DVertexBuffer();
}

void inv_RW3_fn_vQueueHalo(GeometricObjectView* source, SpriteElement* sprite)
{
    MTH3D_tdstVector_ screen;
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ copy;
    OctreeSegment segment;
    LightHaloEntry* entry;
    MTH3D_tdstVector_* output;
    MTH3D_tdstVector_* input;
    MTH4D_tdstMatrix_* matrix;
    MTH3D_tdstVector_* copyPointer;
    s32 frame;
    u8 index;
    f32 inverse;
    f32 one;
    f32 half;
    f32 scaleX;
    f32 scaleY;
    f32 u;
    f32 v;

    index = sprite->u.inv_ucHaloIndex;
    frame = inv_fn_ulGetEngineTrameNumber();
    copyPointer = &copy;
    position = *source->vertices;
    entry = &inv_RW3_p_stLightHalos[index];
    output = &screen;
    matrix = &inv_GLI_renderContext->geometryMatrix->stMatrix;
    input = &position;
    if (((s32) input ^ (s32) output) == 0) {
        copy = *input;
        R5900_TRANSFORM_POINT_inv(output, copyPointer, matrix);
    } else {
        R5900_TRANSFORM_POINT_inv(output, input, matrix);
    }
    if (screen.xZ < inv_GLI_fn_p_stCurrentCamera()->xNear) {
        return;
    }
    one = 1.0f;
    half = 0.5f;
    screen.xZ = one / (screen.xZ + screen.xZ);
    scaleX = inv_GLI_fn_p_stCurrentCamera()->xXProjectionRatio * screen.xZ;
    entry->halfWidth = scaleX;
    scaleY = inv_GLI_fn_p_stCurrentCamera()->xYProjectionRatio * screen.xZ;
    entry->halfHeight = scaleY;
    u = half - screen.xX * scaleX;
    entry->x = u;
    v = half - screen.xY * scaleY;
    entry->y = v;
    entry->halfWidth = scaleX * sprite->halfWidth;
    entry->halfHeight = scaleY * sprite->halfHeight;
    if (one < u || u < 0.0f || one < v || v < 0.0f) {
        if (entry->visible != 0) {
            entry->startFrame = frame;
            entry->visible = 0;
        }
    } else if (half < screen.xZ) {
        if (entry->visible != 1) {
            entry->startFrame = frame;
            entry->visible = 1;
        }
    } else if ((inv_RW3_ucQueuedHaloCount ^ frame) & 1) {
        POS_tdstMatrix_* cameraMatrix;
        s32 visible;
        f32 dx, dy, dz, squared, length;

        segment.end = position;
        cameraMatrix = CAM_fn_hGetActiveCamera(1)->gu_hLocalMatrix;
        segment.start = *inv_POS_fn_p_stPosMatrixTranslation(cameraMatrix);
        dx = segment.start.xX - position.xX;
        dy = segment.start.xY - position.xY;
        dz = segment.start.xZ - position.xZ;
        MTH3D_M_vSetXYZ_inv(&copy, dx, dy, dz);
        squared = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
        length = MTH_fn_xSqrtOpt2(squared);
        if (length != 0.0f) {
            inverse = one / length;
            MTH3D_M_vSetXYZ_inv(&copy, inverse * dx, inverse * dy, inverse * dz);
        }
        inv_MTH3D_fn_vAddXYZ(&segment.end, &position, &copy);
        visible = inv_COL_fn_bIntersectSegmentWithLevel(&segment) == 0;
        if (entry->visible != visible) {
            entry->startFrame = frame;
            entry->visible = visible;
        }
    }
    inv_RW3_p_ucQueuedHalos[inv_RW3_ucQueuedHaloCount] = index;
    inv_RW3_ucQueuedHaloCount++;
}

void RW3_Light_UpdateMatrixFromGLILightMatrix(GLI_tdstLight_* light)
{
    RpLight* backendLight = light->backendLight;

    if (backendLight != NULL && backendLight != inv_RW3_singleLight) {
        RwFrame* frame = RpLightGetFrame(backendLight);

        RwFrameGetMatrix(frame);
        RW3_UpdateFrameWithGeometricObjectMatrix(frame, &light->stMatrix);
    }
}

void RW3_Light_SetColor(GLI_tdstLight_* light, GEO_tdstColor_* color)
{
    RpLight* backendLight = light->backendLight;

    if (backendLight != NULL) {
        RpLightSetColor(backendLight, color);
    }
}

void RW3_Light_SetAngle(GLI_tdstLight_* light)
{
    if (light->backendLight != NULL) {
        RpLightSetConeAngle(light->backendLight, light->xBigAlpha);
    }
}

void RW3_Light_SetRadius(GLI_tdstLight_* light)
{
    if (light->backendLight != NULL) {
        RpLightSetRadius(light->backendLight, light->xFar);
    }
}

void inv_RW3_fn_vResetEnabledLights(void)
{
    inv_RW3_enabledPointLights = 0;
    inv_RW3_enabledDirectionalLights = 0;
}

void RW3_Light_CreateFromGLILight(GLI_tdstLight_* light)
{
    s32 type;
    RpLight* backendLight;

    light->backendLight = NULL;
    switch (light->lTypeOfLight) {
    case GLI_LIGHT_TYPE_1:
        type = RP_LIGHT_DIRECTIONAL;
        break;
    case GLI_LIGHT_TYPE_2:
        type = RW3_LIGHT_TYPE_80;
        break;
    case GLI_LIGHT_TYPE_3:
        return;
    case GLI_LIGHT_TYPE_4:
        type = RW3_LIGHT_TYPE_SINGLE;
        if (inv_RW3_singleLight != NULL) {
            light->backendLight = inv_RW3_singleLight;
            return;
        }
        break;
    case GLI_LIGHT_HOTSPOT:
        return;
    case GLI_LIGHT_FOG:
        GLI_vSetLightSwitch(light, 0);
        return;
    default:
        return;
    }
    backendLight = RpLightCreate(type);
    if (backendLight == NULL) {
        return;
    }
    light->backendLight = backendLight;
    if (type == RW3_LIGHT_TYPE_SINGLE) {
        inv_RW3_singleLight = backendLight;
    }
    RpLightSetColor(backendLight, &light->stColor);
    RpLightSetFlags(backendLight, 0);
    if (type != RW3_LIGHT_TYPE_SINGLE) {
        RpLightSetRadius(backendLight, light->xFar);
        RpLightSetConeAngle(backendLight, light->xBigAlpha);
        RpLightSetFrame(backendLight, RwFrameCreate());
        RW3_Light_UpdateMatrixFromGLILightMatrix(light);
    }
    RpWorldAddLight(RW3_g_pWorld, backendLight);
    inv_RW3_worldLightCount = inv_RW3_worldLightCount + 1;
}

void inv_RW3_fn_vSetSingleLightColor(void* color)
{
    RpLight* light = inv_RW3_singleLight;

    if (light == NULL) {
        return;
    }
    RpLightSetColor(light, color);
    RpLightSetFlags(inv_RW3_singleLight, 1);
}

void inv_RW3_fn_vDisableSingleLight(void)
{
    RpLight* light = inv_RW3_singleLight;

    if (light != NULL) {
        RpLightSetFlags(light, 0);
    }
}

void inv_RW3_fn_vClearLightList(LevelLightList* list)
{
    list->count = 0;
    memset(list->lights, 0, sizeof(list->lights));
}

void inv_RW3_fn_vSetViewportLightMask(s32 mask)
{
    inv_RW3_ulViewportLightMask = mask;
}

void func_0014EA40(void)
{
    GLI_tdstLight_** viewportLight;
    s32 index;
    s32 count;
    GLI_tdstLight_* light;
    RpLight* backendLight;
    s32 type;
    s32 flags;
    s32 enabled;

    viewportLight = gs_aDEF_p_CurrentListOfLight;
    count = gs_CurrentNumberOfLightInViewport;
    index = 0;
    if (count > 0) {
        do {
            if (((inv_RW3_ulViewportLightMask >> index) & 1) != 0) {
                light = *viewportLight;
                backendLight = light->backendLight;
                if (backendLight != NULL) {
                    if (backendLight == inv_RW3_singleLight) {
                        RpLightSetColor(backendLight, &light->stColor);
                        RpLightSetFlags(backendLight, 1);
                        count = gs_CurrentNumberOfLightInViewport;
                    } else {
                        type = RpLightGetType(backendLight);
                        flags = RpLightGetFlags(backendLight);
                        enabled = 1;
                        if (flags == enabled) {
                            count = gs_CurrentNumberOfLightInViewport;
                        } else if (type == enabled) {
                            if (inv_RW3_enabledDirectionalLights == 0) {
                                RpLightSetFlags(backendLight, 1);
                                enabled = inv_RW3_enabledDirectionalLights;
                                count = gs_CurrentNumberOfLightInViewport;
                                enabled = enabled + 1;
                                inv_RW3_enabledDirectionalLights = enabled;
                            } else {
                                count = gs_CurrentNumberOfLightInViewport;
                            }
                        } else if ((u32) inv_RW3_enabledPointLights < MAX_POINT_LIGHTS) {
                            RpLightSetFlags(backendLight, 1);
                            enabled = inv_RW3_enabledPointLights;
                            count = gs_CurrentNumberOfLightInViewport;
                            enabled = enabled + 1;
                            inv_RW3_enabledPointLights = enabled;
                        } else {
                            count = gs_CurrentNumberOfLightInViewport;
                        }
                    }
                }
            }
            index = index + 1;
            viewportLight = viewportLight + 1;
        } while (index < count);
    }
}

void inv_RW3_fn_vCopyViewportLights(LevelLightList* list)
{
    GLI_tdstLight_** destination;
    GLI_tdstLight_** source;
    s32 index;
    s32 count;

    list->count = 0;
    source = gs_aDEF_p_CurrentListOfLight;
    destination = list->lights;
    count = gs_CurrentNumberOfLightInViewport;
    for (index = 0; index < count; source = source + 1) {
        if (((u32) (inv_RW3_ulViewportLightMask >> index) & 1) != 0) {
            if (list->count < LEVEL_LIGHT_COUNT) {
                *destination = *source;
                destination = destination + 1;
                list->count = list->count + 1;
                count = gs_CurrentNumberOfLightInViewport;
            }
        }
        index = index + 1;
    }
}

RpLight* RW3_Light_RemoveFromWorld(RpLight* light)
{
    s32 type;
    s32 flags;

    if (light == NULL) {
        return NULL;
    }
    if (light != inv_RW3_singleLight) {
        type = RpLightGetType(light);
        flags = RpLightGetFlags(light);
        if (flags != 0) {
            if (type == RP_LIGHT_DIRECTIONAL) {
                inv_RW3_enabledDirectionalLights -= 1;
            } else {
                inv_RW3_enabledPointLights -= 1;
            }
            RpLightSetFlags(light, 0);
            return light;
        }
    } else {
        RpLightSetFlags(light, 0);
    }
    return light;
}

typedef void* (*LightCallback)(void* light);

void RpWorldForAllLights(void* world, LightCallback callback, void* data);

void inv_GLI_vPopLightsInViewport(void)
{
    RpWorldForAllLights(RW3_g_pWorld, (LightCallback) RW3_Light_RemoveFromWorld, NULL);
}
