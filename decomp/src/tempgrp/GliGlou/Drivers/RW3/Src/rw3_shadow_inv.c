#include "common.h"

#include "ps2/libc.h"

#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "PO/PO.h"

#include "ps2/copy_types.h"
#include "ps2/r5900.h"
#include "renderware.h"
#include "rwps2.h"
#include "rwps2_macros.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

RwFrame* RwFrameCreate(void);
RwCamera* RwCameraCreate(void);
RwTexture* RwTextureCreate(void* raster);
s32 RpSkyTextureCacheSetState(s32 state);
RwCamera* RwCameraSetFrame(void* camera, void* frame);
RwCamera* RwCameraSetRaster(void* camera, void* raster);
RwCamera* RwCameraSetZRaster(void* camera, void* raster);
void RwCameraSetFarClipPlane(void* camera, f32 farClip);
void RwCameraSetNearClipPlane(void* camera, f32 nearClip);
void RwCameraSetProjection(void* camera, s32 projection);
void RpWorldAddCamera(void* world, void* camera);
extern void* gp_6F74;
extern s32 inv_SHW_cameraCount128;
extern RwCamera** inv_SHW_cameras128;
extern RwTexture** inv_SHW_rasters128;
extern s32 inv_SHW_cameraCount64;
extern RwCamera** inv_SHW_cameras64;
extern RwTexture** inv_SHW_rasters64;
extern s8 D_00393748;
extern void* inv_SHW_queuedFrames[];
extern RwRaster* RwCameraGetZRaster(void*);
extern RwRGBA inv_SHW_materialColor;
extern s32 inv_SHW_queuedAtomicCount;
extern RpAtomic* inv_SHW_queuedAtomics[];
void* RwFrameGetParent(void* frame);
RwMatrix* RwFrameGetMatrix(RwFrame* frame);
RwMatrix* RwMatrixUpdate(RwMatrix* matrix);
RwMatrix* RwMatrixScale(RwMatrix* matrix, f32* scale, s32 combine);
void RwFrameUpdateObjects(RwFrame* frame);
extern s32 inv_SHW_cameraIndex64;
extern s32 inv_SHW_cameraIndex128;
    void* memset(void* s, s32 c, s32 n);
void skyTexCacheRasterLock(void* raster, s32 lock);
void sweOpenLocalPkt(s32 a0, s32 a1);
s32 sweFinaliseOpenLocalPkt(s32 a0, s32 a1);
void _sweAddPkt(s32* counter, s32 type);
f32 RwCameraGetNearClipPlane(void* a0);
extern RwSky2DVertexFields flt_370250[4];
void RwIm2DRenderIndexedPrimitive(s32 a0, void* a1, s32 a2, void* a3, s32 t0);
extern PackedU64x2Aligned4 inv_RW3_stWhiteVertexColor;
extern u8 inv_GLI_rectangleIndices[];
extern u8 inv_GLI_ucTextGradientRed[];
extern s8 inv_GLI_cPrepared2DRectangleMode;
extern f32 inv_GLI_preparedRectangleVertices[];

void func_00153970(u8* modeOut)
{
    GliTexture* texture;

    texture = inv_GLI_renderContext->currentMaterial->layers[0].texture;
    if (texture != 0 && texture->loaded != 0) {
        if ((texture->flags & 0x20) != 0) {
            modeOut[0] = 2;
            return;
        }
        modeOut[0] = 3;
        return;
    }
    modeOut[0] = 1;
}

void func_001539C0(u8* modeOut, GLI_tdstRenderContext* context)
{
    s32 hasTexture;
    GliTexture* texture;

    hasTexture = 0;
    texture = inv_GLI_renderContext->currentMaterial->layers[0].texture;
    if (texture != NULL) {
        hasTexture = texture->loaded != 0;
    }
    inv_GLI_renderContext->unk20AC = 0;
    inv_GLI_renderContext->inv_lDrawFlags &= ~GLI_DRAW_DEFERRED_inv;
    if (!inv_GLI_zWriteState) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
    }
    if (inv_GLI_renderContext->fogEnabled) {
        if (inv_GLI_renderContext->appliedFogParameters != inv_GLI_renderContext->fogParameters) {
            inv_GLI_RW_vApplyFogParameters(inv_GLI_renderContext->fogParameters);
            inv_GLI_renderContext->appliedFogParameters = inv_GLI_renderContext->fogParameters;
        }
        if (!inv_GLI_fogState) {
            RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 1);
            inv_GLI_fogState = 1;
        }
    } else if (inv_GLI_fogState) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
        inv_GLI_fogState = 0;
    }
    if (inv_GLI_renderContext->lCurrentDrawMask & 0x40000000) {
        if (!inv_GLI_zTestState) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
        }
    } else if (inv_GLI_zTestState) {
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
        inv_GLI_zTestState = 0;
    }
    if (inv_GLI_renderContext->lCurrentDrawMask & 0x08000000) {
        if (!inv_GLI_zWriteState) {
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = 1;
        }
    } else if (inv_GLI_zWriteState) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
        inv_GLI_zWriteState = 0;
    }
    if (hasTexture) {
        if (inv_GLI_renderContext->currentMaterial->layers[0].texture->flags & 0x100) {
            *modeOut = 5;
            return;
        }
        if (inv_GLI_renderContext->unk20B0 & 2) {
            *modeOut = 3;
            return;
        }
    }
    *modeOut = 4;
}

void* RW3_SHW_fn_pstCreateCameraTexture(s32 width, s32 height, void** rasterSlot)
{
    RwCamera* camera;
    RwFrame* frame;
    RwRaster* colorRaster;
    RwRaster* zRaster;
    RwTexture* texture;

    camera = RwCameraCreate();
    if (camera != NULL) {
        frame = RwFrameCreate();
        if (frame != NULL) {
            colorRaster = RwRasterCreate(width, height, 0x20, 5);
            if (colorRaster != NULL) {
                RpSkyTextureCacheSetState(1);
                zRaster = RwRasterCreate(width, height, 0, 1);
                if (zRaster != NULL) {
                    texture = RwTextureCreate(colorRaster);
                    *rasterSlot = texture;
                    if (texture != NULL) {
                        RwCameraSetFrame(camera, frame);
                        RwCameraSetRaster(camera, colorRaster);
                        RwCameraSetZRaster(camera, zRaster);
                        RwCameraSetFarClipPlane(camera, 500.0f);
                        RwCameraSetNearClipPlane(camera, 0.25f);
                        RwCameraSetProjection(camera, 2);
                        RpWorldAddCamera(RW3_g_pWorld, camera);
                    }
                }
            }
        }
    }
    return camera;
}

s32 RW3_SHW_fn_ulCreateCamerasTextures(s32 width, s32 height, RwCamera*** cameraPtr,
                                       RwTexture*** rasterPtr, u32 count)
{
    RwCamera** cameras;
    RwTexture** rasters;
    RwCamera* created;
    s32 success;
    u32 i;
    s32 bytes;
    bytes = count << 2;
    *cameraPtr = AI_fn_p_vTrueAlloc(gp_6F74, bytes);
    cameras = *cameraPtr;
    *rasterPtr = AI_fn_p_vTrueAlloc(gp_6F74, bytes);
    rasters = *rasterPtr;
    success = 0;
    i = 0;
    for (; i < count; i++) {
        created = RW3_SHW_fn_pstCreateCameraTexture(width, height, (void**) rasters++);
        *cameras++ = created;
        if (created)
            success++;
    }
    return success;
}

s32 RW3_SHW_fn_bInit(s32 a0, s32 a1)
{
    s32 count0;
    s32 count1;
    s32 temp;
    s32 frameIndex;
    s32 frameAddress;
    void** frames;

    count0 = a0;
    count1 = a1;
    inv_SHW_cameraCount128 = RW3_SHW_fn_ulCreateCamerasTextures(0x80, 0x80, &inv_SHW_cameras128, &inv_SHW_rasters128, count0);
    inv_SHW_cameraCount64 = RW3_SHW_fn_ulCreateCamerasTextures(0x40, 0x40, &inv_SHW_cameras64, &inv_SHW_rasters64, count1);
    if (D_00393748 == 0) {
        frames = inv_SHW_queuedFrames;
        frameIndex = 0;
        frameAddress = (s32) frames;
        do {
            *(void**) frameAddress = RwFrameCreate();
            frameIndex++;
            frameAddress += 4;
        } while ((u32) frameIndex < 0x64U);
        D_00393748 = 1;
    }
    a1 = 0;
    if (inv_SHW_cameraCount128 == count0) {
        temp = inv_SHW_cameraCount64 ^ count1;
        a1 = (u32) temp < 1U;
    }
    return a1;
}

void inv_SHW_fn_vDestroyShadowCamera(void* camera, void* texture)
{
    RwRaster* raster;
    RwFrame* frame;

    RwTextureDestroy(texture);
    raster = RwCameraGetZRaster(camera);
    RwRasterDestroy(raster);
    frame = RwCameraGetFrame(camera);
    RwFrameDestroy(frame);
    RwCameraDestroy(camera);
}

void inv_SHW_fn_vDestroyShadowCameras(void** cameraPtr, void** rasterPtr, s32 count)
{
    void** cameras;
    void** rasters;
    s32 i;

    cameras = (void**) *cameraPtr;
    rasters = (void**) *rasterPtr;
    i = 0;
    if (count != 0) {
        do {
            i++;
            inv_SHW_fn_vDestroyShadowCamera(*cameras, *rasters);
            *cameras = 0;
            *rasters = 0;
            cameras++;
            rasters++;
        } while ((u32) i < (u32) count);
    }
    inv_MEM_fn_vFreeInHeap(gp_6F74, *cameraPtr);
    *cameraPtr = 0;
    inv_MEM_fn_vFreeInHeap(gp_6F74, *rasterPtr);
    *rasterPtr = 0;
}

void inv_SHW_fn_vDesInitShadow(void)
{
    inv_SHW_fn_vDestroyShadowCameras((void**) &inv_SHW_cameras128, (void**) &inv_SHW_rasters128,
                  inv_SHW_cameraCount128);
    inv_SHW_fn_vDestroyShadowCameras((void**) &inv_SHW_cameras64, (void**) &inv_SHW_rasters64, inv_SHW_cameraCount64);
}

void RW3_SHW_RenderObject(GeometricObjectView* object)
{
    RwRGBA* RpMaterialGetColor(RpMaterial * mat);
    s32 meshIndex;
    RpAtomic** meshCursor;
    RpAtomic** firstMesh;
    s16 signedMeshCount;
    s32 isShadowEnabled;
    GliTexture* texture;
    RpMaterial* material;
    RwRGBA savedMode3Color;
    RwRGBA savedMode5Color;
    RpAtomic* atomicToRender;
    void* parentFrame;
    RpAtomicCallBackRender renderCallback;
    void** matrixSlot;
    RpAtomic* queuedAtomic;

    meshIndex = 0;
    signedMeshCount = object->elementCount;
    firstMesh = object->rw3Object->atomics;
    if (signedMeshCount == 0) {
        return;
    }
    meshCursor = firstMesh;

    do {
        if (*(u8*) &object->elementTypes[meshIndex] != 1) {
            continue;
        }

        texture = object->elements[meshIndex]->material->layers[0].texture;
        isShadowEnabled = texture != 0 && texture->loaded != 0;
        if (isShadowEnabled != 0 && (texture->flags & 0x128) != 0) {
            if ((texture->flags & 8) != 0) {
                material = RpGeometryGetMaterial((*meshCursor)->geometry, 0);
                savedMode3Color = *RpMaterialGetColor(material);
                if (inv_GLI_blendModeState != 3) {
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                    inv_GLI_blendModeState = 3;
                }
                RpMaterialSetColor(material, &inv_SHW_materialColor);
                atomicToRender = *meshCursor;
                renderCallback = atomicToRender->renderCallBack;
                renderCallback(atomicToRender);
                RpMaterialSetColor(material, &savedMode3Color);
            } else if ((texture->flags & 0x100) != 0) {
                material = RpGeometryGetMaterial((*meshCursor)->geometry, 0);
                savedMode5Color = *RpMaterialGetColor(material);
                if (inv_GLI_blendModeState != 5) {
                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 1);
                    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDSRCALPHA);
                    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDINVSRCALPHA);
                    inv_GLI_blendModeState = 5;
                }
                RpMaterialSetColor(material, &inv_SHW_materialColor);
                atomicToRender = *meshCursor;
                renderCallback = atomicToRender->renderCallBack;
                renderCallback(atomicToRender);
                RpMaterialSetColor(material, &savedMode5Color);
            }
        } else {
            queuedAtomic = *meshCursor;
            if (inv_SHW_queuedAtomicCount != 0x64) {
                inv_SHW_queuedAtomics[inv_SHW_queuedAtomicCount] = queuedAtomic;
                matrixSlot = ((void*) inv_SHW_queuedFrames + (inv_SHW_queuedAtomicCount << 2));
                parentFrame = RwFrameGetParent(queuedAtomic->object.object.parent);
                RW3_Utils_DuplicateFrame(*matrixSlot, parentFrame);
                inv_SHW_queuedAtomicCount++;
            }
        }
        meshCursor++;
    } while ((u32) ++meshIndex < (u32) object->elementCount);
}

void RW3_SHW_fn_vRenderObjectRecursive(HIE_tdstSuperObject_* superObject)
{
    GeometricObjectView* object;
    RW3ObjectView* rw3Object;

    if (((superObject->flags >> 1) & 1) == 0) {
        if (superObject->lTypeOfLinkedObject == HIE_TYPE_PHYSICAL_OBJECT_inv) {
            object = ((PO_tdstPhysicalObject_*) superObject->hLinkedObject)->_hVisualSet;
            rw3Object = object->rw3Object;
            if (rw3Object != 0) {
                RW3_UpdateFrameWithGeometricObjectMatrix(rw3Object->frame, superObject->gu_hGlobalMatrix);
                RW3_SHW_RenderObject(object);
            }
        }
    }
    superObject = superObject->hFirstElementSta;

    if (superObject != 0) {
        do {
            RW3_SHW_fn_vRenderObjectRecursive(superObject);
            superObject = superObject->hNextBrotherSta;
        } while (superObject != 0);
    }
}

void RW3_SHW_UpdateCameraMatrix(RwCamera* camera, POS_tdstMatrix_* obj, s32 height, s32 width)
{
    struct {
        POS_tdstMatrix_ local;
        f32 pad[3];
        f32 scale[3];
    } st;

    RwFrame* frame;
    RwMatrix* mat;
    RwV3d* up;
    RwV3d* at;
    RwV3d* pos;
    f32* q;
    f32 ten;

    inv_POS_fn_vRemoveScaleFromMatrix(&st.local, obj);
    frame = RwCameraGetFrame(camera);
    if (frame == 0) {
        return;
    }

    mat = RwFrameGetMatrix(frame);
    ten = 10.0f;
    q = &inv_RW3_stWorldOrigin.x;
    up = &mat->up;
    at = &mat->at;
    pos = &mat->pos;

    mat->right.x = st.local.stMatrix.stCol_0.xX;
    mat->right.y = st.local.stMatrix.stCol_0.xY;
    mat->right.z = st.local.stMatrix.stCol_0.xZ;
    mat->up.x = st.local.stMatrix.stCol_1.xX;
    up->y = st.local.stMatrix.stCol_1.xY;
    up->z = st.local.stMatrix.stCol_1.xZ;
    mat->at.x = st.local.stMatrix.stCol_2.xX;
    at->y = st.local.stMatrix.stCol_2.xY;
    at->z = st.local.stMatrix.stCol_2.xZ;

    pos->x = (st.local.stMatrix.stCol_3.xX + q[0]) - (mat->at.x * ten);
    pos->y = (st.local.stMatrix.stCol_3.xY + q[1]) - (at->y * ten);
    pos->z = (st.local.stMatrix.stCol_3.xZ + q[2]) - (at->z * ten);
    RwMatrixUpdate(mat);

    {
        struct B {
            f32 f;
        } bx, by;

        bx.f = 2.0f / obj->stScale.xX - 4.0f / (f32) width;
        by.f = 2.0f / obj->stScale.xY - 4.0f / (f32) height;
        st.scale[2] = 1.0f;
        st.scale[0] = -bx.f;
        st.scale[1] = -by.f;
    }
    RwMatrixScale(mat, st.scale, 1);
    RwFrameUpdateObjects(frame);
}

void RW3_SHW_fn_vRenderCharacterInShadowView(HIE_tdstSuperObject_* superObject, POS_tdstMatrix_* matrix,
                                             GliMaterialData* material)
{
    UnalignedS32* RpMaterialGetColor(void* mat);

    u32 slot[8];
    GliTexture* texInfo;
    s32 savedState;
    GliTexture* outPtr;
    RwTexture* savedTexture;
    s32 camIndex;
    RwCamera** camList;
    RwTexture** rasList;
    RwCamera* camera;
    s32 mode4;
    s32 count;
    RpAtomic** atomics;
    void** frames;
    RpMaterial* atomicMaterial;
    void* texture;
    void* frame;
    RpAtomicCallBackRender renderCallback;

    texInfo = 0;
    GLI_xGetMaterialTexture(material, &texInfo);

    if (texInfo->lWidth == 0x80) {
        camIndex = inv_SHW_cameraIndex128;
        camList = inv_SHW_cameras128;
        rasList = inv_SHW_rasters128;
    } else {
        camIndex = inv_SHW_cameraIndex64;
        camList = inv_SHW_cameras64;
        rasList = inv_SHW_rasters64;
    }

    memset(slot, 0, 4);
    camera = camList[camIndex];
    savedTexture = rasList[camIndex];
    RwCameraGetRaster(camera);
    RwCameraClear(camera, (RwRGBA*) slot, 3);
    RW3_SHW_UpdateCameraMatrix(camera, matrix, texInfo->lHeight, texInfo->lWidth);

    if (RwCameraBeginUpdate(camera) != 0) {
        RwRenderStateGet(rwRENDERSTATEFOGENABLE, &savedState);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) 0);
        if (inv_GLI_zWriteState != 0) {
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
            inv_GLI_zWriteState = 0;
        }
        if (inv_GLI_zTestState != 0) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 0);
            inv_GLI_zTestState = 0;
        }
        inv_SHW_queuedAtomicCount = 0;
        RW3_SHW_fn_vRenderObjectRecursive(superObject);

        mode4 = 4;
        atomics = inv_SHW_queuedAtomics;
        frames = inv_SHW_queuedFrames;
        if (inv_GLI_blendModeState != (u8) mode4) {
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*) 0);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
            inv_GLI_blendModeState = (u8) mode4;
        }

        count = inv_SHW_queuedAtomicCount;
        if (count != 0) {
            do {
                RpAtomic* atomic;

                count = count - 1;
                atomic = *atomics;
                frame = atomic->object.object.parent;
                atomicMaterial = RpGeometryGetMaterial(atomic->geometry, 0);
                texture = RpMaterialGetTexture(atomicMaterial);
                *(UnalignedS32*) &slot[4] = *RpMaterialGetColor(atomicMaterial);
                RpMaterialSetColor(atomicMaterial, &inv_SHW_materialColor);
                RpMaterialSetTexture(atomicMaterial, (void*) 0);
                RpAtomicSetFrame(*atomics, *frames);
                frames = frames + 1;
                renderCallback = (*atomics)->renderCallBack;
                renderCallback(*atomics);
                RpMaterialSetColor(atomicMaterial, (RwRGBA*) &slot[4u - count + count]);
                RpMaterialSetTexture(atomicMaterial, texture);
                RpAtomicSetFrame(*atomics, frame);
                atomics = atomics + 1;
            } while (count != 0);
        }

        if (inv_GLI_zWriteState == 0) {
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = 1;
        }
        if (inv_GLI_zTestState == 0) {
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = 1;
        }
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*) savedState);
        RwCameraEndUpdate(camera);
    }

    GLI_xGetMaterialTexture(material, &outPtr);
    outPtr->rwTexture = savedTexture;

    if (texInfo->lWidth == 0x80) {
        inv_SHW_cameraIndex128 = inv_SHW_cameraIndex128 + 1;
    } else {
        inv_SHW_cameraIndex64 = inv_SHW_cameraIndex64 + 1;
    }
}

void RW3_SHW_fn_vBeginRenderCharacterInShadowView(void)
{
    s32 i;
    RwRaster* raster;

    i = 0;
    if ((inv_SHW_cameraIndex128 = 0, inv_SHW_cameraCount128) != 0) {
        do {
            raster = RwCameraGetRaster(inv_SHW_cameras128[i]);
            skyTexCacheRasterLock(raster, 1);
            i++;
        } while ((u32) i < (u32) inv_SHW_cameraCount128);
    }

    i = 0;
    if ((inv_SHW_cameraIndex64 = 0, inv_SHW_cameraCount64) != 0) {
        do {
            raster = RwCameraGetRaster(inv_SHW_cameras64[i]);
            skyTexCacheRasterLock(raster, 1);
            i++;
        } while ((u32) i < (u32) inv_SHW_cameraCount64);
    }
}

void RW3_SHW_fn_vEndRenderCharacterInShadowView(void)
{
    s32 i;
    RwRaster* raster;

    i = 0;
    if (inv_SHW_cameraCount128 != 0) {
        do {
            raster = RwCameraGetRaster(inv_SHW_cameras128[i]);
            skyTexCacheRasterLock(raster, 0);
            i++;
        } while ((u32) i < (u32) inv_SHW_cameraCount128);
    }

    i = 0;
    if (inv_SHW_cameraCount64 != 0) {
        do {
            raster = RwCameraGetRaster(inv_SHW_cameras64[i]);
            skyTexCacheRasterLock(raster, 0);
            i++;
        } while ((u32) i < (u32) inv_SHW_cameraCount64);
    }
}

void inv_RW3_fn_vBuildRasterUploadPacket(void* a0, void* a1, void* a2, s32 a3, s32 t0, void* t1)
{
    Qword* pkt;
    u128 q0, q1, q2, q3, q4, q5;
    u128 q6, q7;
    u64 reg;
    u32 len;

    u32 c1 = (u32) 0x3F;
    reg = 0x1000000000008005ULL;
    RWPS2_MAKE_QWORD(q0, (u64) 0xE, reg);
    RWPS2_MAKE_QWORD(q1, c1, (u64) 0);
    RWPS2_MAKE_QWORD(q2, (u64) 0x51, (u64) 0);
    RWPS2_MAKE_QWORD(q5, (u64) 0x53, (u64) 0);

    sweOpenLocalPkt(1, 0);
    pkt = inv_GLI_packetCursor;
    if (!pkt) {
        printf("%s(%d): DMA memory failure\n", "rw3_texupload.c", 0x68);
    } else {
        Qword* next = pkt + 1;
        reg = (((u64) (u32) t0 << 56) |
               (0xA0000 |
                (((u64) (u32) t0 << 24) | (((u64) ((u32) a2 & 0x3FFF) << 32) | ((u64) a3 << 48)))));
        pkt[0].q = q0;
        inv_GLI_packetCursor = next;
        pkt[1].q = q1;
        inv_GLI_packetCursor = pkt + 2;
        RWPS2_MAKE_QWORD(q3, (u64) 0x50, reg);
        pkt[2].q = q3;
        inv_GLI_packetCursor = pkt + 3;
        pkt[3].q = q2;
        inv_GLI_packetCursor = pkt + 4;
        reg = ((u64) a1 << 32) | (u32) a0;
        RWPS2_MAKE_QWORD(q4, (u64) 0x52, reg);
        pkt[4].q = q4;
        inv_GLI_packetCursor = pkt + 5;
        pkt[5].q = q5;
        inv_GLI_packetCursor = pkt + 6;
    }
    sweFinaliseOpenLocalPkt(0x88000501, -4);
    {
        Qword* pkt;

        pkt = inv_GLI_packetCursor;
        if (!pkt) {
            printf("%s(%d): DMA memory failure\n", "rw3_texupload.c", 0x97);
            return;
        }
        len = ((u32) t1 + 0x7F) & 0xFFFFFF80;
        reg = ((u64) len << 32) | 0x50000000;
        RWPS2_MAKE_QWORD(q6, (u64) 0, reg);
        pkt[0].q = q6;
        inv_GLI_packetCursor = pkt + 1;
        reg = (u64) 0xF0000000;
        RWPS2_MAKE_QWORD(q6, (u64) 0, reg);
        pkt[1].q = q6;
        inv_GLI_packetCursor = pkt + 2;
        sweFinaliseOpenLocalPkt(1, 0);
    }
}

void inv_RW3_fn_vUploadRaster(RwRaster* raster, void* a1)
{
    _SkyRasterExt* ext;
    void* t1;
    u32 word;
    void* width;
    void* height;

    ext = (_SkyRasterExt*) ((u8*) raster + skyRasterExt);
    t1 = ext->mipUploadPkts[0];
    if (t1 == 0) {
        return;
    }
    if (ext->cachePkts == 0) {
        return;
    }
    word = ext->tex0.words[0];
    height = (void*) raster->height;
    width = (void*) raster->width;
    inv_RW3_fn_vBuildRasterUploadPacket(width, height, a1, (s32) (word >> 14) & 0x3F, (s32) (word >> 20) & 0x3F, t1);
}

void func_00154B00(u8* object)
{
    _SkyRasterExt* ext = (_SkyRasterExt*) (object + skyRasterExt);
    s32* counter = &ext->dmaRefCount;
    s32 count = *counter;
#ifdef PC_PORT
    *counter = count + 1;
#else
    u32 status;

    do {
        R5900_DI();
        R5900_SYNC_P();
        R5900_MFC0(status, 12);
        if ((u8) status & 0x100)
            count = 0;
    } while (status & 0x10000);
    *counter = count + 1;
    R5900_SYNC();
    R5900_EI();
#endif
    _sweAddPkt(counter, 0x82);
}

void inv_RW3_fn_vInitScreenQuadVertices(void)
{
    extern f32 qword_370150[];

    typedef union {
        u64 dw[8];
        s32 sw[16];
        f32 ff[16];
    } ShadowScreenVertexBits;

    typedef struct {
        ShadowScreenVertexBits v[4];
    } ShadowScreenQuad;

    f32 near_z, one, col, rhw;
    ShadowScreenVertexBits tmp;
    u8* dst;

    near_z = RwIm2DGetNearScreenZ();
    rhw = RwCameraGetNearClipPlane(inv_GLI_primaryCamera);
    one = 1.0f;
    col = 255.0f;
    dst = (u8*) flt_370250;
    rhw = one / rhw;

    tmp.sw[0] = 0;
    tmp.sw[1] = 0;
    tmp.ff[2] = near_z;
    tmp.sw[3] = 0;
    tmp.ff[4] = one;
    tmp.ff[5] = one;
    tmp.ff[6] = rhw;
    tmp.sw[7] = 0;
    tmp.ff[8] = col;
    tmp.ff[9] = col;
    tmp.ff[10] = col;
    tmp.ff[11] = col;
    tmp.sw[12] = 0;
    tmp.sw[13] = 0;
    tmp.sw[14] = 0;
    tmp.sw[15] = 0;
    ((ShadowScreenVertexBits*) qword_370150)[0] = tmp;
    tmp.sw[4] = 0;
    tmp.ff[5] = one;
    ((ShadowScreenVertexBits*) qword_370150)[1] = tmp;
    tmp.sw[4] = 0;
    tmp.sw[5] = 0;
    ((ShadowScreenVertexBits*) qword_370150)[2] = tmp;
    tmp.ff[4] = one;
    tmp.sw[5] = 0;
    ((ShadowScreenVertexBits*) qword_370150)[3] = tmp;
    *(ShadowScreenQuad*) dst = *(ShadowScreenQuad*) qword_370150;
}

void func_00154CF8(GliMaterialData* material, f32 x0, f32 y0, f32 x1, f32 y1)
{
    typedef union VertexData {
        u64 dw[8];
        s32 sw[16];
        f32 ff[16];
    } VertexData;

    extern f32 qword_370150[];

    u32 color;
    s32 bits;
    f32 tmp;
    s32 ix0;
    s32 iy0;
    s32 ix1;
    s32 iy1;
    f32* base;
    VertexData* vertices;
    u8* cp;
    s32 red_byte;
    s32 blue_byte;
    s32 green_byte;
    s32 alpha_byte;
    f32 red;
    f32 blue;
    f32 green;
    f32 alpha;
    f32 fx0;
    f32 fy0;
    f32 fx1;
    f32 fy1;
    u32 mask;
    s32 disabled_state;
    GLI_tdstRenderContext* renderer;
    GLI_tdstRenderContext* updated_renderer;

    color = 0xFF000000;

    tmp = material->color.xR;
    tmp = tmp + 49152.0f;
    bits = *(s32*) &tmp;
    if ((bits & 0x400000) == 0) {
        bits = 0;
    } else if ((bits & 0xFF00) != 0) {
        bits = 0xFF0000;
    } else {
        bits = (bits & 0xFF) << 16;
    }
    color |= bits;

    tmp = material->color.xG;
    tmp = tmp + 49152.0f;
    bits = *(s32*) &tmp;
    if ((bits & 0x400000) == 0) {
        bits = 0;
    } else if ((bits & 0xFF00) != 0) {
        bits = 0xFF00;
    } else {
        bits = (bits & 0xFF) << 8;
    }
    color |= bits;

    tmp = material->color.xB;
    tmp = tmp + 49152.0f;
    bits = *(s32*) &tmp;
    if ((bits & 0x400000) == 0) {
        bits = 0;
    } else if ((bits & 0xFF00) != 0) {
        bits = 0xFF;
    } else {
        bits &= 0xFF;
    }
    color |= bits;

    ix0 = (s32) x0;
    iy0 = (s32) y0;
    ix1 = (s32) x1;
    iy1 = (s32) y1;

    renderer = inv_GLI_renderContext;
    base = qword_370150;
    vertices = (VertexData*) base;
    cp = (u8*) &color;

    red_byte = cp[2];
    fx0 = (f32) ix0;
    green_byte = cp[1];
    blue_byte = cp[0];
    red = (f32) red_byte;
    alpha_byte = cp[3];
    blue = (f32) blue_byte;
    green = (f32) green_byte;
    mask = 0xFFFFFBFF;
    alpha = (f32) alpha_byte;
    disabled_state = -0x401;

    vertices[0].ff[8] = red;
    vertices[0].ff[10] = blue;
    fy0 = (f32) iy0;
    fx1 = (f32) ix1;
    vertices[0].ff[9] = green;
    fy1 = (f32) iy1;
    vertices[0].ff[11] = alpha;

    ((RwSky2DVertexFields*) base)[1].color = ((RwSky2DVertexFields*) base)[0].color;
    ((RwSky2DVertexFields*) base)[2].color = ((RwSky2DVertexFields*) base)[0].color;
    ((RwSky2DVertexFields*) base)[3].color = ((RwSky2DVertexFields*) base)[0].color;

    qword_370150[0] = fx0;
    base[17] = fy0;
    base[32] = fx1;
    base[1] = fy0;
    base[49] = fy1;
    base[16] = fx1;
    base[33] = fy1;
    base[48] = fx0;

    renderer->currentMaterial = material;
    material->type = mask;
    renderer->lHierachDrawMask = disabled_state;
    renderer->lCurrentDrawMask = disabled_state;
    GLI_vDoMaterialSelection();
    updated_renderer = inv_GLI_renderContext;

    updated_renderer->unk20AC = 1;
    if ((updated_renderer->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) != 0) {
        GLI_RW_vDoTextureSelection();
    }

    RwIm2DRenderIndexedPrimitive(3, base, 4, inv_GLI_rectangleIndices, 6);
}

void func_00154F68(s32 a0, u32* a1, s32 a2, s32 a3, f32 x12, f32 x13, f32 x14, f32 x15)
{
    extern RwSky2DVertexFields qword_370150[4];

    RwSky2DVertexFields* vertices;
    GLI_tdstRenderContext* ctx;
    GLI_tdstRenderContext* ctx2;
    f32 sx0;
    f32 sy0;
    f32 sx1;
    f32 sy1;

    sx0 = (f32) (s32) x12;
    sy0 = (f32) (s32) x14;
    sx1 = (f32) (s32) x13;
    sy1 = (f32) (s32) x15;

    vertices = qword_370150;
    *(PackedU64x2Aligned4*) &vertices[0].color = inv_RW3_stWhiteVertexColor;
    *(PackedU64x2Aligned4*) &vertices[1].color = inv_RW3_stWhiteVertexColor;
    *(PackedU64x2Aligned4*) &vertices[2].color = inv_RW3_stWhiteVertexColor;
    *(PackedU64x2Aligned4*) &vertices[3].color = inv_RW3_stWhiteVertexColor;

    vertices[0].scrVertex.x = sx0;
    vertices[1].scrVertex.y = sy0;
    vertices[2].scrVertex.x = sx1;
    vertices[3].scrVertex.y = sy1;
    vertices[0].scrVertex.y = sy0;
    vertices[1].scrVertex.x = sx1;
    vertices[2].scrVertex.y = sy1;
    vertices[3].scrVertex.x = sx0;

    ctx = inv_GLI_renderContext;
    ctx->currentMaterial = (GliMaterialData*) a1;
    *a1 = 0xFFFFFBFF;
    ctx->lHierachDrawMask = -1025;
    ctx->lCurrentDrawMask = -1025;
    GLI_vDoMaterialSelection();

    ctx2 = inv_GLI_renderContext;
    ctx2->unk20AC = 1;
    if ((ctx2->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) != 0) {
        GLI_RW_vDoTextureSelection();
    }

    RwIm2DRenderIndexedPrimitive(3, vertices, 4, inv_GLI_rectangleIndices, 6);
}

void func_001550E0(s32 arg0, u32* mode, f32 x0, f32 x1, f32 y0, f32 y1, f32 u1, f32 u0, f32 v1,
                   f32 v0)
{
    extern RwSky2DVertexFields qword_370150[4];

    RwSky2DVertexFields* vertices;
    GLI_tdstRenderContext* ctx;
    GLI_tdstRenderContext* ctx2;
    f32 one;
    f32 zero;
    f32 sx0;
    f32 sy0;
    f32 sx1;
    f32 sy1;

    sx0 = (f32) (s32) x0;
    sy0 = (f32) (s32) y0;
    sx1 = (f32) (s32) x1;
    sy1 = (f32) (s32) y1;
    vertices = qword_370150;
    *(PackedU64x2Aligned4*) &vertices[0].color = inv_RW3_stWhiteVertexColor;
    *(PackedU64x2Aligned4*) &vertices[1].color = inv_RW3_stWhiteVertexColor;
    *(PackedU64x2Aligned4*) &vertices[2].color = inv_RW3_stWhiteVertexColor;
    *(PackedU64x2Aligned4*) &vertices[3].color = inv_RW3_stWhiteVertexColor;
    vertices[0].scrVertex.x = sx0;
    vertices[1].scrVertex.y = sy0;
    vertices[2].scrVertex.x = sx1;
    vertices[3].scrVertex.y = sy1;
    vertices[3].u = u0;
    vertices[1].v = v0;
    vertices[2].u = u1;
    vertices[3].v = v1;
    vertices[0].scrVertex.y = sy0;
    vertices[1].scrVertex.x = sx1;
    vertices[2].scrVertex.y = sy1;
    vertices[0].u = u0;
    vertices[0].v = v0;
    vertices[1].u = u1;
    vertices[2].v = v1;
    vertices[3].scrVertex.x = sx0;
    ctx = inv_GLI_renderContext;
    ctx->currentMaterial = (GliMaterialData*) mode;
    *mode = 0xFFFFFBFF;
    ctx->lHierachDrawMask = -1025;
    ctx->lCurrentDrawMask = -1025;
    GLI_vDoMaterialSelection();
    ctx2 = inv_GLI_renderContext;
    ctx2->unk20AC = 1;
    if ((ctx2->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) != 0) {
        GLI_RW_vDoTextureSelection();
    }
    RwIm2DRenderIndexedPrimitive(3, vertices, 4, inv_GLI_rectangleIndices, 6);
    one = 1.0f;
    zero = 0.0f;
    vertices[3].u = one;
    vertices[1].v = one;
    vertices[0].v = one;
    vertices[0].u = one;
    vertices[3].v = zero;
    vertices[2].v = zero;
    vertices[2].u = zero;
    vertices[1].u = zero;
}

void inv_RW3_fn_vDrawScreenSprite(void* dimsArg, void* materialArg, f32 x0, f32 y0, f32 x1, f32 y1)
{
    extern RwSky2DVertexFields qword_370150[];

    GLD_tdstViewportAttributes_* dims;
    GliMaterialData* material;
    f32 scaleX;
    f32 scaleY;
    f32 hundred;
    RwSky2DVertexFields* vertices;
    s16 oldFog;

    dims = dimsArg;
    material = materialArg;
    vertices = qword_370150;
    hundred = 100.0f;
    scaleX = (f32) (s32) dims->values.dwWidth / hundred;
    scaleY = (f32) (s32) dims->values.dwHeight / hundred;
    oldFog = *(s16*) &inv_GLI_renderContext->fogEnabled;

    GLI_vFogOff();

    vertices[3].scrVertex.x = x1 * scaleX;
    vertices[1].scrVertex.y = y0 * scaleY;
    vertices[2].scrVertex.x = x0 * scaleX;
    vertices[3].scrVertex.y = y1 * scaleY;
    vertices[0].scrVertex.x = vertices[3].scrVertex.x;
    vertices[1].scrVertex.x = vertices[2].scrVertex.x;
    vertices[0].scrVertex.y = vertices[1].scrVertex.y;
    vertices[2].scrVertex.y = vertices[3].scrVertex.y;

    {
        volatile GLI_tdstRenderContext* ctx = inv_GLI_renderContext;
        ctx->currentMaterial = material;
        material->type = 0xFFFFFBF7u;
        ctx->lCurrentDrawMask = -0x409;
        ctx->lHierachDrawMask = -0x409;
    }

    if (inv_GLI_blendModeState != 0) {
        inv_GLI_blendModeState = 0;
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        inv_GLI_zTestState = 1;
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
        inv_GLI_blendModeState = 0;
    }

    GLI_vDoMaterialSelection();
    {
        GLI_tdstRenderContext* ctx = inv_GLI_renderContext;
        ctx->unk20AC = 1;
        if ((ctx->inv_lDrawFlags & GLI_DRAW_DEFERRED_inv) != 0) {
            GLI_RW_vDoTextureSelection();
        }
    }

    if (inv_GLI_cTextGradientMode != 0) {
        GLI_tdstRenderContext* colorCtx = inv_GLI_renderContext;
        *(volatile f32*) &vertices[0].color.blue = 255.0f;
        *(volatile f32*) &vertices[0].color.red = 255.0f;
        *(volatile f32*) &vertices[0].color.green = 255.0f;
        vertices[0].color.alpha = (f32) colorCtx->vertexColor.channels.alpha;
        vertices[1].color = vertices[0].color;
        do {
            RwSky2DVertexFields* v2 = &vertices[2];
            u8* colors = inv_GLI_ucTextGradientRed;
            v2->color.red = (f32) colors[0];
            v2->color.green = (f32) colors[1];
            v2->color.blue = (f32) colors[2];
            v2->color.alpha = (f32) colorCtx->vertexColor.channels.alpha;
            vertices[3].color = v2->color;
        } while (0);
    } else {
        GLI_tdstRenderContext* colorCtx = inv_GLI_renderContext;
        vertices[0].color.red = (f32) colorCtx->vertexColor.channels.red;
        vertices[0].color.green = (f32) colorCtx->vertexColor.channels.green;
        vertices[0].color.blue = (f32) colorCtx->vertexColor.channels.blue;
        vertices[0].color.alpha = (f32) colorCtx->vertexColor.channels.alpha;
        vertices[1].color = vertices[0].color;
        vertices[2].color = vertices[0].color;
        vertices[3].color = vertices[0].color;
    }

    RwIm2DRenderIndexedPrimitive(3, qword_370150, 4, inv_GLI_rectangleIndices, 6);

    if (oldFog != 0) {
        GLI_vFogOn();
    }
}

void inv_GLI_fn_vDraw2DTexturedRectWithColor(GLD_tdstViewportAttributes_* viewport, u32 color,
                                             f32 x, f32 y, f32 z, f32 w)
{
    typedef struct {
        u64 a;
        u64 b;
    } __attribute__((packed, aligned(4))) RectVertexColorBlock;

    typedef struct {
        f32 at00;
        f32 at04;
        f32 pad08[6];
        RectVertexColorBlock color;
        f32 pad30[4];
        f32 at40;
        f32 at44;
        f32 pad48[6];
        RectVertexColorBlock copy60;
        f32 pad70[4];
        f32 at80;
        f32 at84;
        f32 pad88[6];
        RectVertexColorBlock copyA0;
        f32 padB0[4];
        f32 atC0;
        f32 atC4;
        f32 padC8[6];
        RectVertexColorBlock copyE0;
    } TexturedRectVertexTable;

    extern TexturedRectVertexTable qword_370150;

    f32 scaleY;
    f32 scaleX;
    u32 packed;
    u8* bytes;

    scaleX = (f32) (s32) viewport->values.dwWidth;
    scaleY = (f32) (s32) viewport->values.dwHeight;
    z *= scaleX;
    y *= scaleY;
    w *= scaleY;
    x *= scaleX;
    packed = color;
    bytes = (u8*) &packed;
    qword_370150.atC0 = z;
    qword_370150.at44 = y;
    qword_370150.at80 = x;
    qword_370150.atC4 = w;
    *(f32*) (&qword_370150.color) = (f32) bytes[2];
    *(f32*) ((u8*) &qword_370150.color + 4) = (f32) bytes[1];
    *(f32*) ((u8*) &qword_370150.color + 8) = (f32) bytes[0];
    *(f32*) ((u8*) &qword_370150.color + 12) = (f32) bytes[3];
    qword_370150.at04 = y;
    qword_370150.at00 = z;
    qword_370150.at40 = x;
    qword_370150.at84 = w;
    qword_370150.copy60 = qword_370150.color;
    qword_370150.copyA0 = qword_370150.color;
    qword_370150.copyE0 = qword_370150.color;
    RwIm2DRenderIndexedPrimitive(3, &qword_370150, 4, inv_GLI_rectangleIndices, 6);
}

void GLI_vDisplayFrame(MTH3D_tdstVector_* topLeft, MTH3D_tdstVector_* bottomRight,
                                    GLD_tdstViewportAttributes_* viewport, f32 alpha, f32 luminance)
{
    f32 scaleX = (f32) (s32) viewport->values.dwWidth / 100.0f;
    f32 scaleY = (f32) (s32) viewport->values.dwHeight / 100.0f;
    u8 level = luminance;

    flt_370250[0].scrVertex.x = bottomRight->xX * scaleX;
    flt_370250[3].scrVertex.x = flt_370250[0].scrVertex.x;
    flt_370250[1].scrVertex.y = topLeft->xY * scaleY;
    flt_370250[0].scrVertex.y = flt_370250[1].scrVertex.y;
    flt_370250[2].scrVertex.x = topLeft->xX * scaleX;
    flt_370250[1].scrVertex.x = flt_370250[2].scrVertex.x;
    flt_370250[3].scrVertex.y = bottomRight->xY * scaleY;
    flt_370250[2].scrVertex.y = flt_370250[3].scrVertex.y;
    do {
        flt_370250[0].color.red = level;
        flt_370250[0].color.green = level;
        flt_370250[0].color.blue = level;
        flt_370250[0].color.alpha = (u8) alpha;
    } while (0);
    flt_370250[1].color = flt_370250[0].color;
    inv_GLI_cPrepared2DRectangleMode = 1;
    flt_370250[2].color = flt_370250[0].color;
    flt_370250[3].color = flt_370250[0].color;
}

void inv_GLI_fn_vRenderPrepared2DRectangle(void)
{
    s8 renderMode;

    renderMode = inv_GLI_cPrepared2DRectangleMode;
    if (renderMode == 1) {
        GLI_tdstRenderContext* ctx;
        s16 oldFog;
        GliMaterialData* texture;

        ctx = inv_GLI_renderContext;
        texture = gs_hDefaultMaterial;
        oldFog = *(s16*) &ctx->fogEnabled;
        ctx->currentMaterial = texture;
        texture->type = 0xFFFFFBF7u;
        *(volatile s32*) &ctx->lCurrentDrawMask = -0x409;
        *(volatile s32*) &ctx->lHierachDrawMask = -0x409;

        if (inv_GLI_blendModeState != 0) {
            inv_GLI_blendModeState = 0;
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
            inv_GLI_zTestState = renderMode;
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = renderMode;
            inv_GLI_blendModeState = 0;
        }

        GLI_vFogOff();
        if (inv_GLI_zWriteState != 0) {
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 0);
            inv_GLI_zWriteState = 0;
        }

        GLI_vDoMaterialSelection();
        inv_GLI_renderContext->unk20AC = renderMode;
        GLI_RW_vDoTextureSelection();
        RwIm2DRenderIndexedPrimitive(3, inv_GLI_preparedRectangleVertices, 4,
                                     inv_GLI_rectangleIndices, 6);

        if (oldFog != 0) {
            GLI_vFogOn();
        }

        if (inv_GLI_zWriteState != 0) {
            inv_GLI_cPrepared2DRectangleMode = 0;
        } else {
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
            inv_GLI_zWriteState = renderMode;
            inv_GLI_cPrepared2DRectangleMode = 0;
        }
    }
}
