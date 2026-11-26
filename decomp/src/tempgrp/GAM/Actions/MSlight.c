#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "AI/AI.h"
#include "SCT/SCT.h"

#define MATRIX_TYPE_IDENTITY 1
#define MATRIX_TYPE_TRANSLATION 2

typedef struct tdstMSLight_ {
    u8 localLight;
    u8 onlyLocalLight;
    u8 bGiroPhare;
    u8 bPulsing;
    MTH3D_tdstVector_ stOffsetVect;
    MTH3D_tdstVector_ stAngle;
    f32 xStepGiro;
    f32 xStepPulse;
    f32 xMaxRangePulse;
    f32 xGiroAngle;
    GLI_tdstLight_* p_stLight;
    void* sectorLightNode;
} tdstMSLight_;

extern const f32 D_00392740;
extern f32 D_00392744;
extern f32 inv_fn_vUpdateLight_x;
extern f32 inv_fn_vUpdateLight_y;
extern f32 inv_fn_vUpdateLight_z;
extern f32 inv_fn_vUpdateLight_step;

f32 atanf(f32 value);
void GLI_xSetLightState(GLI_tdstLight_* light, u8 enabled);

tdstMSLight_* fn_h_LightRealAlloc(void);
void func_00109170(tdstMSLight_* light);
void inv_fn_vMSLightSetSectorLightNode(tdstMSLight_* light, void* lightNode);

typedef u8 (*MSLightStateQuery)(tdstMSLight_* light, s32 linkedType);

static inline tdstMSLight_* inv_GAM_fn_p_stMsLightOf(tdstEngineObject_* object)
{
    return object->h_MSLight;
}

static inline tdstMSLight_* inv_GAM_fn_p_stSuperObjectLight(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv
                                    ? superObject->hLinkedObject
                                    : NULL;

    return object->h_MSLight;
}

static inline tdstMSLight_* inv_GAM_fn_p_stSuperObjectLightOfType(HIE_tdstSuperObject_* superObject, s64 type)
{
    tdstEngineObject_* object =
        superObject->lTypeOfLinkedObject == type ? superObject->hLinkedObject : NULL;

    return object->h_MSLight;
}

GLI_tdstLight_** fn_pp_stMSLightGetLight(tdstMSLight_* light)
{
    return &light->p_stLight;
}

void inv_fn_vMSLightCreateGliLight(tdstMSLight_* light, s32 kind)
{
    GLI_xCreateLight(fn_pp_stMSLightGetLight(light), kind);
}

f32 fn_xMSLightGetFar(tdstMSLight_* light)
{
    f32 nearDistance;
    f32 farDistance;

    GLI_vGetLightNearFar(fn_p_stMSLightGetLight(light), &nearDistance, &farDistance);
    return farDistance;
}

void inv_fn_vMSLightSetLight(tdstMSLight_* light, GLI_tdstLight_* glLight)
{
    light->p_stLight = glLight;
}

f32 fn_xMSLightGetGiroStep(tdstMSLight_* light)
{
    return light->xStepGiro;
}

void fn_vMSLightSetGiroAngle(tdstMSLight_* light, f32 angle)
{
    light->xGiroAngle = angle;
}

f32 fn_xMSLightGetGiroAngle(tdstMSLight_* light)
{
    return light->xGiroAngle;
}

f32 fn_xMSLightGetPulseStep(tdstMSLight_* light)
{
    return light->xStepPulse;
}

void fn_xMSLightSetPulseStep(tdstMSLight_* light, f32 step)
{
    light->xStepPulse = step;
}

f32 fn_xMSLightGetPulseMaxRange(tdstMSLight_* light)
{
    return light->xMaxRangePulse;
}

void fn_vMSLightSetOffset(tdstMSLight_* light, f32 x, f32 y, f32 z)
{
    MTH3D_M_vSetXYZ_inv(&light->stOffsetVect, x, y, z);
}

f32 fn_xMSLightGetOffsetX(tdstMSLight_* light)
{
    return light->stOffsetVect.xX;
}

f32 fn_xMSLightGetOffsetY(tdstMSLight_* light)
{
    return light->stOffsetVect.xY;
}

f32 fn_xMSLightGetOffsetZ(tdstMSLight_* light)
{
    return light->stOffsetVect.xZ;
}

void fn_vLightAlloc(tdstEngineObject_* p_stObject)
{
    p_stObject->h_MSLight = fn_h_LightRealAlloc();
}

tdstMSLight_* fn_h_LightRealAlloc(void)
{
    tdstMSLight_* light = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstMSLight_));

    fn_vMSLightSetGiroAngle(light, 0.0f);
    fn_vMSLightSetOffset(light, 0.0f, 0.0f, 0.0f);
    inv_fn_vMSLightCreateGliLight(light, 1);
    light->sectorLightNode = SECT_fn_hCreateElementLstDNMLights();
    return light;
}

void fn_vLightInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    POS_tdstMatrix_ matrix;
    HIE_tdstSuperObject_* owner;

    switch (eObjectInit) {
    case OTI_WhenGameStart:
        break;
    case OTI_MapLoaded:
        func_00109170(inv_GAM_fn_p_stMsLightOf(p_stObject));
        break;
    case OTI_ReinitTheMap:
    case OTI_LoadSavedGame:
    case 4:
    case OTI_AlwaysCreated:
        break;
    default:
        return;
    }

    owner = p_stObject->h_StandardGame->h_SuperObject;
    if (owner == NULL) {
        return;
    }
    POS_fn_vInvertIsoMatrix(&matrix, owner->gu_hLocalMatrix);
    POS_fn_vNormalizeMatrix(&matrix);
    GLI_xSetLightMatrix(fn_p_stMSLightGetLight(inv_GAM_fn_p_stMsLightOf(p_stObject)), &matrix);
}

void func_00109170(tdstMSLight_* light)
{
}

void func_00109178(void)
{
}

void fn_vLightCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    GLI_tdstLight_* p_stLight = fn_p_stMSLightGetLight(inv_GAM_fn_p_stMsLightOf(_p_stObjectDest));
    void* hLightInSector = fn_hMSLightGetLightNodeInSector(inv_GAM_fn_p_stMsLightOf(_p_stObjectDest));

    *inv_GAM_fn_p_stMsLightOf(_p_stObjectDest) = *inv_GAM_fn_p_stMsLightOf(_p_stObjectSrc);
    inv_fn_vMSLightSetLight(inv_GAM_fn_p_stMsLightOf(_p_stObjectDest), p_stLight);
    GLI_vCopyLight(fn_p_stMSLightGetLight(inv_GAM_fn_p_stMsLightOf(_p_stObjectDest)),
                   fn_p_stMSLightGetLight(inv_GAM_fn_p_stMsLightOf(_p_stObjectSrc)));
    inv_fn_vMSLightSetSectorLightNode(inv_GAM_fn_p_stMsLightOf(_p_stObjectDest), hLightInSector);
}

void* fn_hMSLightGetLightNodeInSector(tdstMSLight_* light)
{
    return light->sectorLightNode;
}

void inv_fn_vMSLightSetSectorLightNode(tdstMSLight_* light, void* lightNode)
{
    light->sectorLightNode = lightNode;
}

void fn_vMSLightSetTypeOfLight(tdstMSLight_* h_MSLight, u16 lTypeOfLight)
{
    GLI_vSetLightType(fn_p_stMSLightGetLight(h_MSLight), lTypeOfLight);
}

u8 fn_ucMSLightGetOnOff(tdstMSLight_* h_MSLight)
{
    s32 lOnOff;

    GLI_xGetLightState(fn_p_stMSLightGetLight(h_MSLight), &lOnOff);
    return lOnOff;
}

void fn_vMSLightSetOnOff(tdstMSLight_* h_MSLight, u8 ucOnOff)
{
    GLI_xSetLightState(fn_p_stMSLightGetLight(h_MSLight), ucOnOff);
}

void fn_vMSLightSetNearFar(tdstMSLight_* h_MSLight, f32 xNear, f32 xFar)
{
    GLI_vSetLightNearFar(fn_p_stMSLightGetLight(h_MSLight), xNear, xFar);
}

void fn_vMSLightSetLittleAlpha(tdstMSLight_* h_MSLight, f32 xLittleAlpha)
{
    f32 xLittleIni;
    f32 xBigIni;

    GLI_vGetLightAlphas(fn_p_stMSLightGetLight(h_MSLight), &xLittleIni, &xBigIni);
    GLI_vSetLightAlphas(fn_p_stMSLightGetLight(h_MSLight), xLittleAlpha, xBigIni);
}

void fn_vMSLightSetBigAlpha(tdstMSLight_* h_MSLight, f32 xBigAlpha)
{
    f32 xLittleIni;
    f32 xBigIni;

    GLI_vGetLightAlphas(fn_p_stMSLightGetLight(h_MSLight), &xLittleIni, &xBigIni);
    GLI_vSetLightAlphas(fn_p_stMSLightGetLight(h_MSLight), xLittleIni, xBigAlpha);
}

void fn_vMSLightSetColor(tdstMSLight_* light, GEO_tdstColor_* color)
{
    GLI_xSetLightColor(fn_p_stMSLightGetLight(light), color);
}

void fn_vMSLightSetColorRGBA(tdstMSLight_* h_MSLight, f32 xR, f32 xG, f32 xB, f32 xA)
{
    GEO_tdstColor_ color;

    color.xR = xR;
    color.xG = xG;
    color.xB = xB;
    color.xA = xA;
    fn_vMSLightSetColor(h_MSLight, &color);
}

GLI_tdstLight_* fn_p_stMSLightGetLight(tdstMSLight_* h_MSLight)
{
    return h_MSLight->p_stLight;
}

void fn_vMSLightSetLocalLight(tdstMSLight_* h_MSLight, u8 bLocalLight)
{
    h_MSLight->localLight = bLocalLight;
    GLI_vSetObjectLighted(fn_p_stMSLightGetLight(h_MSLight), bLocalLight == 0);
    inv_GLI_vSetLightSectorSwitch(fn_p_stMSLightGetLight(h_MSLight), 1);
}

u8 fn_bMSLightGetLocalLight(tdstMSLight_* h_MSLight)
{
    return h_MSLight->localLight;
}

void fn_vMSLightSetOnlyLocalLight(tdstMSLight_* h_MSLight, u8 bOnlyLocalLight)
{
    h_MSLight->onlyLocalLight = bOnlyLocalLight;
}

u8 fn_bMSLightGetOnlyLocalLight(tdstMSLight_* h_MSLight)
{
    return h_MSLight->onlyLocalLight;
}

void fn_vMSLightSetGiroPhare(tdstMSLight_* h_MSLight, u8 enabled, f32 xStep)
{
    h_MSLight->bGiroPhare = enabled;
    h_MSLight->xStepGiro = xStep;
}

u8 fn_bMSLightGetGiroPhare(tdstMSLight_* h_MSLight)
{
    return h_MSLight->bGiroPhare;
}

void fn_vMSLightSetPulse(tdstMSLight_* h_MSLight, u8 enabled, f32 xStep, f32 xMaxRange)
{
    h_MSLight->bPulsing = enabled;
    h_MSLight->xStepPulse = xStep;
    h_MSLight->xMaxRangePulse = xMaxRange;
}

u8 fn_bMSLightGetPulse(tdstMSLight_* h_MSLight)
{
    return h_MSLight->bPulsing;
}

MTH3D_tdstVector_* fn_p_stMSLightGetOffset(tdstMSLight_* light)
{
    return &light->stOffsetVect;
}

void inv_fn_vMSLightGetAngle(tdstMSLight_* light, f32* p_fUSpeed, f32* p_fVSpeed, f32* z)
{
    *p_fUSpeed = light->stAngle.xX;
    *p_fVSpeed = light->stAngle.xY;
    *z = light->stAngle.xZ;
}

f32 fn_xComputeAngleOfPerso(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ xAxis;
    MTH3D_tdstVector_ yAxis;
    MTH3D_tdstVector_ zAxis;
    f32 x;
    f32 y;
    f32 angle;

    POS_fn_vGetRotationVectors(p_SuperObjPerso->gu_hLocalMatrix, &xAxis, &yAxis, &zAxis);
    x = xAxis.xX;
    y = xAxis.xY;
    if (x != 0.0f) {
        angle = atanf(y / x);
        if (x < 0.0f) {
            angle += MTH_PI_inv;
        }
        if (angle < 0.0f) {
            angle += MTH_TWO_PI_inv;
        } else if (angle >= MTH_TWO_PI_inv) {
            angle -= MTH_TWO_PI_inv;
        }
    } else if (y >= 0.0f) {
        angle = MTH_HALF_PI_inv;
    } else {
        angle = -D_00392740;
    }
    return angle;
}

void fn_vUpdateLight(HIE_tdstSuperObject_* hSupObject)
{
    POS_tdstMatrix_ matrix;
    POS_tdstMatrix_ offsetMatrix;
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ offset;
    POS_tdstMatrix_* globalMatrix;
    POS_tdstMatrix_* offsetTransform;
    f32* offsetTranslation;
    f32* lightTranslation;
    f32 persoAngle;
    s64 giroType;

    if (inv_GAM_fn_p_stSuperObjectLight(hSupObject) == NULL) {
        return;
    }
    if (((MSLightStateQuery) fn_ucMSLightGetOnOff)(inv_GAM_fn_p_stSuperObjectLight(hSupObject),
                                                   hSupObject->lTypeOfLinkedObject) == 0) {
        return;
    }

    persoAngle = fn_xComputeAngleOfPerso(hSupObject);
    globalMatrix = hSupObject->gu_hGlobalMatrix;
    matrix = *globalMatrix;
    offsetTransform = &offsetMatrix;
    offsetTranslation = &offsetMatrix.stMatrix.stCol_3.xX;
    lightTranslation = &matrix.stMatrix.stCol_3.xX;

    offset = *fn_p_stMSLightGetOffset(inv_GAM_fn_p_stSuperObjectLight(hSupObject));
    gu_MTH4D_M_vSetIdentityMatrix(offsetTransform);
    if (offsetMatrix.ucType == MATRIX_TYPE_IDENTITY) {
        offsetTransform->ucType = MATRIX_TYPE_TRANSLATION;
    }
    MTH4D_fn_vCopy3DTo4DVector(offsetTranslation, &offset);
    fn_vTurnMatrixZ(offsetTransform, persoAngle);
    giroType = HIE_TYPE_ACTOR_inv;
    MTH4D_fn_vCopy4DTo3DVector(&offset, offsetTranslation);

    MTH4D_fn_vCopy4DTo3DVector(&position, &globalMatrix->stMatrix.stCol_3.xX);
    MTH3D_M_vAddXYZ_inv(&position, &position, &offset);

    inv_fn_vMSLightGetAngle(inv_GAM_fn_p_stSuperObjectLight(hSupObject),
                                         &inv_fn_vUpdateLight_x, &inv_fn_vUpdateLight_y,
                                         &inv_fn_vUpdateLight_z);
    fn_vTurnMatrixZ(&matrix, inv_fn_vUpdateLight_z);
    fn_vTurnMatrixX(&matrix, inv_fn_vUpdateLight_x);
    if (inv_fn_vUpdateLight_y != 0.0f) {
        fn_vTurnMatrixY(&matrix, inv_fn_vUpdateLight_y);
    }
    if (matrix.ucType == MATRIX_TYPE_IDENTITY) {
        matrix.ucType = MATRIX_TYPE_TRANSLATION;
    }
    MTH4D_fn_vCopy3DTo4DVector(lightTranslation, &position);

    if (fn_bMSLightGetGiroPhare(inv_GAM_fn_p_stSuperObjectLight(hSupObject))) {
        inv_fn_vUpdateLight_y =
            fn_xMSLightGetGiroAngle(inv_GAM_fn_p_stSuperObjectLight(hSupObject));
        fn_vTurnMatrixZ(&matrix, inv_fn_vUpdateLight_y);
        if (matrix.ucType == MATRIX_TYPE_IDENTITY) {
            matrix.ucType = MATRIX_TYPE_TRANSLATION;
        }
        MTH4D_fn_vCopy3DTo4DVector(lightTranslation, &position);
        inv_fn_vUpdateLight_y +=
            fn_xMSLightGetGiroStep(inv_GAM_fn_p_stSuperObjectLight(hSupObject));
        if (inv_fn_vUpdateLight_y > D_00392744) {
            inv_fn_vUpdateLight_y = 0.0f;
        }
        fn_vMSLightSetGiroAngle(inv_GAM_fn_p_stSuperObjectLightOfType(hSupObject, giroType),
                                inv_fn_vUpdateLight_y);
    }

    if (fn_bMSLightGetPulse(inv_GAM_fn_p_stSuperObjectLight(hSupObject))) {
        inv_fn_vUpdateLight_x = fn_xMSLightGetFar(inv_GAM_fn_p_stSuperObjectLight(hSupObject));
        inv_fn_vUpdateLight_step =
            fn_xMSLightGetPulseStep(inv_GAM_fn_p_stSuperObjectLight(hSupObject));
        inv_fn_vUpdateLight_x += inv_fn_vUpdateLight_step;
        if (fn_xMSLightGetPulseMaxRange(inv_GAM_fn_p_stSuperObjectLight(hSupObject)) <
                inv_fn_vUpdateLight_x ||
            inv_fn_vUpdateLight_x < 0.0f) {
            inv_fn_vUpdateLight_step = -inv_fn_vUpdateLight_step;
            inv_fn_vUpdateLight_x += 2.0f * inv_fn_vUpdateLight_step;
        }
        fn_xMSLightSetPulseStep(inv_GAM_fn_p_stSuperObjectLight(hSupObject),
                                inv_fn_vUpdateLight_step);
        GLI_vSetLightNearFar(fn_p_stMSLightGetLight(inv_GAM_fn_p_stSuperObjectLight(hSupObject)), 0.0f,
                             inv_fn_vUpdateLight_x);
    }

    GLI_xSetLightMatrix(fn_p_stMSLightGetLight(inv_GAM_fn_p_stSuperObjectLight(hSupObject)), &matrix);
}

GLI_tdstLight_* inv_fn_p_stGetLocalLightOfSuperObject(HIE_tdstSuperObject_* superObject)
{
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        tdstEngineObject_* object = superObject->hLinkedObject;
        tdstMSLight_* light = inv_GAM_fn_p_stMsLightOf(object);

        if (light != NULL && fn_bMSLightGetLocalLight(light)) {
            return fn_p_stMSLightGetLight(light);
        }
    }
    return NULL;
}
