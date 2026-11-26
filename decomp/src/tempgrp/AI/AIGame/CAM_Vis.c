#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "COL/COL.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"

#include "ps2/copy_types.h"

#include "include_asm.h"
#include "GAM/GAM.h"

extern f32 inv_CAM_xOffsetMinDistPerso[];
extern s16 inv_CAM_xTickFindBetterPosIfNoMove[8];

void* memcpy(void*, const void*, unsigned int);

void CAM_fn_vComputeCineinfoWorkForAPosition(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* position);

extern u16 guw_NbVectorArrayCpt;

void qsort(void* base, s32 n, s32 size, void* cmp);

void CAM_fn_vMakeAsNewIdealPosition(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* wanted);

s32 func_001E4B98(HIE_tdstSuperObject_* context, s32 value)
{
    s32* base;
    s32 count;
    s32* entry;

    base = context->hLinkedObject;
    count = base[0x2C / 4];
    entry = (s32*) base[0x28 / 4];
    if (count > 0) {
        do {
            if (*entry == value) {
                return 1;
            }
            count = count - 1;
            entry = entry + 1;
        } while (count > 0);
    }
    return 0;
}

s32 CAM_fn_cSendSegmentInDynamicWorld(MTH3D_tdstVector_* from, MTH3D_tdstVector_* to, HIE_tdstSuperObject_* sector,
                  HIE_tdstSuperObject_* camera, void* out)
{
    MTH3D_tdstVector_ delta;
    SECT_tdstListOfSOCharacters_* node;
    HIE_tdstSuperObject_* obj;
    tdstEngineObject_* entry;
    gu_tdstStandardGame* stdGame;
    CollisionSet* collSet;
    u8 cameraKind = inv_HIE_fn_p_stGetEngineObject(camera)->h_CollSet->ucCharacterPriority;

    MTH3D_M_vSubXYZ_inv(&delta, to, from);

    node = ((SECT_tdstSector_*) sector->hLinkedObject)->stListOfCharacters.hFirstElementDyn;
    while (node != 0) {
        obj = node->hPointerOfCharacter;
        entry = obj->hLinkedObject;
        collSet = entry->h_CollSet;
        stdGame = entry->h_StandardGame;
        if ((collSet != 0 && collSet->ucCharacterPriority == cameraKind) ||
            (stdGame != 0 && (stdGame->ulCustomBits & 0x02000000) != 0)) {
            if (obj->lNumberOfElementsSta != 0 &&
                HIE_bDetectIntersectSegmentWithSuperObject(from, &delta, obj, out)) {
                return 1;
            }
        }
        node = node->hNextBrotherDyn;
    }
    return 0;
}

s32 CAM_fn_cSendSegment(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stSource,
                        MTH3D_tdstVector_* _p_stDest, void* cineinfo)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ local_from;
    MTH3D_tdstVector_ local_to;
    tdstInternalStructurCineinfo_* work;
    SECT_tdstSector_* sector;
    HIE_tdstSuperObject_* camera;
    HIE_tdstSuperObject_* targeted;
    HIE_tdstSuperObject_* primary;
    HIE_tdstSuperObject_* fallback;
    HIE_tdstSuperObject_* node;
    SECT_tdstListOfSOSectorsInCollisionInteraction_* items;
    f32 scale;
    s32 flag;
    s32 count;
    s32 ix;
    s32 iy;
    s32 iz;
    s32 jx;
    s32 jy;
    s32 jz;

    if (_p_stSource->xX == _p_stDest->xX && _p_stSource->xY == _p_stDest->xY && _p_stSource->xZ == _p_stDest->xZ) {
        return 0;
    }

    work = _p_stStruct->hCineinfoWork;
    scale = 1000.0f;
    flag = ((work->uwIAFlags >> 4) ^ 1) & 1;
    local_from = *(MTH3D_tdstVector_*) _p_stSource;
    local_to = *(MTH3D_tdstVector_*) _p_stDest;
    ix = inv_MTH_fn_lToInteger(local_from.xX * scale);
    iy = inv_MTH_fn_lToInteger(local_from.xY * scale);
    iz = inv_MTH_fn_lToInteger(local_from.xZ * scale);
    jx = inv_MTH_fn_lToInteger(local_to.xX * scale);
    jy = inv_MTH_fn_lToInteger(local_to.xY * scale);
    jz = inv_MTH_fn_lToInteger(local_to.xZ * scale);
    MTH3D_M_vSetXYZ_inv(&local_from, (f32) ix / scale, (f32) iy / scale, (f32) iz / scale);
    MTH3D_M_vSetXYZ_inv(&local_to, (f32) jx / scale, (f32) jy / scale, (f32) jz / scale);

    camera = _p_stStruct->hSuperObjCamera;
    targeted = work->hSuperObjectTargeted;
    fallback = inv_HIE_fn_p_stGetEngineObject(camera)->h_SectInfo->hCurrentSector;
    primary = fallback;
    if (targeted != (HIE_tdstSuperObject_*) 0) {
        primary = inv_HIE_fn_p_stGetEngineObject(targeted)->h_SectInfo->hCurrentSector;
    }

    MTH3D_M_vSubXYZ_inv(&delta, &local_to, &local_from);

    if (cCanTestStatic != 0) {
        if (HIE_bDetectIntersectSegmentWithSuperObject(&local_from, &delta, primary, cineinfo) !=
            0) {
            return 1;
        }
    }
    if (flag != 0) {
        if (CAM_fn_cSendSegmentInDynamicWorld(&local_from, &local_to, primary, camera, cineinfo) != 0) {
            return 1;
        }
    }
    if (primary != fallback) {
        if (cCanTestStatic != 0) {
            if (HIE_bDetectIntersectSegmentWithSuperObject(&local_from, &delta, fallback,
                                                            cineinfo) != 0) {
                return 1;
            }
        }
        if (flag != 0) {
            if (CAM_fn_cSendSegmentInDynamicWorld(&local_from, &local_to, fallback, camera, cineinfo) != 0) {
                return 1;
            }
        }
    }

    sector = primary->hLinkedObject;
    count = sector->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt;
    items = sector->stListOfSectorsInCollisionInteraction.hFirstElementOpt;
    if (count > 0) {
        do {
            node = items->hPointerOfSectorSO;
            if (node != fallback) {
                if (cCanTestStatic != 0) {
                    if (HIE_bDetectIntersectSegmentWithSuperObject(&local_from, &delta, node,
                                                                    cineinfo) != 0) {
                        return 1;
                    }
                }
                if (flag != 0) {
                    if (CAM_fn_cSendSegmentInDynamicWorld(&local_from, &local_to, node, camera, cineinfo) !=
                        0) {
                        return 1;
                    }
                }
            }
            count = count - 1;
            items = items + 1;
        } while (count > 0);
    }

    if (primary != fallback) {
        sector = fallback->hLinkedObject;
        count = sector->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt;
        items = sector->stListOfSectorsInCollisionInteraction.hFirstElementOpt;
        if (count > 0) {
            do {
                node = items->hPointerOfSectorSO;
                if (func_001E4B98(primary, (s32) node) == 0 && node != primary) {
                    if (cCanTestStatic != 0) {
                        if (HIE_bDetectIntersectSegmentWithSuperObject(&local_from, &delta, node,
                                                                        cineinfo) != 0) {
                            return 1;
                        }
                    }
                    if (flag != 0) {
                        if (CAM_fn_cSendSegmentInDynamicWorld(&local_from, &local_to, node, camera, cineinfo) != 0) {
                            return 1;
                        }
                    }
                }
                count = count - 1;
                items = items + 1;
            } while (count > 0);
        }
    }
    return 0;
}

s32 CAM_fn_cSendMultiSegment(f32 w0, f32 w1, CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stSourcePos,
                             MTH3D_tdstVector_* _p_stTargetPos, s32 _lNbRay, s32 _hCineinfo)
{
    MTH3D_tdstVector_ dir;
    MTH3D_tdstVector_ v;
    MTH3D_tdstVector_ stSourcePos;
    MTH3D_tdstVector_ u;
    MTH3D_tdstVector_ stTargetPos;
    MTH_stRotation params;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ off;
    f32 cx;
    f32 cy;
    f32 cz;
    s32 i;

    if (_lNbRay == 1) {
        return CAM_fn_cSendSegment(_p_stStruct, _p_stSourcePos, _p_stTargetPos, (void*) _hCineinfo);
    }
    inv_MTH3D_fn_vSubXYZ(&dir, _p_stTargetPos, _p_stSourcePos);
    {
        f32 len2;
        f32 len;
        inv_MTH3D_fn_vNormalizeXYZ(&dir);
    }
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    cx = inv_MTH_fn_xMulSub(dir.xY, up.xZ, dir.xZ, up.xY);
    cy = inv_MTH_fn_xMulSub(dir.xZ, up.xX, dir.xX, up.xZ);
    cz = inv_MTH_fn_xMulSub(dir.xX, up.xY, dir.xY, up.xX);
    MTH3D_M_vSetXYZ_inv(&off, cx, cy, cz);
    if (cx == 0.0f && cy == 0.0f && cz == 0.0f) {
        up.xX = 1.0f;
        up.xZ = 0.0f;
        DNM_p_stVectorCrossProduct(&off, &dir, &up);
    }
    {
        f32 len2;
        f32 len;
        inv_MTH3D_fn_vNormalizeXYZ(&off);
    }
    inv_MTH3D_fn_vKTimesXYZ(&off, w0, &off);
    params.m_stAxis = dir;
    params.m_xAngle = MTH_TWO_PI_inv / (f32) _lNbRay;
    for (i = 0; i < _lNbRay; i++) {
        inv_MTH3D_fn_vKTimesXYZ(&v, w0, &off);
        inv_MTH3D_fn_vKTimesXYZ(&u, w1, &off);
        inv_MTH3D_fn_vAddXYZ(&stSourcePos, &v, _p_stSourcePos);
        inv_MTH3D_fn_vAddXYZ(&stTargetPos, &u, _p_stTargetPos);
        if (CAM_fn_cSendSegment(_p_stStruct, &stSourcePos, &stTargetPos, (void*) _hCineinfo) != 0) {
            return 1;
        }
        MTH_p_stRotationMulVector(&params, &off, &off);
    }
    return 0;
}

s32 CAM_fn_cCameraCantGo(CAM_tdstUpdateCamera_* _p_stStruct, f32* _p_stSourcePos, f32* _p_stTargetPos)
{
    s32 result;

    result = 0;
    if (_p_stSourcePos[0] != _p_stTargetPos[0] || _p_stSourcePos[1] != _p_stTargetPos[1] || _p_stSourcePos[2] != _p_stTargetPos[2]) {
        result = CAM_fn_cSendMultiSegment(CAM_g_stCameraConstants.xRayCameraSphereToGo1,
                                          CAM_g_stCameraConstants.xRayCameraSphereToGo2, _p_stStruct,
                                          (MTH3D_tdstVector_*) _p_stSourcePos, (MTH3D_tdstVector_*) _p_stTargetPos,
                                          (s32) CAM_g_stCameraConstants.xNumRayCameraCanGo, 0);
        result = (result != 0);
    }
    return result;
}

s32 CAM_fn_cCameraCantGoFailure(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stSourcePos, MTH3D_tdstVector_* _p_stTargetPos)
{
    MTH3D_tdstVector_ v;
    f32 len2;
    f32 len;
    f32 xInvLength;
    f32 scale;

    inv_MTH3D_fn_vSubXYZ(&v, _p_stTargetPos, _p_stSourcePos);
    len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(v.xX, v.xY, v.xZ);
    len = MTH_fn_xSqrtOpt2(len2);
    scale = len;
    len = len + 0.35f;
    if (scale != 0.0f) {
        xInvLength = 1.0f / scale;
        inv_MTH3D_fn_vKTimesXYZ(&v, xInvLength, &v);
    }
    MTH3D_M_vSetXYZ_inv(&v, len * v.xX + _p_stSourcePos->xX, len * v.xY + _p_stSourcePos->xY,
                        len * v.xZ + _p_stSourcePos->xZ);
    return CAM_fn_cSendMultiSegment(CAM_g_stCameraConstants.xRayCameraSphereToGo1,
                                    CAM_g_stCameraConstants.xRayCameraSphereToGo2, _p_stStruct, _p_stSourcePos, &v,
                                    CAM_g_stCameraConstants.xNumRayCameraCanGo, 0);
}

s32 CAM_fn_cCameraCantSee(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stSourcePos,
                          MTH3D_tdstVector_* _p_stTargetPos)
{
    CAM_tdstUpdateCamera_* savedCamera;
    MTH3D_tdstVector_* savedFrom;
    MTH3D_tdstVector_* savedTo;

    savedTo = _p_stTargetPos;
    savedFrom = _p_stSourcePos;
    savedCamera = _p_stStruct;
    if (cCanTestStatic != 0) {
        CAM_fn_vResetArrayOfTransparentSuperObject(savedCamera->hCineinfo);
    }
    return CAM_fn_cSendMultiSegment(CAM_g_stCameraConstants.xRayCameraSphereToSee1,
                                    CAM_g_stCameraConstants.xRayCameraSphereToSee2, savedCamera, savedFrom, savedTo,
                                    (s32) CAM_g_stCameraConstants.xNumRayCameraCanSee,
                                    (s32) savedCamera->hCineinfo);
}

typedef s32 (*E55_Fn_)(CAM_tdstUpdateCamera_* a0, MTH3D_tdstVector_* a1, MTH3D_tdstVector_* a2);

s32 CAM_fn_cIsWantedPosIncorrect(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stWantedPos, s8 _cMode)
{
    MTH3D_tdstVector_ delta;
    s32 result;
    s32 savedFlags;
    E55_Fn_ testSegment;

    savedFlags = _cMode;
    MTH3D_M_vSubXYZ_inv(&delta, _p_stWantedPos, &_p_stStruct->stTargetedPersoRealPos);
    if (MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ)) <
        _p_stStruct->hCineinfoWork->xBoundDistMin + inv_CAM_xOffsetMinDistPerso[0]) {
        return 1;
    }
    if (savedFlags & 1) {
        if (savedFlags & 4) {
            testSegment = (E55_Fn_) CAM_fn_cCameraCantGoFailure;
        } else {
            testSegment = (E55_Fn_) CAM_fn_cCameraCantSee;
        }
        result = testSegment(_p_stStruct, &_p_stStruct->stTargetedPersoRealPos, _p_stWantedPos);
        if (result != 0) {
            return 1;
        }
    }
    if (savedFlags & 2) {
        result = CAM_fn_cCameraCantGo(_p_stStruct, &_p_stStruct->stCurrentCameraPos.xX, &_p_stWantedPos->xX);
        if (result != 0) {
            return 1;
        }
    }
    return 0;
}

s32 func_001E56B0(CAM_tdstUpdateCamera_* camera, f32* offset, f32* out, s32 flags)
{
    MTH3D_tdstVector_ position;
    f32* savedOut;
    s32 result;

    savedOut = out;
    flags = (s8) flags;
    MTH3D_M_vSetXYZ_inv(&position, offset[0] + camera->stTargetedPersoPos.xX,
                        offset[1] + camera->stTargetedPersoPos.xY,
                        offset[2] + camera->stTargetedPersoPos.xZ);
    result = CAM_fn_cIsWantedPosIncorrect(camera, &position, flags);
    if (result != 0) {
        return 0;
    }
    *(MTH3D_tdstVector_*) savedOut = position;
    return 1;
}

f32 CAM_fnx_ComputePosPound(CAM_tdstUpdateCamera_* _pst_Struct, MTH3D_tdstVector_* _pst_Dir)
{
    MTH3D_tdstVector_ stPos;
    tdstInternalStructurCineinfo_ st_Temp;
    tdstInternalStructurCineinfo_* pst_Memo;
    f32 x_P3;
    f32 x_P2;
    f32 x_P1;
    f32 x_Pound;
    f32 x_B;
    f32 x_C;
    f32 x_D;
    f32 x_E;
    s32 l_1;
    s32 l_2;
    s32 l_3;
    s32 l_4;
    s32 l_5;
    s32 l_6;

    MTH3D_M_vAddXYZ_inv(&stPos, &_pst_Struct->stTargetedPersoPos, _pst_Dir);
    pst_Memo = _pst_Struct->hCineinfoWork;
    st_Temp.hSuperObjectTargeted = pst_Memo->hSuperObjectTargeted;
    st_Temp.hSecondSuperObjectTargeted = pst_Memo->hSecondSuperObjectTargeted;
    st_Temp.cChannel = pst_Memo->cChannel;
    _pst_Struct->hCineinfoWork = &st_Temp;
    CAM_fn_vComputeCineinfoWorkForAPosition(_pst_Struct, &stPos);
    _pst_Struct->hCineinfoWork = pst_Memo;

    x_D = gst_Ideal.xAngleAlpha;
    if (x_D > MTH_PI_inv) {
        x_Pound = MTH_TWO_PI_inv - x_D;
    } else {
        x_Pound = x_D;
    }
    x_C = st_Temp.xAngleAlpha;
    if (x_C > MTH_PI_inv) {
        x_B = MTH_TWO_PI_inv - x_C;
    } else {
        x_B = x_C;
    }
    x_P1 = inv_MTH_fn_xAbsolute(x_Pound - x_B);
    l_1 = inv_MTH_fn_lToInteger(x_P1 * 100.0f);
    l_2 = inv_MTH_fn_lToInteger(x_P1 * 100.0f * 2.0f);
    x_P1 = (f32) (l_2 - l_1);
    x_P1 = x_P1 * 0.01f;
    x_E = gst_Ideal.xAngleTheta;
    if (x_E > MTH_PI_inv) {
        x_Pound = MTH_TWO_PI_inv - x_E;
    } else {
        x_Pound = x_E;
    }
    x_B = st_Temp.xAngleTheta;
    if (x_B > MTH_PI_inv) {
        x_B = x_B - MTH_TWO_PI_inv;
    }
    x_P2 = inv_MTH_fn_xAbsolute(x_Pound - x_B) * 100.0f;
    x_P3 = inv_MTH_fn_xAbsolute(gst_Ideal.xDistMin - st_Temp.xDistMin) * 100.0f;
    l_5 = inv_MTH_fn_lToInteger(x_P2);
    l_3 = inv_MTH_fn_lToInteger(x_P3);
    l_6 = inv_MTH_fn_lToInteger(x_P2 * 2.0f);
    l_4 = inv_MTH_fn_lToInteger(x_P3 * 2.0f);
    return x_P1 + (f32) (l_6 - l_5) * 0.1f + (f32) (l_4 - l_3) * 0.01f * 0.1f;
}

s32 CAM_fnc_IsDirectionCorrect(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _pst_Dir, f32 _x_Pound,
                               f32 _x_PoundRef, s8 _c_Mode)
{
    MTH3D_tdstVector_ stTempVector;
    tdstInternalStructurCineinfo_* hWork;
    f32 lengthSquared;
    f32 length;

    MTH3D_M_vAddXYZ_inv(&stTempVector, _pst_Dir, &_p_stStruct->stTargetedPersoRealPos);
    if (_c_Mode & 8) {
        if (_x_PoundRef <= _x_Pound)
            return 0;
        _x_PoundRef = _x_PoundRef - _x_Pound;
        if (_x_PoundRef < 0.05f)
            return 0;
        if (CAM_fn_cCanResetConstraintMoveVisFailure(_p_stStruct, &stTempVector) == 0)
            return 0;
    }
    hWork = _p_stStruct->hCineinfoWork;
    lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(_pst_Dir->xX, _pst_Dir->xY, _pst_Dir->xZ);
    length = MTH_fn_xSqrtOpt2(lengthSquared);
    if (length < hWork->xBoundDistMin + CAM_g_stCameraConstants.xOffsetMinDistPerso)
        return 0;
    if ((_c_Mode & 4) == 0) {
        if (CAM_fn_bEqualVectorWithEpsilon(&stTempVector, &_p_stStruct->stWantedCameraPos,
                                           CAM_g_stCameraConstants.xEpsilonForEqualVectors) != 0)
            return 0;
    }
    if ((_c_Mode & 0xC) != 4) {
        if (CAM_fn_bEqualVectorWithEpsilon(&stTempVector, &_p_stStruct->stCurrentCameraPos,
                                           CAM_g_stCameraConstants.xEpsilonForEqualVectors) != 0)
            return 0;
    }
    return 1;
}

typedef struct CameraVisibilityEntry {
    MTH3D_tdstVector_ position;
    f32 score;
} CameraVisibilityEntry;

extern CameraVisibilityEntry gast_VectorArrayCpt[];

s32 CAM_fni_ComparePositions(CameraVisibilityEntry* left, CameraVisibilityEntry* right)
{
    f32 leftValue = left->score;
    f32 rightValue = right->score;

    if (leftValue < rightValue) {
        return -1;
    }
    if (rightValue == leftValue) {
        return 0;
    }
    return 1;
}

static inline void inv_AI_fn_vStoreWantedOffset(MTH3D_tdstVector_* offset,
                                                CAM_tdstUpdateCamera_* camera)
{
    inv_MTH3D_fn_vSubXYZ(offset, &camera->stWantedCameraPos, &camera->stTargetedPersoPos);
}

void CAM_fn_vFillVectorArray(CAM_tdstUpdateCamera_* _p_stStruct, s8 _cMode)
{
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ direction;
    tdstInternalStructurCineinfo_ slot;
    CAM_tdstUpdateCamera_ trial;
    MTH3D_tdstVector_* offset;
    s32 count;
    u32 i;
    u8 savedMode;
    u32 stepCount;
    tdstCineinfo_* cineinfo;
    f32 step;
    f32 angle;
    f32 distanceMin;
    f32 distance;
    f32 lengthSquared;
    f32 scale;

    CAM_fn_vInitCameraStructure(_p_stStruct->hSuperObjCamera, &trial);
    trial.hCineinfoWork = &slot;
    slot = gst_Ideal;
    CAM_fn_vComputeReferencePoint(&trial);

    MTH3D_M_vSubXYZ_inv(&direction, &_p_stStruct->stRealWantedCameraPos,
                        &_p_stStruct->stTargetedPersoPos);
    lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(direction.xX, direction.xY, direction.xZ);
    distance = MTH_fn_xSqrtOpt2(lengthSquared);

    if (CAM_fn_bPersoOnFace(_p_stStruct->hCineinfoWork->hSuperObjectTargeted, &target) == 0) {
        MTH3D_M_vSetXYZ_inv(&target, 0.0f, 0.0f, 1.0f);
    }

    if (_cMode & 4) {
        step = CAM_g_stCameraConstants.xBaseAngleComputePosFailure;
    } else {
        step = CAM_g_stCameraConstants.xBaseAngleComputePosNormal;
    }

    count = 0;
    angle = slot.xAngleAlpha;
    cineinfo = trial.hCineinfo;
    savedMode = cineinfo->eState;
    stepCount = inv_MTH_fn_lToInteger(MTH_TWO_PI_inv / step);
    cineinfo->eState = 0;
    distanceMin = slot.xAngleTheta;

    for (i = 0; i < stepCount; i++) {
        slot.xAngleAlpha = angle;
        slot.xAngleAlpha = CAM_fn_xSetAngleInInterval02Pi(angle);
        slot.xAngleTheta = distanceMin;
        trial.hCineinfoWork->uwIAFlags |= 0x40;
        CAM_fn_vComputeMovePosWithDynTheta(&trial);
        offset = &gast_VectorArrayCpt[count].position;
        inv_AI_fn_vStoreWantedOffset(offset, &trial);
        gast_VectorArrayCpt[count++].score = CAM_fnx_ComputePosPound(_p_stStruct, offset);

        if (!(CAM_g_stCameraConstants.wFlags & 0x40) && (_cMode & 4)) {
            offset = &gast_VectorArrayCpt[count].position;
            inv_AI_fn_vStoreWantedOffset(offset, &trial);
            inv_MTH3D_fn_vNormalizeXYZ(offset);
            scale = distance * 0.5f;
            inv_MTH3D_fn_vKTimesXYZ(offset, scale, offset);
            gast_VectorArrayCpt[count++].score = CAM_fnx_ComputePosPound(_p_stStruct, offset);

            if (_cMode & 8) {
                offset = &gast_VectorArrayCpt[count].position;
                inv_AI_fn_vStoreWantedOffset(offset, &trial);
                inv_MTH3D_fn_vNormalizeXYZ(offset);
                inv_MTH3D_fn_vKTimesXYZ(offset, distance, offset);
                gast_VectorArrayCpt[count++].score = CAM_fnx_ComputePosPound(_p_stStruct, offset);
            }
        }

        slot.xAngleTheta = 0.0f;
        trial.hCineinfoWork->uwIAFlags |= 0x40;
        CAM_fn_vComputeMovePosWithDynTheta(&trial);
        offset = &gast_VectorArrayCpt[count].position;
        inv_AI_fn_vStoreWantedOffset(offset, &trial);
        gast_VectorArrayCpt[count++].score = CAM_fnx_ComputePosPound(_p_stStruct, offset);

        slot.xAngleTheta = MTH_QUARTER_PI_inv;
        trial.hCineinfoWork->uwIAFlags |= 0x40;
        CAM_fn_vComputeMovePosWithDynTheta(&trial);
        offset = &gast_VectorArrayCpt[count].position;
        inv_AI_fn_vStoreWantedOffset(offset, &trial);
        gast_VectorArrayCpt[count++].score = CAM_fnx_ComputePosPound(_p_stStruct, offset);

        slot.xAngleTheta = -MTH_QUARTER_PI_inv;
        trial.hCineinfoWork->uwIAFlags |= 0x40;
        CAM_fn_vComputeMovePosWithDynTheta(&trial);
        offset = &gast_VectorArrayCpt[count].position;
        inv_AI_fn_vStoreWantedOffset(offset, &trial);
        gast_VectorArrayCpt[count++].score = CAM_fnx_ComputePosPound(_p_stStruct, offset);

        angle += step;
    }

    guw_NbVectorArrayCpt = count;
    cineinfo = trial.hCineinfo;
    cineinfo->eState = savedMode;
}

s32 CAM_fn_cFindNewGoodIdealPos(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stComputeVector, s8 _cMode)
{
    MTH3D_tdstVector_ delta;
    CameraVisibilityEntry* entry;
    u32 i;
    f32 dist;

    dist = _p_stComputeVector->xZ = _p_stComputeVector->xY = _p_stComputeVector->xX = 0.0f;
    if (_cMode & 8) {
        if (_cMode & 4) {
            MTH3D_M_vSubXYZ_inv(&delta, &_p_stStruct->stCurrentCameraPos,
                                &_p_stStruct->stTargetedPersoPos);
        } else {
            MTH3D_M_vSubXYZ_inv(&delta, &_p_stStruct->stWantedCameraPos,
                                &_p_stStruct->stTargetedPersoPos);
        }
        dist = CAM_fnx_ComputePosPound(_p_stStruct, &delta);
    }
    CAM_fn_vFillVectorArray(_p_stStruct, _cMode);
    qsort(gast_VectorArrayCpt, guw_NbVectorArrayCpt, 0x10, CAM_fni_ComparePositions);
    for (i = 0; i < guw_NbVectorArrayCpt; i++) {
        entry = &gast_VectorArrayCpt[i];
        if (CAM_fnc_IsDirectionCorrect(_p_stStruct, &entry->position, entry->score, dist, _cMode) != 0) {
            if (func_001E56B0(_p_stStruct, (f32*) entry, &_p_stComputeVector->xX, _cMode) != 0) {
                return i ? 2 : 1;
            }
        }
    }
    return 0;
}

void CAM_fn_vForceFailureVisibility(CAM_tdstUpdateCamera_* _p_stStruct)
{
    MTH3D_tdstVector_ reportPosition;
    MTH3D_tdstVector_ delta;

    if (CAM_fn_cFindNewGoodIdealPos(_p_stStruct, &reportPosition, (inv_CAM_wFlags[0] & 4) ? 13 : 5) != 0) {
        MTH3D_M_vSubXYZ_inv(&delta, &reportPosition, &_p_stStruct->stCurrentCameraPos);
        _p_stStruct->hCineinfo->eState = 2;
        _p_stStruct->hCineinfo->stLastFailedWanted = reportPosition;
    }
}

void CAM_fn_vComputeFailureVisibility(CAM_tdstUpdateCamera_* _p_stStruct)
{
    if (_p_stStruct->hCineinfo->eState != CAM_e_State_FailureVisibility) {
        return;
    }
    CAM_fn_vForceCameraPos(_p_stStruct->hSuperObjCamera, &_p_stStruct->hCineinfo->stLastFailedWanted);
    CAM_fn_vForceCameraTgt(_p_stStruct->hSuperObjCamera, &_p_stStruct->stCptPos.stTarget,
                           &_p_stStruct->stCptPos.stRefAxisZ);
    _p_stStruct->stCurrentCameraPos = _p_stStruct->hCineinfo->stLastFailedWanted;
    CAM_fn_vMakeAsNewIdealPosition(_p_stStruct, &_p_stStruct->hCineinfo->stLastFailedWanted);
}

void CAM_fn_vTestBadDistance(CAM_tdstUpdateCamera_* update)
{
    tdstCineinfo_* cineinfo;
    MTH3D_tdstVector_ delta;
    f32 distance;

    cineinfo = update->hCineinfo;
    if (cineinfo->eState >= 2) {
        return;
    }
    if ((*(u32*) &update->hCineinfoWork->uwDNMFlags & 0x30000000) != 0) {
        return;
    }
    if ((cineinfo->ucVolIAFlags & 2) != 0) {
        return;
    }

    MTH3D_M_vSubXYZ_inv(&delta, &update->stCurrentCameraPos, &update->stTargetedPersoRealPos);
    distance = MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ);
    distance = MTH_fn_xSqrtOpt2(distance);
    if (update->hCineinfoWork->xBoundDistMin < distance) {
        return;
    }

    if ((update->p_stReport->m_cBitField & 1) == 0) {
        if ((cineinfo->ucVolIAFlags & 4) == 0) {
            return;
        }
    }

    if (CAM_fn_cIsWantedPosIncorrect(update, &update->stWantedCameraPos, 3) != 0) {
        MTH3D_tdstVector_ report;
        s32 mode;

        update->hCineinfo->xCounter1 = 0.0f;
        mode = (update->hCineinfoWork->uwIAFlags & 8) != 0 ? 2 : 3;
        if (CAM_fn_cFindNewGoodIdealPos(update, &report, mode) == 0) {
            CAM_fn_vForceFailureVisibility(update);
        } else {
            CAM_fn_vMakeAsNewIdealPosition(update, &report);
            update->hCineinfo->ucPerIAFlags |= 8;
        }
    }

    return;
}

void CAM_fn_vTestBadVisibility(CAM_tdstUpdateCamera_* _p_stStruct)
{
    MTH3D_tdstVector_ reportVector;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ stNull;
    f32 xClock;
    f32 xCounter;
    f32 xThreshold;
    f32 xEpsilon;
    f32 dot;
    f32 reportLength;
    f32 deltaLength;
    f32 cur_x;
    f32 target_x;
    f32 target_y;
    f32 cur_y;
    f32 cur_z;
    f32 target_z;
    f32 delta_x;
    f32 delta_y;
    f32 delta_z;
    s32 result;

    {
        tdstCineinfo_* cineinfo;

        cineinfo = _p_stStruct->hCineinfo;
        if (cineinfo->eState >= 2) {
            goto done;
        }
        if ((*(u32*) &_p_stStruct->hCineinfoWork->uwDNMFlags & 0x20080000) != 0 ||
            (cineinfo->ucVolIAFlags & 2) != 0) {
            CAM_fn_vResetArrayOfTransparentSuperObject(cineinfo);
            goto done_restore;
        }
    }
    xClock = inv_fn_xGetEngineClockDelta();
    _p_stStruct->hCineinfo->xCounter2 += xClock * 1000.0f;
    {
        tdstCineinfo_* info;

        info = _p_stStruct->hCineinfo;
        xClock = inv_fn_xGetEngineClockScale();
        xCounter = info->xCounter2;
        xThreshold = (f32) CAM_g_stCameraConstants.xTickTestVisibility;
        xThreshold *= 1000.0f;
        xThreshold *= xClock;
        if (xCounter < xThreshold) {
            goto done_restore;
        }
        _p_stStruct->hCineinfo->xCounter2 = 0.0f;
        info = _p_stStruct->hCineinfo;
        if (info->xCounter3 != 0.0f) {
            goto failed;
        }
        xClock = inv_fn_xGetEngineClockScale() * 10000.0f;
        if (xClock < info->xCounter4) {
            cCanTestStatic = 0;
            goto failed;
        }
    }
    MTH3D_M_vZeroXYZ_inv(&stNull);
    xEpsilon = 0.0001f;
    reportVector = _p_stStruct->p_stReport->m_stAbsolutePrevSpeed.m_stLinear;
    MTH3D_M_vSubXYZ_inv(&delta, &_p_stStruct->stTargetedPersoRealPos,
                        &_p_stStruct->stCurrentCameraPos);
    result = CAM_fn_bEqualVectorWithEpsilon(&reportVector, &stNull, xEpsilon);
    if (result != 0) {
        goto failed;
    }
    result = CAM_fn_bEqualVectorWithEpsilon(&delta, &stNull, xEpsilon);
    if (result != 0) {
        goto failed;
    }
    dot = inv_MTH_fn_xMulAdd3Grouped(reportVector.xX, reportVector.xY, reportVector.xZ,
                           reportVector.xX, reportVector.xY, reportVector.xZ);
    reportLength = MTH_fn_xSqrtOpt2(dot);
    if (reportLength != 0.0f) {
        f32 xInvLength;
        xInvLength = 1.0f / reportLength;
        MTH3D_M_vKTimesXYZ_inv(&reportVector, xInvLength, &reportVector);
    }
    dot = inv_MTH_fn_xMulAdd3Grouped(delta.xX, delta.xY, delta.xZ, delta.xX, delta.xY, delta.xZ);
    deltaLength = MTH_fn_xSqrtOpt2(dot);
    if (deltaLength != 0.0f) {
        f32 xInvLength;
        xInvLength = 1.0f / deltaLength;
        inv_MTH3D_fn_vKTimesXYZ(&delta, xInvLength, &delta);
    }
    dot = inv_MTH_fn_xMulAdd3Grouped(delta.xX, delta.xY, delta.xZ, reportVector.xX, reportVector.xY,
                           reportVector.xZ);
    xThreshold = 0.9f;
    dot = inv_MTH_fn_xAbsolute(dot);
    if (xThreshold < dot) {
        cCanTestStatic = 0;
    } else {
        goto failed;
    }
failed: {
    tdstCineinfo_* info;

    result =
        CAM_fn_cCameraCantSee(_p_stStruct, &_p_stStruct->stCurrentCameraPos, &_p_stStruct->stTargetedPersoRealPos);
    if (result == 0) {
        _p_stStruct->hCineinfo->xCounter3 = 0.0f;
        return;
    }
    _p_stStruct->hCineinfo->xCounter3 += inv_fn_xGetEngineClockDelta() * 1000.0f;
    info = _p_stStruct->hCineinfo;
    xClock = inv_fn_xGetEngineClockScale();
    xThreshold = (f32) CAM_g_stCameraConstants.xTickFailureCantSee;
    xThreshold *= 1000.0f;
    xThreshold *= xClock;
    xCounter = info->xCounter3;
    if (xThreshold < xCounter) {
        _p_stStruct->hCineinfo->xCounter3 = 0.0f;
        CAM_fn_vForceFailureVisibility(_p_stStruct);
        goto done_restore;
    }
}
    result = CAM_fn_cIsWantedPosIncorrect(_p_stStruct, &_p_stStruct->stWantedCameraPos, 3);
    if (result == 0) {
        goto done;
    }
    _p_stStruct->hCineinfo->xCounter1 = 0.0f;
    _p_stStruct->hCineinfo->xCounter3 = 0.0f;
    result = CAM_fn_cFindNewGoodIdealPos(_p_stStruct, &reportVector, 3);
    if (result == 0) {
        CAM_fn_vForceFailureVisibility(_p_stStruct);
    } else {
        CAM_fn_vMakeAsNewIdealPosition(_p_stStruct, &reportVector);
        _p_stStruct->hCineinfo->ucPerIAFlags |= 8;
        return;
    }
done_restore:
    (void) xClock;
    (void) xCounter;
done:
    return;
}

void CAM_fn_vTestOppositeCollision(CAM_tdstUpdateCamera_* update)
{
    tdstCineinfo_* cineinfo;
    u8 ucReportFlags;
    MTH3D_tdstVector_ wanted;

    cineinfo = update->hCineinfo;
    if (cineinfo->eState >= 2) {
        return;
    }
    if ((*(u32*) &update->hCineinfoWork->uwDNMFlags & 0x30000000) != 0) {
        return;
    }
    if ((cineinfo->ucVolIAFlags & 2) != 0) {
        return;
    }
    ucReportFlags = update->p_stReport->m_cBitField;
    if ((ucReportFlags & 1) != 0) {
        if ((ucReportFlags & 2) != 0) {
            cineinfo->xCounter1 = 0.0f;
            if (CAM_fn_cFindNewGoodIdealPos(
                    update, &wanted,
                    (update->hCineinfoWork->uwIAFlags & 8) != 0 ? 2 : 3) == 0) {
                CAM_fn_vForceFailureVisibility(update);
            } else {
                CAM_fn_vMakeAsNewIdealPosition(update, &wanted);
            }
        }
    }
}

void CAM_fn_vComputeCineinfoWorkForAPosition(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* position)
{
    MTH3D_tdstVector_ axis;
    MTH3D_tdstVector_ delta;
    f32 horizontal;
    f32 vertical;
    f32 x, y, z;
    f32 length;

    CAM_fn_vComputeRefAxis(update, &axis);
    x = position->xX - update->stTargetedPersoPos.xX;
    y = position->xY - update->stTargetedPersoPos.xY;
    z = position->xZ - update->stTargetedPersoPos.xZ;
    MTH3D_M_vSetXYZ_inv(&delta, x, y, z);
    length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z));
    CAM_fn_vComputeAnglesBetweenTwoVectors(update->hCineinfoWork, &axis, &delta, &horizontal, &vertical,
                  &update->stCptPos.stRefAxisZ);
    update->hCineinfoWork->xDistMin = length;
    update->hCineinfoWork->xDistMax = length;
    update->hCineinfoWork->xAngleAlpha = horizontal;
    update->hCineinfoWork->xAngleTheta = vertical;
}

void CAM_fn_vComputeCineinfoCurrentForAPosition(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* position)
{
    tdstInternalStructurCineinfo_* savedWork;
    tdstInternalStructurCineinfo_* current;

    savedWork = update->hCineinfoWork;
    current = update->hCineinfoCurrent;
    update->hCineinfoWork = current;
    CAM_fn_vComputeCineinfoWorkForAPosition(update, position);
    update->hCineinfoWork = savedWork;
}

void CAM_fn_vMakeAsNewIdealPosition(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* _p_stPosition)
{
    tdstCineinfo_* cineinfo;

    update->hCineinfo->cLastCutAngleSens = 0;
    cineinfo = update->hCineinfo;

    if ((cineinfo->ucPerIAFlags & 2) == 0) {
        *cineinfo->hVisibility = *cineinfo->hCurrent;
        cineinfo->ucPerIAFlags |= 2;
    }

    CAM_fn_vComputeCineinfoCurrentForAPosition(update, _p_stPosition);

    update->hCineinfo->eState = 1;
    update->hCineinfo->ucPerIAFlags |= 4;

    update->hCineinfoWork->xAngleAlpha = update->hCineinfoCurrent->xAngleAlpha;
    update->hCineinfoWork->xAngleTheta = update->hCineinfoCurrent->xAngleTheta;
    update->hCineinfoWork->xDistMin = update->hCineinfoCurrent->xDistMin;
    update->hCineinfoWork->xDistMax = update->hCineinfoCurrent->xDistMax;

    CAM_fn_vComputeMovePos(update);

    update->hCineinfo->stLastFailedOrigin = update->stCurrentCameraPos;
    update->hCineinfo->stLastFailedWanted = *_p_stPosition;
}

s32 CAM_fn_cCanResetConstraintMoveVisFailure(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* vector)
{
    tdstCineinfo_* p_ucCineinfo;
    tdstCineinfo_* p_ucCineinfo2;
    MTH3D_tdstVector_ stToVector;
    MTH3D_tdstVector_ stSegment;
    f32 xClockScale;
    f32 xThreshold;
    f32 xCounter;
    f32 xDistance;
    f32 xSegmentLength;
    f32 xSegmentSquareLength;

    p_ucCineinfo = update->hCineinfo;
    if (p_ucCineinfo->eState == 0) {
        return 1;
    }
    xClockScale = inv_fn_xGetEngineClockScale();
    xThreshold = (f32) (s32) (s16) inv_CAM_xTickFindBetterPosIfNoMove[0];
    xThreshold *= 1000.0f;
    xThreshold *= xClockScale;
    xCounter = p_ucCineinfo->xCounter4;
    if (xThreshold <= xCounter) {
        return 1;
    }
    p_ucCineinfo2 = update->hCineinfo;
    if ((p_ucCineinfo2->ucPerIAFlags & 8) == 0) {
        return 1;
    }
    MTH3D_M_vSubXYZ_inv(&stToVector, vector, &p_ucCineinfo2->stLastFailedOrigin);
    xDistance = MTH_fn_xSqrAddSqrAddSqrOpt2(stToVector.xX, stToVector.xY, stToVector.xZ);
    xDistance = MTH_fn_xSqrtOpt2(xDistance);
    MTH3D_M_vSubXYZ_inv(&stSegment, &p_ucCineinfo2->stLastFailedWanted,
                        &p_ucCineinfo2->stLastFailedOrigin);
    xSegmentSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(stSegment.xX, stSegment.xY, stSegment.xZ);
    xSegmentLength = MTH_fn_xSqrtOpt2(xSegmentSquareLength);
    xSegmentLength = xSegmentLength + 0.5f;
    if (xDistance <= xSegmentLength) {
        return 0;
    }
    return 1;
}

void func_001E6D00(CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstCineinfo_* initial;
    tdstCineinfo_* set_flags;
    tdstCineinfo_* clear_flags;
    void* source;
    void* destination;

    _p_stStruct->hCineinfo->eState = 0;
    CAM_fn_vSetCineinfoCurrentFromVisibility(_p_stStruct->hCineinfo);
    initial = _p_stStruct->hCineinfo;
    source = initial->hVisibility;
    destination = initial->hWork;
    memcpy(destination, source, 0x78);
    set_flags = _p_stStruct->hCineinfo;
    set_flags->ucPerIAFlags |= 4;
    clear_flags = _p_stStruct->hCineinfo;
    clear_flags->ucPerIAFlags &= 0xF7;
    CAM_fn_vComputeMovePosWithDynTheta(_p_stStruct);
}

void CAM_fn_vCheckForABetterPos(CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstCineinfo_* obj;
    f32 scale = 1000.0f;
    f32 delta = inv_fn_xGetEngineClockDelta();
    _p_stStruct->hCineinfo->xCounter1 += delta * scale;
    obj = _p_stStruct->hCineinfo;
    delta = inv_fn_xGetEngineClockScale();
    if ((f32) CAM_g_stCameraConstants.xTickFindBetterPos * scale * delta <= obj->xCounter1) {
        if (!(CAM_g_stCameraConstants.wFlags & 0x20)) {
            if (_p_stStruct->hCineinfo->bCanDoBestPos) {
                s32 x;
                u8 buf[16];
                _p_stStruct->hCineinfo->xCounter1 = 0.0f;
                x = CAM_fn_cFindNewGoodIdealPos(_p_stStruct, (MTH3D_tdstVector_*) buf, 11);
                if (x != 0) {
                    if (x == 1)
                        func_001E6D00(_p_stStruct);
                    else {
                        CAM_fn_vMakeAsNewIdealPosition(_p_stStruct, (MTH3D_tdstVector_*) buf);
                        if (_p_stStruct->hCineinfo->ucPerIAFlags & 8)
                            cJustBetterPos = 1;
                        else
                            cJustBetterPos = 0;
                        _p_stStruct->hCineinfo->ucPerIAFlags &= ~8;
                    }
                    _p_stStruct->hCineinfo->bCanDoBestPos = 0;
                }
            }
        }
    }
}
