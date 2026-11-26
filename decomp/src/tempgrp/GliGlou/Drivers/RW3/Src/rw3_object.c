#include "common.h"

#include "GLI/GLI.h"
#include "renderware.h"
#include "GLI/GLD.h"
#include "MTH/MTH.h"
#include "ps2/libc.h"
#include "rwps2.h"
#include "GAM/GAM.h"
#include "IPT/IPT.h"

#define GEOMETRIC_OBJECT_CHILD_FRAMES 0x8
#define GEOMETRIC_ELEMENT_ATOMIC 1
#define RW3_MATRIX_FLAGS 0x20003

RwMatrix* RwFrameGetMatrix(RwFrame* frame);
void RwFrameUpdateObjects(RwFrame* frame);
void inv_RW3_fn_vSetGeometryMaterial(RpGeometry* geometry, GeometricObjectView* object, s32 element);
void inv_RW3_fn_vSetAtomicLightmap(RpAtomic* atomic, s32 slot);
RwFrame* RwFrameCreate(void);

void RW3_CreateRW3ObjectFromGeometric(GeometricObjectView* object)
{
    RW3ObjectView* rw3Object = object->rw3Object;
    u32 elementCount = object->elementCount;
    s32 atomicCount = 0;
    u32 element;
    s32 flags;
    RwFrame* frame;
    RwMatrix* matrix;

    if (rw3Object == NULL) {
        return;
    }
    do {
        frame = RwFrameCreate();
        rw3Object->frame = frame;
    } while (0);
    RwFrameGetMatrix(frame)->right.x = RwFrameGetMatrix(rw3Object->frame)->up.y =
        RwFrameGetMatrix(rw3Object->frame)->at.z = 1.0f;
    RwFrameGetMatrix(rw3Object->frame)->right.y = RwFrameGetMatrix(rw3Object->frame)->right.z =
        RwFrameGetMatrix(rw3Object->frame)->up.x = 0.0f;
    RwFrameGetMatrix(rw3Object->frame)->up.z = RwFrameGetMatrix(rw3Object->frame)->at.x =
        RwFrameGetMatrix(rw3Object->frame)->at.y = 0.0f;
    RwFrameGetMatrix(rw3Object->frame)->pos.x = RwFrameGetMatrix(rw3Object->frame)->pos.y =
        RwFrameGetMatrix(rw3Object->frame)->pos.z = 0.0f;
    matrix = RwFrameGetMatrix(rw3Object->frame);
    flags = RwFrameGetMatrix(rw3Object->frame)->flags;
    flags |= RW3_MATRIX_FLAGS;
    do {
        matrix->flags = flags;
    } while (0);
    RwFrameTranslate(rw3Object->frame, &inv_RW3_stWorldOrigin, 1);
    RwFrameUpdateObjects(rw3Object->frame);
    flags = object->flags;
    flags &= GEOMETRIC_OBJECT_CHILD_FRAMES;
    if (flags != 0) {
        rw3Object->mode = 2;
    } else {
        rw3Object->mode = 1;
    }

    for (element = 0; element < elementCount; element++) {
        RpAtomic* atomic;

        if (object->elementTypes[element] != GEOMETRIC_ELEMENT_ATOMIC) {
            continue;
        }
        atomic = func_0014DC70();
        inv_RW3_fn_vSetGeometryMaterial(atomic->geometry, object, element);
        RpWorldAddAtomic(RW3_g_pWorld, atomic);
        flags = object->flags;
        flags &= GEOMETRIC_OBJECT_CHILD_FRAMES;
        if (flags == 0) {
            s16 slot;

            RpAtomicSetFrame(atomic, rw3Object->frame);
            slot = object->elements[element]->lightmapSlot;
            if (slot != -1 && inv_RW3_lightmapSlots[slot].r != NULL) {
                inv_RW3_fn_vSetAtomicLightmap(atomic, slot);
            }
        } else {
            RpAtomicSetFrame(atomic, RwFrameCreate());
            RwFrameAddChild(rw3Object->frame, atomic->object.object.parent);
        }
        rw3Object->atomics[atomicCount] = atomic;
        rw3Object->atomicStates[atomicCount] = 0;
        atomicCount++;
    }
}

#define RW3_OBJECT_SHARED_FRAME 1
#define LIGHTMAP_SLOT_NONE 0xFFFFFFFF
#define LIGHTMAP_PLUGIN_ID 0x732
#define WORLD_DEFAULT_EXTENT 4000.0f

extern RwCamera* inv_GLI_secondaryCamera;
extern u8 D_0039371E;

RwMatrix* RwMatrixUpdate(RwMatrix* matrix);
s32 RpAtomicDestroy(RpAtomic* atomic);
void RpWorldRemoveAtomic(RpWorld* world, RpAtomic* atomic);
s32 RpAtomicRegisterPlugin(s32 size, u32 pluginId, RwPluginCallback constructor,
                           RwPluginCallback destructor, RwPluginCallback copy);
RpWorld* RpWorldCreate(RwBBox* bounds);
void RpWorldAddCamera(RpWorld* world, RwCamera* camera);
RwV3d* RpWorldGetOrigin(RpWorld* world);
s32 func_00178C20(RpAtomic* atomic, void* pipeline);
s32 inv_RW3_fn_lGetAtomicLightmapSlot(RpAtomic* atomic);

static inline void inv_GLI_fn_vAddVector(RwV3d* vector, const RwV3d* offset)
{
    vector->x += offset->x;
    vector->y += offset->y;
    vector->z += offset->z;
}

static inline void inv_GLI_fn_vTransformPoint(MTH4D_tdstVector_* out, const MTH4D_tdstMatrix_* matrix,
                                  const MTH4D_tdstVector_* point)
{
    f32 x;
    f32 y;
    f32 z;

    x = inv_MTH_fn_xMulAdd3Grouped(matrix->stCol_0.xX, matrix->stCol_1.xX, matrix->stCol_2.xX,
                         point->xX, point->xY, point->xZ);
    out->xX = x;
    y = inv_MTH_fn_xMulAdd3Grouped(matrix->stCol_0.xY, matrix->stCol_1.xY, matrix->stCol_2.xY,
                         point->xX, point->xY, point->xZ);
    out->xY = y;
    z = inv_MTH_fn_xMulAdd3Grouped(matrix->stCol_0.xZ, matrix->stCol_1.xZ, matrix->stCol_2.xZ,
                         point->xX, point->xY, point->xZ);
    out->xZ = z;
    out->xW = 1.0f;
}

static inline u32* inv_GLI_fn_p_ulLightmapSlotOf(void* atomic)
{
    u8* base = atomic;
    void* slot = base + inv_RW3_lLightmapAtomicPluginOffset;

    return slot;
}

void RW3_DestroyRW3Object(RW3ObjectView* object)
{
    u32 i = 0;
    RpAtomic** atomic;
    RwFrame* root;
    RwFrame* frame;
    s32 slot;

    if (object == NULL) {
        return;
    }
    atomic = object->atomics;
    root = object->frame;
    for (; i < object->atomicCount; atomic++) {
        if (!(object->mode & RW3_OBJECT_SHARED_FRAME)) {
            frame = (*atomic)->object.object.parent;
            RpAtomicSetFrame(*atomic, NULL);
            RwFrameRemoveChild(frame);
            RwFrameDestroy(frame);
        } else {
            slot = inv_RW3_fn_lGetAtomicLightmapSlot(*atomic);
            if (slot != -1) {
                RwRasterDestroy(inv_RW3_lightmapSlots[slot].r);
                RwRasterDestroy(inv_RW3_lightmapSlots[slot].g);
                RwRasterDestroy(inv_RW3_lightmapSlots[slot].b);
            }
            RpAtomicSetFrame(*atomic, NULL);
        }
        i++;
        RpWorldRemoveAtomic(RW3_g_pWorld, *atomic);
        RpAtomicDestroy(*atomic);
    }
    RwFrameDestroy(root);
}

void inv_RW3_fn_vSetGeometryMaterial(RpGeometry* geometry, GeometricObjectView* object, s32 element)
{
    GliMaterialData* material = object->elements[element]->material;
    RpGeometry* locked = RpGeometryLock(geometry, 1);
    RpMaterial* previous = RpGeometryGetMaterial(locked, 0);

    locked->matList.materials[0] = material->layers[0].rpMaterial;
    locked->matList.materials[0]->refCount += previous->refCount;
    RpGeometryUnlock(locked);
    previous->refCount = 1;
    RpMaterialDestroy(previous);
}

void RW3_UpdateFrameWithGeometricObjectMatrix(RwFrame* frame, const POS_tdstMatrix_* source)
{
    RwMatrix* matrix = RwFrameGetMatrix(frame);
    RwV3d* up = &matrix->up;
    RwV3d* at = &matrix->at;
    RwV3d* pos = &matrix->pos;
    const MTH4D_tdstMatrix_* rotation = &source->stMatrix;

    matrix->right.x = rotation->stCol_0.xX;
    matrix->right.y = rotation->stCol_0.xY;
    matrix->right.z = rotation->stCol_0.xZ;
    up->x = rotation->stCol_1.xX;
    up->y = rotation->stCol_1.xY;
    up->z = rotation->stCol_1.xZ;
    at->x = rotation->stCol_2.xX;
    at->y = rotation->stCol_2.xY;
    at->z = rotation->stCol_2.xZ;
    pos->x = rotation->stCol_3.xX;
    pos->y = rotation->stCol_3.xY;
    pos->z = rotation->stCol_3.xZ;
    inv_GLI_fn_vAddVector(pos, &inv_RW3_stWorldOrigin);
    RwMatrixUpdate(matrix);
    RwFrameUpdateObjects(frame);
}

void inv_RW3_fn_vSelectViewportCamera(GLD_tdstViewportAttributes_* attributes)
{
    GLI_tdstSpecificAttributesFor3D_* cell;
    GLI_tdstCamera_* camera;
    MTH4D_tdstMatrix_* source;
    RwFrame* frame;
    RwMatrix* matrix;
    RwV3d* up;
    RwV3d* at;
    RwV3d* pos;
    MTH4D_tdstVector_ position;
    MTH4D_tdstMatrix_ inverse;

    if (inv_GLI_currentCamera == NULL) {
        return;
    }
    if (attributes == &g_stEngineStructure.stFixViewportAttr) {
        inv_GLI_currentCamera = inv_GLI_secondaryCamera;
    } else {
        inv_GLI_currentCamera = inv_GLI_primaryCamera;
    }
    cell = attributes->p_vSpecificToXD;
    camera = cell->p_stCam;
    source = &camera->stMatrix.stMatrix;
    if (source == NULL) {
        return;
    }
    frame = RwCameraGetFrame(inv_GLI_currentCamera);
    if (frame == NULL) {
        return;
    }
    matrix = RwFrameGetMatrix(frame);
    up = &matrix->up;
    at = &matrix->at;
    pos = &matrix->pos;
    matrix->right.x = source->stCol_0.xX;
    matrix->right.y = source->stCol_1.xX;
    matrix->right.z = source->stCol_2.xX;
    up->x = source->stCol_0.xY;
    up->y = source->stCol_1.xY;
    up->z = source->stCol_2.xY;
    at->x = source->stCol_0.xZ;
    at->y = source->stCol_1.xZ;
    at->z = source->stCol_2.xZ;
    MTH3D_M_vSetXYZ_inv(&inverse.stCol_0, source->stCol_0.xX, source->stCol_1.xX,
                        source->stCol_2.xX);
    inverse.stCol_0.xW = 0;
    MTH3D_M_vSetXYZ_inv(&inverse.stCol_1, source->stCol_0.xY, source->stCol_1.xY,
                        source->stCol_2.xY);
    inverse.stCol_1.xW = 0;
    MTH3D_M_vSetXYZ_inv(&inverse.stCol_2, source->stCol_0.xZ, source->stCol_1.xZ,
                        source->stCol_2.xZ);
    inverse.stCol_2.xW = 0;
    inverse.stCol_3 = source->stCol_3;
    inv_GLI_fn_vTransformPoint(&position, &inverse, &source->stCol_3);
    pos->x = -position.xX;
    pos->y = -position.xY;
    pos->z = -position.xZ;
    inv_GLI_fn_vAddVector(pos, &inv_RW3_stWorldOrigin);
    RwMatrixUpdate(matrix);
    RwFrameUpdateObjects(frame);
}

void inv_RW3_fn_vCreateWorld(RwBBox* bounds)
{
    RwBBox box;

    if (bounds == NULL) {
        box.inf.z = -WORLD_DEFAULT_EXTENT;
        box.inf.y = -WORLD_DEFAULT_EXTENT;
        box.sup.z = WORLD_DEFAULT_EXTENT;
        box.inf.x = -WORLD_DEFAULT_EXTENT;
        box.sup.y = WORLD_DEFAULT_EXTENT;
        box.sup.x = WORLD_DEFAULT_EXTENT;
    } else {
        box = *bounds;
    }
    if (RW3_g_pWorld != NULL) {
        RpWorldDestroy(RW3_g_pWorld);
    }
    RW3_g_pWorld = RpWorldCreate(&box);
    RpWorldSetFlags(RW3_g_pWorld, 0x34);
    RpWorldSetRenderOrder(RW3_g_pWorld, 2);
    if (inv_RW3_unsortedClump == NULL) {
        inv_RW3_unsortedClump = RpClumpCreate();
    }
    RpWorldAddClump(RW3_g_pWorld, inv_RW3_unsortedClump);
    if (inv_RW3_sortedClump == NULL) {
        inv_RW3_sortedClump = RpClumpCreate();
    }
    RpWorldAddClump(RW3_g_pWorld, inv_RW3_sortedClump);
    RpWorldAddCamera(RW3_g_pWorld, inv_GLI_primaryCamera);
    RpWorldAddCamera(RW3_g_pWorld, inv_GLI_secondaryCamera);
    inv_RW3_stWorldOrigin = *RpWorldGetOrigin(RW3_g_pWorld);
    func_001519F0();
    func_001519D0();
}

void func_00150268(void)
{
    if (D_0039371E == (D_00393C10 == 0)) {
        return;
    }
    if (IPT_fn_bIsJoystickButtonJustPressed(0, 0x1000)) {
        D_0039371D--;
        if (D_0039371D < 0) {
            D_0039371D = 2;
        }
    }
    D_0039371E = D_00393C10 == 0;
    if (D_0039371D == 1) {
        D_003936FC = 1;
    } else {
        D_003936FC = 0;
    }
}

void inv_RW3_fn_vCreateLightmapTemplateRaster(RwRaster** raster)
{
    *raster = RwRasterCreate(8, 8, 0x20, 0x504);
    memset(RwRasterLock(*raster, 0, 1), 0x60, 0x100);
    RwRasterUnlock(*raster);
}

void* inv_RW3_fn_pLightmapEngineConstructor(void* object, s32 offset, s32 size)
{
    return object;
}

void* inv_RW3_fn_pLightmapEngineDestructor(void* object, s32 offset, s32 size)
{
    return object;
}

void* inv_RW3_fn_pLightmapAtomicConstructor(void* object, s32 offset, s32 size)
{
    *inv_GLI_fn_p_ulLightmapSlotOf(object) = LIGHTMAP_SLOT_NONE;
    return object;
}

void* inv_RW3_fn_pLightmapAtomicDestructor(void* object, s32 offset, s32 size)
{
    return object;
}

void* inv_RW3_fn_pLightmapAtomicCopy(void* object, s32 offset, s32 size)
{
    return object;
}

void inv_RW3_fn_vRegisterLightmapPlugin(void)
{
    if (RwEngineRegisterPlugin(0, LIGHTMAP_PLUGIN_ID, inv_RW3_fn_pLightmapEngineConstructor, inv_RW3_fn_pLightmapEngineDestructor) < 0) {
        return;
    }
    inv_RW3_lLightmapAtomicPluginOffset =
        RpAtomicRegisterPlugin(4, LIGHTMAP_PLUGIN_ID, inv_RW3_fn_pLightmapAtomicConstructor, inv_RW3_fn_pLightmapAtomicDestructor, inv_RW3_fn_pLightmapAtomicCopy);
    printf("INFO --> Atomic Extended via PlugIn\n");
    func_00150498();
}

void inv_RW3_fn_vSetAtomicLightmap(RpAtomic* atomic, s32 slot)
{
    if (inv_RW3_lightmapTemplateRasterR == NULL) {
        inv_RW3_fn_vCreateLightmapTemplateRaster(&inv_RW3_lightmapTemplateRasterR);
        inv_RW3_fn_vCreateLightmapTemplateRaster(&inv_RW3_lightmapTemplateRasterG);
        inv_RW3_fn_vCreateLightmapTemplateRaster(&inv_RW3_lightmapTemplateRasterB);
    }
    func_00178C20(atomic, inv_RW3_p_stLightmapPipeline);
    *inv_GLI_fn_p_ulLightmapSlotOf(atomic) = slot;
    D_0039371C = 1;
}

void inv_RW3_fn_vSetAtomicLightmapSlot(RpAtomic* atomic, s32 slot)
{
    *inv_GLI_fn_p_ulLightmapSlotOf(atomic) = slot;
}

s32 inv_RW3_fn_lGetAtomicLightmapSlot(RpAtomic* atomic)
{
    return *inv_GLI_fn_p_ulLightmapSlotOf(atomic);
}
