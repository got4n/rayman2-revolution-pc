#include "common.h"

#include "COL/COL.h"
#include "GAM/GAM.h"
#include "GEO/GEO.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "PO/PO.h"

#define DRAW_MODE_OPAQUE 0x8U
#define DRAW_MASK_CHARACTER_DEFAULTS 0xFC65F7FB

#define CUSTOM_BIT_NOT_RENDERED 0x800
#define CUSTOM_BIT_ZFAR_TRANSPARENCY 0x200000

#define RUNTIME_FLAG_RENDERED 0x20

#define MATRIX_TYPE_IDENTITY 1

#define DRAW_MASK_VISIBLE(mask)                                                                    \
    (((mask) & 0x02800000) == 0x02800000 || ((mask) & 0x03000000) == 0x01000000)
#define SUPER_OBJECT_HIDDEN(superObject) (((superObject)->flags >> 1) & 1)
#define SUPER_OBJECT_EXCLU_LIGHT(superObject) (((u32) (superObject)->flags >> 16) & 0x2)

#define CULLING_ALL_PLANES 0x7F
#define CULLING_NO_BOUNDS 0x40
#define CULLING_ZFAR_PLANES 0x30

extern s8 GLI_cGlobalFogIsOn;

void func_00149400(void* _hVpt, HIE_tdstSuperObject_* _hSprObj, s32 _lDrawMask,
                   s32 _lCullingResult);

static inline void inv_HIE_fn_vAddProjectedExtent(f32* minimum, f32* maximum, f32 coefficient,
                                                  f32 low, f32 high)
{
    f32 lowProduct = low * coefficient;
    f32 highProduct = high * coefficient;

    if (lowProduct < highProduct) {
        *minimum = *minimum + lowProduct;
        *maximum = *maximum + highProduct;
    } else {
        *minimum = *minimum + highProduct;
        *maximum = *maximum + lowProduct;
    }
}

void func_001488E0(COL_tdstParallelBox_* bounds, MTH4D_tdstMatrix_* matrix,
                   COL_tdstParallelBox_* source)
{
    MTH3D_tdstVector_* maximum = &bounds->stMaxPoint;
    f32 zero = 0.0f;

    MTH3D_M_vSetXYZ_inv(&bounds->stMinPoint, zero, zero, zero);
    bounds->stMaxPoint.xX = zero;
    maximum->xY = zero;
    maximum->xZ = zero;

    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xX, &maximum->xX, matrix->stCol_0.xX,
                                   source->stMinPoint.xX, source->stMaxPoint.xX);
    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xX, &maximum->xX, matrix->stCol_1.xX,
                                   source->stMinPoint.xY, source->stMaxPoint.xY);
    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xX, &maximum->xX, matrix->stCol_2.xX,
                                   source->stMinPoint.xZ, source->stMaxPoint.xZ);
    bounds->stMinPoint.xX = bounds->stMinPoint.xX + matrix->stCol_3.xX;
    maximum->xX = maximum->xX + matrix->stCol_3.xX;

    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xY, &maximum->xY, matrix->stCol_0.xY,
                                   source->stMinPoint.xX, source->stMaxPoint.xX);
    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xY, &maximum->xY, matrix->stCol_1.xY,
                                   source->stMinPoint.xY, source->stMaxPoint.xY);
    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xY, &maximum->xY, matrix->stCol_2.xY,
                                   source->stMinPoint.xZ, source->stMaxPoint.xZ);
    bounds->stMinPoint.xY = bounds->stMinPoint.xY + matrix->stCol_3.xY;
    maximum->xY = maximum->xY + matrix->stCol_3.xY;

    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xZ, &maximum->xZ, matrix->stCol_0.xZ,
                                   source->stMinPoint.xX, source->stMaxPoint.xX);
    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xZ, &maximum->xZ, matrix->stCol_1.xZ,
                                   source->stMinPoint.xY, source->stMaxPoint.xY);
    inv_HIE_fn_vAddProjectedExtent(&bounds->stMinPoint.xZ, &maximum->xZ, matrix->stCol_2.xZ,
                                   source->stMinPoint.xZ, source->stMaxPoint.xZ);
    bounds->stMinPoint.xZ = bounds->stMinPoint.xZ + matrix->stCol_3.xZ;
    maximum->xZ = maximum->xZ + matrix->stCol_3.xZ;
}

static inline void inv_HIE_fn_vComputeModuleBounds(HIE_tdstSuperObject_* module,
                                                   COL_tdstParallelBox_* box,
                                                   POS_tdstMatrix_* matrix)
{
    PO_tdstPhysicalObject_* geometry = module->hLinkedObject;
    COL_tdstBoundingSphere_* sphere = geometry->_hVisualBoundingVolume;
    COL_tdstBoundingSphere_ world;

    POS_fn_vMulMatrixVertex(&world.stCenterPoint, matrix, &sphere->stCenterPoint);
    world.xRadius = sphere->xRadius * POS_fn_xGetMaxScale(matrix);
    COL_fn_vComputeBoundingBoxOfSphere(&box->stMinPoint, &box->stMaxPoint, &world.stCenterPoint,
                                       world.xRadius);
}

static inline void inv_HIE_fn_vMergeModuleBounds(HIE_tdstSuperObject_* module,
                                                 COL_tdstParallelBox_* total)
{
    COL_tdstParallelBox_ box;
    POS_tdstMatrix_* matrix = module->gu_hGlobalMatrix;

    if ((u8) module->flags & HIE_FLAG_BOX_BOUNDS_inv) {
        func_001488E0(&box, &matrix->stMatrix, module->hVisualBoundingVolume);
    } else {
        inv_HIE_fn_vComputeModuleBounds(module, &box, matrix);
    }
    {
        f32 minX = box.stMinPoint.xX;
        f32 totalMinX = total->stMinPoint.xX;
        f32 minY = box.stMinPoint.xY;
        f32 totalMinY = total->stMinPoint.xY;
        f32 minZ = box.stMinPoint.xZ;
        f32 totalMinZ = total->stMinPoint.xZ;
        f32 maxX = box.stMaxPoint.xX;
        f32 totalMaxX = total->stMaxPoint.xX;
        f32 maxY = box.stMaxPoint.xY;
        f32 totalMaxY = total->stMaxPoint.xY;
        f32 maxZ = box.stMaxPoint.xZ;
        f32 totalMaxZ = total->stMaxPoint.xZ;

        totalMinX = inv_MTH_fn_xMinimum(totalMinX, minX);
        totalMinY = inv_MTH_fn_xMinimum(totalMinY, minY);
        totalMinZ = inv_MTH_fn_xMinimum(totalMinZ, minZ);
        totalMaxX = inv_MTH_fn_xMaximum(totalMaxX, maxX);
        totalMaxY = inv_MTH_fn_xMaximum(totalMaxY, maxY);
        totalMaxZ = inv_MTH_fn_xMaximum(totalMaxZ, maxZ);
        MTH3D_M_vSetXYZ_inv(&total->stMinPoint, totalMinX, totalMinY, totalMinZ);
        MTH3D_M_vSetXYZ_inv(&total->stMaxPoint, totalMaxX, totalMaxY, totalMaxZ);
    }
}

s32 func_00148C10(HIE_tdstSuperObject_* superObject, COL_tdstParallelBox_* total)
{
    HIE_tdstSuperObject_* child;

    if (superObject->lTypeOfLinkedObject == HIE_TYPE_PHYSICAL_OBJECT_inv) {
        inv_HIE_fn_vMergeModuleBounds(superObject, total);
    }
    for (child = superObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        if (func_00148C10(child, total) == 0) {
            return 0;
        }
    }
    return 1;
}

void HIE_fn_vSendStaticWorldToViewport(GLD_tdstViewportAttributes_* _hVpt,
                                       HIE_tdstSuperObject_* _hSprObj, s32 _DrawMask,
                                       s32 _lCullingResult)
{
    HIE_tdstSuperObject_* child;
    s32 drawMask;

    drawMask = _DrawMask & _hSprObj->gu_lDrawModeMask;
    HIE_fn_vPushSuperObjectMatrix(_hSprObj);

    if (_lCullingResult != 0 && DRAW_MASK_VISIBLE(drawMask) && !SUPER_OBJECT_HIDDEN(_hSprObj)) {
        inv_GLI_vPushTransparentSuperObject(_hSprObj, drawMask);
    }

    for (child = _hSprObj->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        void* box = child->hVisualBoundingVolume;
        s32 childCulling;

        if (box != NULL) {
            MTH3D_tdstVector_* minimum = COL_fn_pGetMinPointOfParallelBox(box);
            MTH3D_tdstVector_* maximum = COL_fn_pGetMaxPointOfParallelBox(box);
            POS_tdstMatrix_* matrix = child->gu_hGlobalMatrix;
            MTH3D_tdstVector_ worldMinimum;
            MTH3D_tdstVector_ worldMaximum;

            if ((matrix->ucType ^ MATRIX_TYPE_IDENTITY) != 0) {
                MTH4D_tdstVector_* translation = &matrix->stMatrix.stCol_3;

                MTH3D_M_vAddXYZ_inv(&worldMinimum, translation, minimum);
                minimum = &worldMinimum;

                MTH3D_M_vAddXYZ_inv(&worldMaximum, translation, maximum);
                maximum = &worldMaximum;
            }
            childCulling = GEO_lCullingBox(_hVpt, minimum, maximum, _lCullingResult);
        } else {
            childCulling = CULLING_NO_BOUNDS;
        }

        if (childCulling != 0 || (child->flags & HIE_FLAG_ALWAYS_VISIT_CHILDREN_inv)) {
            HIE_fn_vSendStaticWorldToViewport(_hVpt, child, drawMask, childCulling);
        }
    }

    HIE_fn_vPopMatrix();
}

void HIE_fn_vSendCharacterModulesToViewPort(void* _hVpt, HIE_tdstSuperObject_* _hSprObj,
                                            s32 _lDrawMask)
{
    HIE_tdstSuperObject_* child;

    HIE_fn_vPushSuperObjectMatrix(_hSprObj);
    _lDrawMask &= _hSprObj->gu_lDrawModeMask;

    if (DRAW_MASK_VISIBLE(_lDrawMask) && !SUPER_OBJECT_HIDDEN(_hSprObj)) {
        inv_GLI_vPushTransparentSuperObject(_hSprObj, _lDrawMask);
    }

    for (child = _hSprObj->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        if (child->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
            HIE_fn_vSendCharacterModulesToViewPort(_hVpt, child, _lDrawMask);
        }
    }

    HIE_fn_vPopMatrix();
}

void HIE_fn_vSendCharacterToViewport(void* _hVpt, HIE_tdstSuperObject_* _hSprObj, s32 _lDrawMask,
                                     s32 _lCullingResult)
{
    GLI_tdstLight_* light;
    s32 fatherPushed;
    s32 savedDrawMode;
    COL_tdstBoundingSphere_* bounds;
    void* sphere;
    MTH3D_tdstVector_ worldCenter;
    f32 savedZFar;
    f32 savedZoneWidth;
    f32 radius;
    tdstEngineObject_* object;
    s32 drawable;
    gu_tdstStandardGame* standardGame;
    HIE_tdstSuperObject_* child;
    HIE_tdstSuperObject_* father;
    s32 flags;
    u32 customBits;
    s32 render;
    s32 lightIndex;
    void* msLight;

    flags = _hSprObj->flags;
    savedZoneWidth = 0.0f;
    light = NULL;
    savedZFar = 0.0f;
    savedDrawMode = 0;
    bounds = NULL;
    sphere = NULL;
    radius = 0.0f;

    if (flags & HIE_FLAG_SUPERIMPOSED_inv) {
        return;
    }

    object = _hSprObj->hLinkedObject;
    standardGame = object->h_StandardGame;
    drawable = (standardGame->ucMiscFlags >> 2) & 1;
    if (drawable != 1) {
        return;
    }

    customBits = standardGame->ulCustomBits;
    if (flags & HIE_FLAG_HIDDEN_inv) {
        standardGame->ulCustomBits = customBits | CUSTOM_BIT_NOT_RENDERED;
        standardGame->ucMiscFlags &= ~RUNTIME_FLAG_RENDERED;
        return;
    }

    father = _hSprObj->hFatherSta;
    fatherPushed = 0;
    if (father != NULL && father->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv &&
        inv_fn_ucSuperObjectGetPlatFormType(father) != 0) {
        fatherPushed = 1;
        HIE_fn_vPushSuperObjectMatrix(father);
    }

    HIE_fn_vPushSuperObjectMatrix(_hSprObj);

    bounds = _hSprObj->hVisualBoundingVolume;
    if (bounds != NULL) {
        sphere = bounds;
        radius = bounds->xRadius;
    }

    if (customBits & CUSTOM_BIT_ZFAR_TRANSPARENCY) {
        f32 zoneMin;
        f32 zoneMax;
        f32 zFar;

        savedZFar = GEO_g_xZFar;
        zoneMin = (f32) standardGame->ucTransparencyZoneMin;
        zoneMax = (f32) standardGame->ucTransparencyZoneMax;
        savedZoneWidth = GEO_g_xZFarTransparencyZone;
        if (savedZFar == 0.0f) {
            GEO_g_xZFar = zoneMax;
            GEO_vSetZFarTransparencyZone(zoneMax - zoneMin);
        } else if (zoneMin < savedZFar) {
            zFar = inv_MTH_fn_xMinimum(savedZFar, zoneMax);
            GEO_g_xZFar = zFar;
            GEO_vSetZFarTransparencyZone(zFar - zoneMin);
        }

        savedDrawMode = _hSprObj->gu_lDrawModeMask;
        _hSprObj->flags = flags | HIE_FLAG_TRANSPARENT_inv;
        _hSprObj->gu_lDrawModeMask = savedDrawMode & ~DRAW_MODE_OPAQUE;
        _lCullingResult &= ~CULLING_ZFAR_PLANES;
    }

    if (_lCullingResult != CULLING_ALL_PLANES && bounds != NULL) {
        _lCullingResult =
            GEO_lCullingSphere(_hVpt, sphere, radius, _hSprObj->gu_hGlobalMatrix, _lCullingResult);
    } else {
        _lCullingResult = CULLING_ALL_PLANES;
    }

    _lDrawMask |= DRAW_MASK_CHARACTER_DEFAULTS;
    _lDrawMask &= _hSprObj->gu_lDrawModeMask;
    if (DRAW_MASK_VISIBLE(_lDrawMask) && _lCullingResult != 0) {
        render = 1;
        customBits &= ~CUSTOM_BIT_NOT_RENDERED;
        standardGame->ucMiscFlags |= RUNTIME_FLAG_RENDERED;
    } else {
        customBits |= CUSTOM_BIT_NOT_RENDERED;
        render = 0;
        standardGame->ucMiscFlags &= ~RUNTIME_FLAG_RENDERED;
    }
    standardGame->ulCustomBits = customBits;

    if (render != 0 && _hSprObj->lNumberOfElementsSta > 0) {
        tdstEngineObject_* owner;

        if (SUPER_OBJECT_EXCLU_LIGHT(_hSprObj)) {
            GLI_xSetViewportLightsOnOff(0);
        }

        owner = _hSprObj->hLinkedObject;
        msLight = owner->h_MSLight;
        if (msLight != NULL) {
            light = fn_p_stMSLightGetLight(msLight);
            if (light != NULL) {
                light->bValid = 1;
                if (fn_bMSLightGetLocalLight(msLight) != 0) {
                    inv_GLI_vRecordAddLightToViewport(light);
                } else {
                    light = NULL;
                }
            }
        }

        GLI_vResetTableOfActiveLightsForChar(1);
        if (bounds != NULL) {
            POS_tdstMatrix_* matrix;

            lightIndex = 0;
            matrix = _hSprObj->gu_hGlobalMatrix;
            radius = radius * POS_fn_xGetMaxScale(matrix);
            POS_fn_vMulMatrixVertex(&worldCenter, matrix, sphere);
            if (gs_CurrentNumberOfLightInViewport > 0) {
                do {
                    if (GLI_bDoesLightAffectBV(gs_aDEF_p_CurrentListOfLight[lightIndex],
                                               &worldCenter, radius) != 0) {
                        g_ucTableOfActiveLightsForChar[lightIndex] = 1;
                    } else {
                        g_ucTableOfActiveLightsForChar[lightIndex] = 0;
                    }
                    lightIndex++;
                } while (lightIndex < gs_CurrentNumberOfLightInViewport);
            }
        } else {
            memset(g_ucTableOfActiveLightsForChar, 1, gs_CurrentNumberOfLightInViewport);
        }

        for (child = _hSprObj->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
            if (child->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
                HIE_fn_vSendCharacterModulesToViewPort(_hVpt, child, _lDrawMask);
            }
        }

        if (SUPER_OBJECT_EXCLU_LIGHT(_hSprObj)) {
            GLI_xSetViewportLightsOnOff(1);
        }
        if (light != NULL) {
            inv_GLI_vRecordPopLightsInViewport(1);
        }
    }

    if (customBits & CUSTOM_BIT_ZFAR_TRANSPARENCY) {
        GEO_g_xZFar = savedZFar;
        GEO_vSetZFarTransparencyZone(savedZoneWidth);
        _hSprObj->flags = flags;
        _hSprObj->gu_lDrawModeMask = savedDrawMode;
    }

    HIE_fn_vPopMatrix();
    if (fatherPushed != 0) {
        HIE_fn_vPopMatrix();
    }
}

void func_00149400(void* viewport, HIE_tdstSuperObject_* superObject, s32 mask, s32 cullingMask)
{
    f32 radius = 0.0f;
    HIE_tdstSuperObject_* child;
    u32 customBits;
    s32 enabled;

    HIE_fn_vPushSuperObjectMatrix(superObject);
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        if (superObject->lNumberOfElementsSta > 0) {
            void* bounds = superObject->hVisualBoundingVolume;
            tdstEngineObject_* object = superObject->hLinkedObject;
            gu_tdstStandardGame* standardGame = object->h_StandardGame;
            MTH3D_tdstVector_ center;

            if (bounds != NULL) {
                MTH3D_tdstVector_* boundsCenter = inv_GEO_fn_pIdentity(bounds);

                center = *boundsCenter;
                radius = inv_GEO_fn_xGetField0C(bounds);
            }

            if (cullingMask != CULLING_ALL_PLANES && bounds != NULL) {
                cullingMask = GEO_lCullingSphere(viewport, &center, radius,
                                                 superObject->gu_hGlobalMatrix, cullingMask);
            } else {
                cullingMask = CULLING_ALL_PLANES;
            }

            mask |= DRAW_MASK_CHARACTER_DEFAULTS;
            mask &= superObject->gu_lDrawModeMask;
            customBits = fn_ulStandardGameGetCustomBitsSO(superObject);
            if (DRAW_MASK_VISIBLE(mask) && !SUPER_OBJECT_HIDDEN(superObject) && cullingMask != 0) {
                enabled = 1;
                standardGame->ucMiscFlags |= RUNTIME_FLAG_RENDERED;
                customBits &= ~CUSTOM_BIT_NOT_RENDERED;
            } else {
                customBits |= CUSTOM_BIT_NOT_RENDERED;
                enabled = 0;
                standardGame->ucMiscFlags &= ~RUNTIME_FLAG_RENDERED;
            }
            standardGame->ulCustomBits = customBits;

            if (enabled != 0) {
                for (child = superObject->hFirstElementSta; child != NULL;
                     child = child->hNextBrotherSta) {
                    if (child->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
                        func_00149400(viewport, child, mask, cullingMask);
                    }
                }
            }
        }
    } else {
        mask &= superObject->gu_lDrawModeMask;
        if (DRAW_MASK_VISIBLE(mask) && !SUPER_OBJECT_HIDDEN(superObject)) {
            fn_vSendOneModuleToViewPort(viewport, superObject, mask);
        }

        for (child = superObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
            if (child->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
                func_00149400(viewport, child, mask, cullingMask);
            }
        }
    }
    HIE_fn_vPopMatrix();
}

s32 gu_GLI_RW_vInitTextureModule(void* _hVpt, s32 _lDrawMask)
{
    s8 fogWasOn;
    s32 mipMapping;
    HIE_tdstSuperObject_** superObjects;
    s16 i;

    fogWasOn = GLI_cGlobalFogIsOn;
    GLI_cGlobalFogIsOn = 0;
    GLI_vFogOff();
    mipMapping = GLI_fn_SinusEffect_vGetState();
    if (GLI_fn_SinusEffect_cIsSecondViewportAffected() == 0) {
        GLI_fn_SinusEffect_vSetState(0);
    }
    superObjects = gd_hFixSPO;
    for (i = 0; i < g_xFixIndex; i++) {
        HIE_tdstSuperObject_* superObject = *superObjects;

        func_00149400(_hVpt, superObject, superObject->gu_lDrawModeMask & _lDrawMask, 0);
        superObjects++;
    }
    GLI_fn_SinusEffect_vSetState(mipMapping);
    GLI_cGlobalFogIsOn = fogWasOn;
    if (fogWasOn != 0) {
        GLI_vFogOn();
    }
    return 1;
}

void* func_00149738(HIE_tdstSuperObject_* superObject)
{
    u32 type = superObject->lTypeOfLinkedObject;
    void* result = NULL;

    if (type == HIE_TYPE_GEOMETRIC_OBJECT_inv) {
        result = superObject->hLinkedObject;
    } else if (type & HIE_TYPE_PHYSICAL_OBJECT_inv) {
        PO_tdstPhysicalObject_* geometry = superObject->hLinkedObject;

        result = geometry->_hVisualSet;
    }
    return result;
}
