#include "common.h"

#include "GMT/GMT.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#ifdef PC_PORT
#include "GAM/GAM.h"
#endif
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/r5900.h"
#include "GAM/GAM.h"

extern f32 D_00392A80;
extern f32 D_00392A84;
extern f32 D_00392A88;
extern const f32 flt_3934BC[];
f32 MEC_fn_vRoundReal(f32 a, f32 b);
f32 powf(f32 xValue, f32 y);
f32 powf(f32 xValue, f32 y);
extern f32 D_00392AAC;
extern void POS_fn_vGetYAxisVector(void*, f32*);
extern f32 D_00392AB0;
extern f32 D_00392AB4;
extern f32 flt_3934C0[3];

static inline f32 inv_MEC_fn_xCopyFloatBits(f32 value)
{
    union {
        f32 f;
        u32 u;
    } a, b;

    a.f = value;
    b.u = a.u;
    return b.f;
}

static inline s32 inv_MEC_fn_lNodeFlag4000(GMT_tdstGameMaterial_* material)
{
    return material->collideMaterial.xIdentifier & 0x4000;
}

static inline s64 inv_MEC_fn_lBothNodesFlagged(DNM_stMecObstacle* p)
{
    s32 both = 0;
    s32 flag = inv_MEC_fn_lNodeFlag4000(p->contact.hCollidedMaterial);
    if (flag) {
        flag = inv_MEC_fn_lNodeFlag4000(p->contact.hMyMaterial);
        both = flag != 0;
    }
    return both;
}

static inline s32 inv_MEC_fn_lClimbObstacleMarker(DNM_stMecObstacle* it)
{
    return it->m_xCollidedEntity;
}

static inline u16 inv_MEC_fn_uwClimbSurfaceBit(u64 flags)
{
    return (flags >> 6) & 1;
}

s32 MEC_fn_bWallNormal(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stNormal)
{
    f32 one;
    f32 nrm[3];
    f32 dist;
    f32 neg;
    s32 bResult;

    one = 1.0f;
    nrm[0] = 0.0f;
    nrm[1] = 0.0f;
    nrm[2] = one;
    bResult = 0;
    dist = inv_MTH_fn_xMulAdd3(p_stNormal->xX, nrm[0], p_stNormal->xY, nrm[0], p_stNormal->xZ, one);
    neg = inv_MTH_fn_xNegate(p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope);
    if (neg < dist && dist < p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope) {
        bResult = 1;
    }
    return bResult;
}

s32 MEC_fn_bGroundNormal(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stNormal)
{
    f32 one;
    f32 nrm[3];
    f32 dist;
    s32 bResult;

    one = 1.0f;
    nrm[0] = 0.0f;
    nrm[1] = 0.0f;
    nrm[2] = one;
    dist = inv_MTH_fn_xMulAdd3(p_stNormal->xX, nrm[0], p_stNormal->xY, nrm[0], p_stNormal->xZ, one);
    bResult = 0;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope <= dist)
        bResult = 1;
    return bResult;
}

s32 MEC_fn_bCeilObstacle(DNM_stDynamics* p_stDynamic, DNM_stObstacle* p_stObstacle)
{
    f32 xLimit;
    f32 nrm[3];
    f32 xNegCos;
    f32 neg;
    MTH3D_tdstVector_* pNorm;

    xLimit = 1.0f;
    nrm[0] = 0.0f;
    nrm[1] = 0.0f;
    nrm[2] = xLimit;
    pNorm = &p_stObstacle->m_stNorm;
    xNegCos = inv_MTH_fn_xMulAdd3(p_stObstacle->m_stNorm.xX, nrm[0], pNorm->xY, nrm[0], pNorm->xZ,
                                  xLimit);
    neg = inv_MTH_fn_xNegateTied(p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope);
    if (xNegCos < neg) {
        return 1;
    }
    return 0;
}

s32 MEC_fn_bCanCrashActor(DNM_stMecObstacle* p_stCollideObstacle)
{
    void* startNode = p_stCollideObstacle->contact.mp_stSupObj;
    s32 result = 0;

    if (startNode != 0) {
        result = (s32) MEC_fn_hGetFatherActor(startNode);
        result = fn_ulStandardGameGetCustomBitsSO((void*) result);
        result = result >> 6;
        result = (result ^ 1) & 1;
    }
    return result;
}

s32 MEC_fn_bDynamicsNeedCollide(DNM_stDynamics* p_stDynamic)
{
    return inv_POS_fn_bIsEqualMatrixExact(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                                          &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix) == 0;
}

f32 MEC_fn_xGetSlide(DNM_stDynamics* p_stDynamic, DNM_stMecObstacle* p_stObstacle)
{
    s32 flags;
    f32 xSlide;

    if (p_stObstacle == 0) {
        xSlide = 0.0f;
    } else {
        flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
        xSlide = 0.0f;
        flags &= 0x80;
        if (flags != 0) {
            xSlide = p_stDynamic->m_tdstDynamicsBaseBlock.m_xSlide;
        }
    }
    return xSlide;
}

f32 MEC_fn_xGetRebound(DNM_stDynamics* p_stDynamic, DNM_stMecObstacle* p_stObstacle)
{
    s32 flags;
    f32 xRebound;

    if (p_stObstacle == 0) {
        xRebound = 0.0f;
    } else {
        flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
        xRebound = 0.0f;
        flags &= 0x80;
        if (flags != 0) {
            xRebound = p_stDynamic->m_tdstDynamicsBaseBlock.m_xRebound;
        }
    }
    return xRebound;
}

void MEC_fn_vComputeCollisionTranslation(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, const MTH3D_tdstVector_* p_stMove)
{
    MTH3D_tdstVector_ scratch;
    s32 tag;
    f32 coefficient;
    f32 squared;
    f32 length;

    if (lObstacle <= 0) {
        return;
    }
    do {
        tag = p_stObstacle->m_xCollidedEntity;
        if ((tag ^ 0x63) != 0) {
            coefficient = p_stObstacle->contact.m_xRate;
            if (1.0f < coefficient) {
                squared = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stObstacle->m_stZoneMove.xX, p_stObstacle->m_stZoneMove.xY,
                                                      p_stObstacle->m_stZoneMove.xZ);
                p_stObstacle->contact.m_xRate = 1.0f;
                length = MTH_fn_xSqrtOpt2(squared);
                if (length != 0.0f) {
                    f32 inverse = 1.0f / length;
                    MTH3D_M_vKTimesXYZ_inv(&scratch, inverse, &p_stObstacle->m_stZoneMove);
                } else {
                    scratch = p_stObstacle->m_stZoneMove;
                }
                MTH3D_M_vSetXYZ_inv(
                    &scratch,
                    p_stObstacle->m_xZoneRadius * scratch.xX + p_stObstacle->m_stZonePosition.xX,
                    p_stObstacle->m_xZoneRadius * scratch.xY + p_stObstacle->m_stZonePosition.xY,
                    p_stObstacle->m_xZoneRadius * scratch.xZ + p_stObstacle->m_stZonePosition.xZ);
                p_stObstacle->contact.m_stContact = scratch;
            } else if (0.0f <= coefficient) {
                f32 weight = 1.0f - coefficient;
                MTH3D_M_vSetXYZ_inv(&p_stObstacle->m_stTranslation, weight * p_stMove->xX,
                                    weight * p_stMove->xY, weight * p_stMove->xZ);
            }
        }
        p_stObstacle = p_stObstacle + 1;
    } while (--lObstacle != 0);
}

static inline u8 inv_MEC_fn_bIsNullVector(MTH3D_tdstVector_* vector)
{
    u8 result = 0;

    if (inv_MTH_fn_xAbsoluteTied(vector->xX - 0.0f) < D_00392A80 &&
        inv_MTH_fn_xAbsoluteTied(vector->xY - 0.0f) < D_00392A80 &&
        inv_MTH_fn_xAbsoluteTied(vector->xZ - 0.0f) < D_00392A80) {
        result = 1;
    }
    return result;
}

void MEC_fn_vDeleteWallEdge(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic)
{
    DNM_stMecObstacle* p_stTmpObstacle1;
    DNM_stMecObstacle* p_stTmpObstacle;
    MTH3D_tdstVector_ delta;
    s32 lCounter1;
    s32 lCounter;
    s32 keep;
    s32 tag;

    keep = 1;
    for (lCounter1 = 0; lCounter1 < lObstacle; lCounter1++) {
        p_stTmpObstacle1 = &p_stObstacle[lCounter1];
        tag = p_stTmpObstacle1->m_xCollidedEntity;
        if ((tag ^ 0x63) == 0) {
            continue;
        }
        if ((MEC_fn_bWallNormal(p_stDynamic, &p_stTmpObstacle1->contact.m_stNorm) & 0xFF) == 0) {
            continue;
        }
        if ((s32) p_stTmpObstacle1->m_xCollidedEntity != 8) {
            continue;
        }
        if (((p_stTmpObstacle1->m_eType >> 2) & 1) != 0) {
            continue;
        }
        for (lCounter = 0; lCounter < lObstacle; lCounter++) {
            if (lCounter != lCounter1) {
                p_stTmpObstacle = &p_stObstacle[lCounter];
                tag = p_stTmpObstacle->m_xCollidedEntity;
                if (((tag ^ 0x63) != 0) && (u16) (p_stTmpObstacle->m_xCollidedEntity - 7) < 2 &&
                    (MEC_fn_bWallNormal(p_stDynamic, &p_stTmpObstacle->contact.m_stNorm) & 0xFF) != 0) {
                    MTH3D_M_vSubXYZ_inv(&delta, &p_stTmpObstacle1->contact.m_stContact,
                                        &p_stTmpObstacle->contact.m_stContact);
                    if (inv_MEC_fn_bIsNullVector(&delta)) {
                        keep = 0;
                    }
                }
            }
        }
        if (keep != 0) {
            p_stTmpObstacle1->m_xCollidedEntity = 0x63;
        }
    }
}

void MEC_fn_vDeleteUnderGroundWall(s32 lObstacle, void* p_stObstacle, DNM_stDynamics* p_stDynamic,
                                   DNM_stMecObstacle* p_stGroundObstacle)
{
    typedef struct {
        u8 pad00[0x2E];
        s16 tag;
    } UnderGroundWallTagView;

    typedef struct {
        u8 pad00[0xC];
        f32 x;
        f32 y;
        f32 z;
        u8 pad18[0x12];
        s16 tag;
    } UnderGroundWallBiasedContactView;

    typedef struct {
        f32 x;
        f32 y;
        f32 z;
    } UnderGroundWallBasisView;

    s32 n;
    s32 flag;
    s32 bWall;
    s16 s;
    f32 hi;
    f32 lo;
    f32 xDeltaX;
    f32 xDeltaY;
    f32 xDeltaZ;
    f32 xAxisX;
    f32 xAxisY;
    f32 xAxisZ;
    f32 xDot;
    MTH3D_tdstVector_ vr;
    MTH3D_tdstVector_ stDelta;
    u8* v;
    u8* p;
    u8* q;

    if (p_stGroundObstacle == 0) {
        return;
    }
    vr = ((DNM_stMecObstacle*) p_stGroundObstacle)->contact.m_stContact;
    v = (u8*) p_stGroundObstacle + 4;

    if (lObstacle <= 0) {
        return;
    }
    hi = D_00392A84;
    lo = D_00392A88;
    flag = 99;
    q = (u8*) p_stObstacle + 4;
    p = p_stObstacle;
    n = lObstacle;
    do {
        s = ((UnderGroundWallTagView*) p)->tag;
        s = s ^ 99;
        if (s != 0) {
            bWall = MEC_fn_bWallNormal(p_stDynamic, (MTH3D_tdstVector_*) q);
            if ((bWall & 0xFF) != 0) {
                xDeltaX = ((UnderGroundWallBiasedContactView*) q)->x - vr.xX;
                xDeltaY = ((UnderGroundWallBiasedContactView*) q)->y - vr.xY;
                xDeltaZ = ((UnderGroundWallBiasedContactView*) q)->z - vr.xZ;
                s = ((UnderGroundWallBiasedContactView*) q)->tag;
                s = s ^ 8;
                xAxisX = ((UnderGroundWallBasisView*) v)->x;
                xAxisY = ((UnderGroundWallBasisView*) v)->y;
                xAxisZ = ((UnderGroundWallBasisView*) v)->z;
                xDot = inv_MTH_fn_xMulAdd3(xDeltaX, xAxisX, xDeltaY, xAxisY, xDeltaZ, xAxisZ);
                MTH3D_M_vSetXYZ_inv(&stDelta, xDeltaX, xDeltaY, xDeltaZ);
                if (s == 0) {
                    if (__builtin_fabsf(xDot) < hi) {
                        ((UnderGroundWallBiasedContactView*) q)->tag = flag;
                    }
                }
                if (xDot < lo) {
                    ((UnderGroundWallBiasedContactView*) q)->tag = flag;
                }
            }
        }
        q += 0x68;
        n -= 1;
        p += 0x68;
    } while (n != 0);
}

DNM_stMecObstacle* MEC_fn_pSelectGround(DNM_stDynamics* p_stDynamic, s32 lObstacle,
                                        DNM_stMecObstacle* p_stObstacle, MTH3D_tdstVector_* p_stTangentMove,
                                        u8* bFirstGround, DNM_stMecObstacle** step, u8* hasStep)
{
    MTH3D_tdstVector_ delta;
    DNM_stMecObstacle* ground;
    DNM_stMecObstacle* first;
    DNM_stMecObstacle* raised;
    DNM_stMecObstacle* candidate;
    DNM_stMecObstacle* other;
    f32 best;
    f32 difference;
    f32 dz;
    s32 i;
    s32 j;
    s32 found;
    s32 below;
    s32 same;
    s32 tag;
    s32 candidateTag;
    s32 otherTag;

    *bFirstGround = 1;
    *step = 0;
    *hasStep = 0;
    ground = 0;
    first = 0;
    best = flt_3934BC[0];
    difference = 0.0f;
    raised = 0;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10) {
        for (i = 0; i < lObstacle; i++) {
            tag = p_stObstacle[i].m_xCollidedEntity;
            if ((tag ^ 0x63) != 0) {
                below = p_stObstacle[i].contact.m_xRate < 0.0f;
                if (!below && (u8) MEC_fn_bGroundNormal(p_stDynamic, &p_stObstacle[i].contact.m_stNorm)) {
                    if (best < p_stObstacle[i].m_stTranslation.xZ) {
                        best = p_stObstacle[i].m_stTranslation.xZ;
                        ground = &p_stObstacle[i];
                    }
                }
            }
        }
        return ground;
    }

    best = 0.0f;
    for (i = 0; i < lObstacle; i++) {
        candidate = &p_stObstacle[i];
        tag = candidate->m_xCollidedEntity;
        if ((tag ^ 0x63) != 0 &&
            (u8) MEC_fn_bGroundNormal(p_stDynamic, &p_stObstacle[i].contact.m_stNorm)) {
            difference = inv_MTH_fn_xAbsoluteTied(p_stObstacle[i].contact.m_xRate - 0.5f);
            if (ground == 0) {
                ground = candidate;
                best = difference;
                first = ground;
            } else if (((u32) ((u16) p_stObstacle[i].m_xCollidedEntity - 7) >= 2 || ((p_stObstacle[i].m_eType >> 2) & 1)) &&
                       difference < best) {
                ground = candidate;
                best = difference;
            }
        }
    }

    if (ground == 0)
        return 0;

    best = 0.0f;
    for (i = 0; i < lObstacle; i++) {
        candidate = &p_stObstacle[i];
        candidateTag = candidate->m_xCollidedEntity;
        if ((candidateTag ^ 0x63) != 0 &&
            (u8) MEC_fn_bGroundNormal(p_stDynamic, &candidate->contact.m_stNorm)) {
            MTH3D_M_vSetXYZ_inv(
                &delta, candidate->contact.m_stContact.xX - ground->contact.m_stContact.xX,
                candidate->contact.m_stContact.xY - ground->contact.m_stContact.xY,
                dz = candidate->contact.m_stContact.xZ - ground->contact.m_stContact.xZ);
            if (best < dz) {
                if ((u32) ((u16) candidate->m_xCollidedEntity - 7) < 2 || ((candidate->m_eType >> 2) & 1)) {
                    found = 0;
                    for (j = 0; !found && j < lObstacle; j++) {
                        other = &p_stObstacle[j];
                        otherTag = other->m_xCollidedEntity;
                        if ((otherTag ^ 0x63) != 0 && dz <= 0.45f) {
                            if (((u8) MEC_fn_bWallNormal(p_stDynamic, &other->contact.m_stNorm) &&
                                 ((tag = other->m_xCollidedEntity) ^ 8) == 0 &&
                                 !(below = candidate->contact.m_xRate < 0.0f, below)) ||
                                ((u8) MEC_fn_bGroundNormal(p_stDynamic, &other->contact.m_stNorm) &&
                                 j != i)) {
                                MTH3D_M_vSubXYZ_inv(&delta, &candidate->contact.m_stContact,
                                                    &other->contact.m_stContact);
                                same = 0;
                                if (inv_MTH_fn_xAbsoluteTied(delta.xX - 0.0f) < 0.001f &&
                                    inv_MTH_fn_xAbsoluteTied(delta.xY - 0.0f) < 0.001f &&
                                    inv_MTH_fn_xAbsoluteTied(delta.xZ - 0.0f) < 0.001f)
                                    same = 1;
                                if (same) {
                                    found = 1;
                                    best = dz;
                                    *step = candidate;
                                    *hasStep = 1;
                                }
                            }
                        }
                    }
                } else {
                    raised = candidate;
                    best = dz;
                    *hasStep = 1;
                }
            }
        }
    }

    *bFirstGround = (ground == first);
    return raised ? raised : ground;
}

void MEC_fn_vComputeSpeed(DNM_stDynamics* p_stDynamic, f32 xDT)
{
    MTH3D_tdstVector_ velocity;
    f32* nextPosition;
    f32* currentPosition;

    nextPosition = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3.xX;
    currentPosition = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3.xX;
    MTH3D_M_vSetXYZ_inv(&velocity, (nextPosition[0] - currentPosition[0]) / xDT,
                        (nextPosition[1] - currentPosition[1]) / xDT,
                        (nextPosition[2] - currentPosition[2]) / xDT);
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed = velocity;
}

void MEC_fn_vInitMatrix(DNM_stDynamics* p_stDynamic)
{
    POS_tdstMatrix_* p_stCurrentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    POS_tdstMatrix_* p_stPreviousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) != 0) {
        POS_tdstMatrix_* p_stAbsolutePreviousMatrix = &p_stDynamic->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix;

        p_stDynamic->m_tdstDynamicsComplexBlock.m_stPrevPreviousMatrix = *p_stPreviousMatrix;
        *p_stAbsolutePreviousMatrix = *p_stCurrentMatrix;
    }

    *p_stPreviousMatrix = *p_stCurrentMatrix;
}

void MEC_fn_vCorrectMatrix(DNM_stDynamics* p_stDynamic, void* hMatrix)
{
    POS_fn_vNormalizeMatrix(hMatrix);
    POS_fn_vSetScaleMatrix(hMatrix, &p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale);
}

void MEC_fn_vTranslateMatrixOn(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stMatrixTranslation)
{
    POS_tdstMatrix_* hPreviousMatrix;
    POS_tdstMatrix_* hCurrentMatrix;
    MTH4D_tdstVector_* p_stPrevTrans;
    MTH4D_tdstVector_* p_stCurrTrans;
    MTH3D_tdstVector_ stFraction;
    s32 lX;
    s32 lY;
    s32 lZ;

    hPreviousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    hCurrentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    p_stPrevTrans = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    p_stCurrTrans = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;

    lX = inv_MTH_fn_lToInteger(p_stPrevTrans->xX);
    lY = inv_MTH_fn_lToInteger(p_stPrevTrans->xY);
    lZ = inv_MTH_fn_lToInteger(p_stPrevTrans->xZ);
    MTH3D_M_vSetXYZ_inv(p_stMatrixTranslation, (f32) lX, (f32) lY, (f32) lZ);

    MTH3D_M_vSubXYZ_inv(&stFraction, p_stCurrTrans, p_stMatrixTranslation);
    if (hCurrentMatrix->ucType == MAT_eTransform_Identity) {
        hCurrentMatrix->ucType = MAT_eTransform_Translation;
    }
    *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix) = stFraction;
    p_stCurrTrans->xW = 1.0f;

    MTH3D_M_vSubXYZ_inv(&stFraction, p_stPrevTrans, p_stMatrixTranslation);
    if (hPreviousMatrix->ucType == MAT_eTransform_Identity) {
        hPreviousMatrix->ucType = MAT_eTransform_Translation;
    }
    *inv_POS_fn_p_stPosMatrixTranslation(hPreviousMatrix) = stFraction;
    p_stPrevTrans->xW = 1.0f;
}

void MEC_fn_vTranslateMatrixOff(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stTranslation)
{
    inv_POS_fn_vTranslate(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix, p_stTranslation);
}

void MEC_fn_vAdjustScaleVector(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stVector)
{
    MTH3D_M_vSetXYZ_inv(p_stVector,
                        p_stVector->xX * p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xX,
                        p_stVector->xY * p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xY,
                        p_stVector->xZ * p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xZ);
}

void MEC_fn_vAdjustSpeed(DNM_stDynamics* p_stDynamic, f32 xDT)
{
    POS_tdstMatrix_* s = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    f32* dp = &s->stMatrix.stCol_3.xX;
    f32* rp = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3.xX;
    MTH3D_tdstVector_ dir;
    MTH3D_tdstVector_ stNewPosition;
    f32 zero;
    f32 len;
    f32 reciprocal;
    f32 xFactor;
    s32 small;

    s64 zeroLen;

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&dir, dp[0] - rp[0], dp[1] - rp[1], dp[2] - rp[2]);
    small = 0;
    if (inv_MTH_fn_xAbsoluteTied(dir.xX - zero) < 0.001f) {
        if (inv_MTH_fn_xAbsoluteTied(dir.xY - zero) < 0.001f) {
            if (inv_MTH_fn_xAbsoluteTied(dir.xZ - zero) < 0.001f) {
                small = 1;
            }
        }
    }
    zeroLen = small;
    if (zeroLen == 0) {
        len = MTH_fn_xSqrAddSqrAddSqrOpt2(dir.xX, dir.xY, dir.xZ);
        len = MTH_fn_xSqrtOpt2(len);
        if (len != 0.0f) {
            reciprocal = 1.0f / len;
            inv_MTH3D_fn_vKTimesXYZ(&dir, reciprocal, &dir);
        }
    } else {
        dir.xZ = dir.xY = dir.xX = 0.0f;
    }
    xFactor = inv_MTH_fn_xMulAdd3Grouped(dir.xX, dir.xY, dir.xZ,
                         p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX,
                         p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY,
                         p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ);
    xFactor = xFactor * xDT;
    inv_MTH3D_fn_vKTimesXYZ(&dir, xFactor, &dir);
    MTH3D_M_vSetXYZ_inv(&stNewPosition, rp[0] + dir.xX, rp[1] + dir.xY, rp[2] + dir.xZ);
    if (s->ucType == MAT_eTransform_Identity) {
        s->ucType = MAT_eTransform_Translation;
    }
    *(MTH3D_tdstVector_*) &s->stMatrix.stCol_3 = stNewPosition;
    s->stMatrix.stCol_3.xW = 1.0f;
}

f32 MEC_fn_vRoundReal(f32 a, f32 b)
{
    s32 n;
    a = a / b;
    n = inv_MTH_fn_lToInteger(a);
    return (f32) n * b;
}

void MEC_fn_vRoundVector(MTH3D_tdstVector_* p_stVector, f32 xEpsilon)
{
    p_stVector->xX = MEC_fn_vRoundReal(p_stVector->xX, xEpsilon);
    p_stVector->xY = MEC_fn_vRoundReal(p_stVector->xY, xEpsilon);
    p_stVector->xZ = MEC_fn_vRoundReal(p_stVector->xZ, xEpsilon);
}

f32 MEC_fn_xConvertScale(f32 xValue, u8 bAdjust)
{
    f32 one;
    f32 result;

    one = 1.0f;
    if (one <= xValue) {
        result = xValue;
    } else {
        f32 zero = 0.0f;
        if (zero < xValue) {
            result = one - xValue;
            result = MTH_fn_xSqrOpt1(result);
            result = MTH_fn_xSqrOpt1(result);
            result = one - result;
        } else {
            result = zero;
        }
    }
#ifdef PC_PORT
    if (bAdjust != 0) {
        f32 clockScale = inv_fn_xGetEngineClockScale();
        f32 ticks = inv_GAM_fn_xGetEngineClockTicks();
        f32 steps = (f32) fn_lGetTemporalCurveType();

        if (ticks != steps) {
            clockScale = clockScale * ticks / steps;
        }
        if (0.001f < __builtin_fabsf(clockScale - 0.016666668f)) {
            result = powf(result, clockScale * 60.0f);
        }
    }
#else
    if (bAdjust != 0) {
        f32 clockScale = inv_fn_xGetEngineClockScale();
        if (0.001f < __builtin_fabsf(clockScale - 0.016666668f)) {
            clockScale = inv_fn_xGetEngineClockScale();
            result = powf(result, clockScale * 60.0f);
        }
    }
#endif
    return result;
}

void MEC_fn_vDeleteOnGroundBackFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic,
                                    MTH3D_tdstVector_* p_stVector, DNM_stMecObstacle* p_stGroundObstacle)
{
    DNM_stMecObstacle* p_stCurrentObstacle;
    f32* ref;
    s32 lCounter;
    s32 tag;
    s32 bWall;
    f32 dot;
    f32 xDelta[3];
    f32 k[3];

    for (lCounter = 0; lCounter < lObstacle; lCounter++) {
        p_stCurrentObstacle = &p_stObstacle[lCounter];
        tag = p_stCurrentObstacle->m_xCollidedEntity;
        if ((tag ^ 0x63) != 0) {
            bWall = MEC_fn_bWallNormal(p_stDynamic, &p_stCurrentObstacle->contact.m_stNorm);
            if (((u8) bWall) != 0) {
                dot = inv_MTH_fn_xMulAdd3(p_stCurrentObstacle->contact.m_stNorm.xX, p_stVector->xX,
                                          p_stCurrentObstacle->contact.m_stNorm.xY, p_stVector->xY,
                                          p_stCurrentObstacle->contact.m_stNorm.xZ, p_stVector->xZ);
                if (dot <= 0.0f) {
                    ref = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3.xX;
                    xDelta[0] = p_stCurrentObstacle->contact.m_stContact.xX - ref[0];
                    xDelta[1] = p_stCurrentObstacle->contact.m_stContact.xY - ref[1];
                    xDelta[2] = p_stCurrentObstacle->contact.m_stContact.xZ - ref[2];
                    k[0] =
                        p_stCurrentObstacle->m_xZoneRadius * p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stGroundNormal.xX;
                    k[1] =
                        p_stCurrentObstacle->m_xZoneRadius * p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stGroundNormal.xY;
                    k[2] =
                        p_stCurrentObstacle->m_xZoneRadius * p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stGroundNormal.xZ;
                    xDelta[0] = xDelta[0] - k[0];
                    xDelta[1] = xDelta[1] - k[1];
                    xDelta[2] = xDelta[2] - k[2];
                    dot = inv_MTH_fn_xMulAdd3Grouped(p_stCurrentObstacle->contact.m_stNorm.xX,
                                           p_stCurrentObstacle->contact.m_stNorm.xY,
                                           p_stCurrentObstacle->contact.m_stNorm.xZ, xDelta[0], xDelta[1],
                                           xDelta[2]);
                    if (dot > 0.0f) {
                        p_stCurrentObstacle->m_xCollidedEntity = 0x63;
                    }
                } else {
                    p_stCurrentObstacle->m_xCollidedEntity = 0x63;
                }
            } else {
                bWall = MEC_fn_bGroundNormal(p_stDynamic, &p_stCurrentObstacle->contact.m_stNorm);
                if (((u8) bWall) != 0) {
                    continue;
                }
                if (p_stGroundObstacle == 0) {
                    continue;
                }
                xDelta[0] = p_stCurrentObstacle->contact.m_stContact.xX - p_stGroundObstacle->contact.m_stContact.xX;
                xDelta[1] = p_stCurrentObstacle->contact.m_stContact.xY - p_stGroundObstacle->contact.m_stContact.xY;
                xDelta[2] = p_stCurrentObstacle->contact.m_stContact.xZ - p_stGroundObstacle->contact.m_stContact.xZ;
                dot = inv_MTH_fn_xMulAdd3Grouped(p_stGroundObstacle->contact.m_stNorm.xX,
                                       p_stGroundObstacle->contact.m_stNorm.xY,
                                       p_stGroundObstacle->contact.m_stNorm.xZ, xDelta[0], xDelta[1], xDelta[2]);
                if (dot < 0.9f) {
                    p_stCurrentObstacle->m_xCollidedEntity = 0x63;
                }
            }
        }
    }
}

static inline f32 inv_MEC_fn_xObstacleDot(DNM_stMecObstacle* obstacle, MTH3D_tdstVector_* vector)
{
    f32 dot;

    dot = inv_MTH_fn_xMulAdd3(obstacle->contact.m_stNorm.xX, vector->xX,
                              obstacle->contact.m_stNorm.xY, vector->xY,
                              obstacle->contact.m_stNorm.xZ, vector->xZ);
    return dot;
}

void MEC_fn_vDeleteGiBackFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic,
                              MTH3D_tdstVector_* p_stVector)
{
    MTH3D_tdstVector_ up;
    f32 zero = 0.0f;
    DNM_stMecObstacle* p_stCurrentObstacle;
    u64 flags;
    f32 dot;
    s32 lCounter;
    s32 stride;

    lCounter = 0;
    up.xX = zero;
    up.xY = zero;
    up.xZ = 1.0f;
    if (lObstacle <= 0) {
        return;
    }
    do {
        stride = sizeof(DNM_stMecObstacle);
        p_stCurrentObstacle = (DNM_stMecObstacle*) ((u8*) p_stObstacle + lCounter * stride);
        if ((inv_MEC_fn_lClimbObstacleMarker(p_stCurrentObstacle) ^ 0x63) == 0) {
            lCounter++;
            continue;
        }
        flags = p_stCurrentObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;
        if (((flags >> 5) & 1) == 1 && !(flags & 0x8000)) {
            dot = inv_MEC_fn_xObstacleDot(p_stCurrentObstacle, &up);
            if (0.0f < dot) {
                p_stCurrentObstacle->m_xCollidedEntity = 0x63;
            }
            lCounter++;
            continue;
        }
        if (MEC_fn_bCeilObstacle(p_stDynamic, &p_stCurrentObstacle->contact)) {
            p_stCurrentObstacle->m_xCollidedEntity = 0x63;
            lCounter++;
            continue;
        }
        if (!(u8) MEC_fn_bWallNormal(p_stDynamic, &p_stCurrentObstacle->contact.m_stNorm)) {
            lCounter++;
            continue;
        }
        stride = (s32) p_stCurrentObstacle->contact.hCollidedMaterial;
        dot = inv_MEC_fn_xObstacleDot(p_stCurrentObstacle, p_stVector);
        if (0.0f < dot) {
            p_stCurrentObstacle->m_xCollidedEntity = 0x63;
        }
        lCounter++;
    } while (lCounter < lObstacle);
}

__asm__(
    ".section .lit4, \"a\"\n.balign 4\n.word 0xBF34FDF4\n.word 0xBF733333\n.word 0x3A83126F\n.word 0x3A83126F\n.word 0x3F34FDF4\n\n.text\n");

void MEC_fn_vDeleteClimbBackFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic,
                                 MTH3D_tdstVector_* p_stVector)
{
    typedef struct {
        f32 normal_threshold;
        f32 other_threshold;
    } ClimbThresholdPair;

    extern ClimbThresholdPair D_00392AA4;

    f32 vector[3];
    f32 threshold;
    s32 i;
    DNM_stMecObstacle* it;

    i = 0;
    if (lObstacle > 0) {
        threshold = D_00392AA4.normal_threshold;
        it = p_stObstacle;
        do {
            if ((inv_MEC_fn_lClimbObstacleMarker(it) ^ 0x63) != 0) {
                u16 flags;
                f32 dot;
                POS_fn_vGetYAxisVector(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix, vector);
                flags = it->contact.hCollidedMaterial->collideMaterial.xIdentifier;
                dot = inv_MTH_fn_xMulAdd3Grouped(it->contact.m_stNorm.xX, it->contact.m_stNorm.xY,
                                       it->contact.m_stNorm.xZ, vector[0], vector[1], vector[2]);
                if (inv_MEC_fn_uwClimbSurfaceBit(flags) && !(flags & 0x8000)) {
                    if (dot < threshold)
                        it->m_xCollidedEntity = 0x63;
                    i += 1;
                    it++;
                    continue;
                }
                if (dot < D_00392AA4.other_threshold) {
                    it->m_xCollidedEntity = 0x63;
                } else {
                    dot = inv_MTH_fn_xMulAdd3Grouped(
                        it->contact.m_stNorm.xX, it->contact.m_stNorm.xY, it->contact.m_stNorm.xZ,
                        p_stVector->xX, p_stVector->xY, p_stVector->xZ);
                    if (D_00392AAC < dot)
                        it->m_xCollidedEntity = 0x63;
                }
            }
            i += 1;
            it++;
        } while (i < lObstacle);
    }
}

void MEC_fn_vDeleteBaseBackFace(s32 lObstacle, DNM_stMecObstacle* base, DNM_stDynamics* p_stDynamic)
{
    typedef struct {
        f32 x, y, z;
    } Vec;

    Vec v;
    f32 zero;
    s32 i = 0, stride;
    if (lObstacle <= 0)
        return;
    do {
        zero = inv_MTH_fn_xZero();
    } while (0);
    do {
        DNM_stMecObstacle* p;
        s32 tag;
        f32 x, xx, y, z, len, squared, dot, nx, ny, nz, vx, vy, vz;
        Vec* src;
        stride = sizeof(DNM_stMecObstacle);
        p = (DNM_stMecObstacle*) ((u8*) base + i * stride);
        tag = p->m_xCollidedEntity;
        tag ^= 99;
        if (tag != 0) {
            if (!(inv_MEC_fn_lBothNodesFlagged(p))) {
                x = p->m_stZoneMove.xX;
                src = (Vec*) &p->m_stZoneMove;
                z = inv_MTH_fn_xZero();
                tag = 0;
                len = D_00392AB0;
                do {
                    xx = inv_MEC_fn_xCopyFloatBits(x);
                } while (0);
                if (inv_MTH_fn_xAbsoluteTied(x - z) < len &&
                    inv_MTH_fn_xAbsoluteTied(src->y - z) < len &&
                    inv_MTH_fn_xAbsoluteTied(src->z - z) < len)
                    tag = 1;
                if (!tag) {
                    y = p->m_stZoneMove.xY;
                    z = p->m_stZoneMove.xZ;
                    squared = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
                    len = MTH_fn_xSqrtOpt2(squared);
                    if (len != zero) {
                        squared = 1.0f;
                        squared /= len;
                        v.x = squared * xx;
                        v.y = squared * y;
                        v.z = squared * z;
                    } else
                        v = *(Vec*) &p->m_stZoneMove;
                } else {
                    v.x = zero;
                    v.y = zero;
                    v.z = zero;
                }
                nx = p->contact.m_stNorm.xX;
                vx = v.x;
                ny = p->contact.m_stNorm.xY;
                vy = v.y;
                nz = p->contact.m_stNorm.xZ;
                vz = v.z;
                vx = inv_MTH_fn_xMulAdd3(nx, vx, ny, vy, nz, vz);
                if (D_00392AB4 < vx)
                    p->m_xCollidedEntity = 99;
                else {
                    stride = 0x400000;
                    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & stride) == 0) {
                        {
                            f32 value = p->m_xZoneRadius;
                            f32 zz;
                            zz = 0.0f;
                            if (value == zz)
                                p->m_xCollidedEntity = 99;
                            else {
                                f32 dx, dy, dz;
                                dx = p->contact.m_stContact.xX - p->m_stZonePosition.xX;
                                dy = p->contact.m_stContact.xY - p->m_stZonePosition.xY;
                                dz = p->contact.m_stContact.xZ - p->m_stZonePosition.xZ;
                                dx += x;
                                dy += p->m_stZoneMove.xY;
                                dz += p->m_stZoneMove.xZ;
                                v.x = dx;
                                v.y = dy;
                                v.z = dz;
                                dot = inv_MTH_fn_xMulAdd3(nx, dx, ny, dy, nz, dz);
                                if (dot > zz)
                                    p->m_xCollidedEntity = 99;
                            }
                        }
                    }
                }
            }
        }
        i++;
    } while (i < lObstacle);
}

void MEC_fn_vDeleteNoCollidedGroundFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                        DNM_stDynamics* p_stDynamic)
{
    s32 i;
    s32 contactType;
    u64 flags;

    for (i = 0; i < lObstacle; i++) {
        contactType = p_stObstacle[i].m_xCollidedEntity;
        if ((contactType ^ 0x63) != 0 &&
            ((u8) MEC_fn_bGroundNormal(p_stDynamic, &p_stObstacle[i].contact.m_stNorm)) != 0) {
            flags = p_stObstacle[i].contact.hMyMaterial->collideMaterial.xIdentifier;
            if (((flags >> 4) & 1) == 1) {
                p_stObstacle[i].m_xCollidedEntity = 0x63;
            }
        }
    }
}

void MEC_fn_vDeleteHangingObstacle(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic)
{
    f32 height;
    f32 lowestZ;
    s32 i;
    DNM_stMecObstacle* contact;
    DNM_stMecObstacle* lowest;
    MTH3D_tdstVector_ displacement;
    s32 type;
    f32 projection;

    lowest = NULL;
    lowestZ = flt_3934C0[0];
    for (i = 0; i < lObstacle; ++i) {
        contact = &p_stObstacle[i];
        type = contact->m_xCollidedEntity;
        if (((type ^ 0x63) != 0) &&
            (u8) MEC_fn_bGroundNormal(p_stDynamic, &contact->contact.m_stNorm) &&
            contact->contact.m_stContact.xZ < lowestZ) {
            lowest = contact;
            lowestZ = contact->contact.m_stContact.xZ;
        }
    }

    if (p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform != NULL)
        height =
            p_stDynamic->m_tdstDynamicsComplexBlock.m_xHangingLimit +
            p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform->gu_hGlobalMatrix->stMatrix.stCol_3.xZ;
    else
        height = p_stDynamic->m_tdstDynamicsComplexBlock.m_xHangingLimit;

    for (i = 0; i < lObstacle; ++i) {
        contact = &p_stObstacle[i];
        type = contact->m_xCollidedEntity;
        if ((type ^ 0x63) != 0) {
            if ((u8) MEC_fn_bGroundNormal(p_stDynamic, &contact->contact.m_stNorm) ||
                !(contact->contact.m_xRate <= 1.0f) || (u32) ((u16) contact->m_xCollidedEntity - 7) < 2) {
                contact->m_xCollidedEntity = 0x63;
            } else if (lowest != NULL) {
                MTH3D_M_vSubXYZ_inv(&displacement, &contact->contact.m_stContact,
                                    &lowest->contact.m_stContact);
                projection = inv_MTH_fn_xMulAdd3Grouped(lowest->contact.m_stNorm.xX,
                                              lowest->contact.m_stNorm.xY,
                                              lowest->contact.m_stNorm.xZ, displacement.xX,
                                              displacement.xY, displacement.xZ);
                if (projection < 0.005f)
                    contact->m_xCollidedEntity = 0x63;
            } else if (contact->contact.m_stContact.xZ <= height) {
                contact->m_xCollidedEntity = 0x63;
            }
        }
    }
}

typedef struct ObstacleTranslationWalk {
    MTH3D_tdstVector_ translation;
    u8 unk0C[0x5C];
} ObstacleTranslationWalk;

void MEC_fn_vDeleteNullTranslationObstacle(s32 lObstacle, DNM_stMecObstacle* a_stObstacle)
{
    ObstacleTranslationWalk* walk;
    s32 isNull;

    if (lObstacle <= 0)
        return;
    walk = (ObstacleTranslationWalk*) &a_stObstacle->m_stTranslation;
    do {
        isNull = ((s16*) &walk->translation)[-1];
        isNull ^= 99;
        if (isNull != 0) {
            isNull = 0;
            if (inv_MTH_fn_xAbsoluteTied(walk->translation.xX - 0.0f) < 0.001f &&
                inv_MTH_fn_xAbsoluteTied(walk->translation.xY - 0.0f) < 0.001f &&
                inv_MTH_fn_xAbsoluteTied(walk->translation.xZ - 0.0f) < 0.001f)
                isNull = 1;
        }
        if (isNull != 0)
            ((s16*) &walk->translation)[-1] = 99;
        walk++;
    } while (--lObstacle != 0);
}

void* MEC_fn_hGetFatherActor(HIE_tdstSuperObject_* hSupObj)
{
    while (*(s32*) hSupObj != 2) {
        hSupObj = hSupObj->hFatherSta;
        if (hSupObj == 0) {
            return 0;
        }
    }
    return hSupObj;
}

void MEC_fn_vSetInitialFrameTranslation(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stTranslation)
{
    inv_POS_fn_vTranslate(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix, p_stTranslation);
}

void MEC_fn_vComputeStream(MTH3D_tdstVector_* p_stStream, DNM_stDynamics* p_stDynamic, u8 bSlide, f32 xDT)
{
    f32 x, y, z;
    xDT *= p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamFactor;
    x = xDT * p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xX;
    p_stStream->xX = x;
    y = xDT * p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xY;
    p_stStream->xY = y;
    z = xDT * p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xZ;
    p_stStream->xZ = z;
    if (bSlide) {
        MTH3D_M_vSetXYZ_inv(p_stStream,
                            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide * x,
                            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide * y,
                            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide * z);
    }
}

void MEC_fn_vComputeInertiaTranslation(DNM_stDynamics* obj, f32 xDT)
{
    MTH3D_tdstVector_ translation;
    s32 flag;

    flag = (obj->m_tdstDynamicsAdvancedBlock.m_xStreamPrority != 1.0f);
    MEC_fn_vComputeStream(&translation, obj, flag, xDT);
    MTH3D_M_vAddXYZ_inv(&translation, &translation,
                        &obj->m_tdstDynamicsBaseBlock.m_stAddTranslation);
    obj->m_tdstDynamicsAdvancedBlock.m_stInertiaTranslation = translation;
}

int MEC_fn_bMainActor(HIE_tdstSuperObject_* hSupObj)
{
    return (unsigned int) fn_ulStandardGameGetCustomBitsSO(hSupObj) >> 31;
}

s32 MEC_fn_bNoMove(DNM_stDynamics* p_stDynamic)
{
    u32 flags;
    u32 flags2;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX != 0.0f)
        return 0;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY != 0.0f)
        return 0;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ != 0.0f)
        return 0;

    flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
    if ((flags & 4) != 0)
        return 0;
    if ((flags & 1) != 0)
        return 0;
    if ((flags & 0xE0000) != 0)
        return 0;
    if ((flags & 0x20000000) != 0)
        return 0;
    if ((flags & 0x700000) != 0)
        return 0;

    flags2 = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
    if ((flags2 & 2) != 0) {
        if ((flags & 0x400) != 0)
            return 0;
        if ((flags & 0x800) != 0)
            return 0;
        if ((flags & 0x3800000) != 0)
            return 0;
    }

    if (flags2 & 4) {
        if (p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform != 0)
            return 0;
    }
    return 1;
}

void MEC_fn_vSlowMode(DNM_stDynamics* p_stDynamic, s32 lMode)
{
    POS_tdstMatrix_* state1 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    POS_tdstMatrix_* state2 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH4D_tdstVector_* previousPos = &state1->stMatrix.stCol_3;
    MTH4D_tdstVector_* currentPos = &state2->stMatrix.stCol_3;
    f32 factor;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ pos;

    factor = 1.0f / (f32) lMode;

    MTH3D_M_vSetXYZ_inv(&delta, factor * (currentPos->xX - previousPos->xX),
                        factor * (currentPos->xY - previousPos->xY),
                        factor * (currentPos->xZ - previousPos->xZ));
    MTH3D_M_vAddXYZ_inv(&pos, previousPos, &delta);

    if (state2->ucType == MAT_eTransform_Identity)
        state2->ucType = MAT_eTransform_Translation;
    *(MTH3D_tdstVector_*) &state2->stMatrix.stCol_3 = pos;
    currentPos->xW = 1.0f;

    {
        MTH3D_tdstVector_* velocity = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
        inv_MTH3D_fn_vKTimesXYZ(velocity, factor, velocity);
    }
}

void MEC_fn_vNormalMode(DNM_stDynamics* p_stDynamic, s32 lMode)
{
    POS_tdstMatrix_* state1 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    POS_tdstMatrix_* state2 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH4D_tdstVector_* previousPos = &state1->stMatrix.stCol_3;
    MTH4D_tdstVector_* currentPos = &state2->stMatrix.stCol_3;
    f32 factor;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ pos;

    factor = (f32) lMode;

    MTH3D_M_vSetXYZ_inv(&delta, factor * (currentPos->xX - previousPos->xX),
                        factor * (currentPos->xY - previousPos->xY),
                        factor * (currentPos->xZ - previousPos->xZ));
    MTH3D_M_vSubXYZ_inv(&pos, currentPos, &delta);

    if (state1->ucType == MAT_eTransform_Identity)
        state1->ucType = MAT_eTransform_Translation;
    *(MTH3D_tdstVector_*) previousPos = pos;
    previousPos->xW = 1.0f;

    {
        MTH3D_tdstVector_* velocity = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
        inv_MTH3D_fn_vKTimesXYZ(velocity, factor, velocity);
    }
}

void func_0013B6E8(DNM_stDynamics* p_stDynamic)
{
    MTH3D_tdstVector_ delta;
    f32* next;
    f32* prev;
    POS_tdstMatrix_* previous;
    POS_tdstMatrix_* matrix;

    next = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3.xX;
    prev = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3.xX;
    previous = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    matrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH3D_M_vSetXYZ_inv(&delta, next[0] - prev[0], next[1] - prev[1], next[2] - prev[2]);
    if (previous->ucType == 1) {
        previous->ucType = 2;
    }
    MTH4D_fn_vCopy3DTo4DVector(prev, (MTH3D_tdstVector_*) next);
    inv_POS_fn_vTranslate(matrix, &delta);
}
