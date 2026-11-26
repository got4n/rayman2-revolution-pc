#include "common.h"

#include "GMT/GMT.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "GAM/GAM.h"

#define OBSTACLE_TYPE_NONE 0x63
#define OBSTACLE_TYPE_STEP 7
#define OBSTACLE_TYPE_STEP_EDGE 8
#define MATERIAL_WATER 0x4000
#define MATERIAL_CRASH 0x8000
#define MATERIAL_CRASH_MASK 0x1F88
#define VECTOR_EPSILON 0.001f

extern DNM_stMecObstacle inv_MEC_stReportWall;
extern DNM_stMecObstacle inv_MEC_stReportCharacter;
extern DNM_stMecObstacle inv_MEC_stReportWater;
extern DNM_stMecObstacle inv_MEC_stReportCeil;

void POS_fn_vGetYAxisVector(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* axis);

static inline f32 inv_MEC_fn_xCollisionDot(f32 ax, f32 bx, f32 ay, f32 by, f32 az, f32 bz)
{
    f32 dot;

    dot = inv_MTH_fn_xMulAdd3Grouped(ax, ay, az, bx, by, bz);
    return dot;
}

s32 MEC_fn_bCollision(s32* p_lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic,
                      HIE_tdstSuperObject_* p_stSupObj, POS_tdstMatrix_* start,
                      POS_tdstMatrix_* end, u8 bBackFace)
{
    DNM_stMecObstacle best;
    MTH3D_tdstVector_ move;
    MTH3D_tdstVector_ toContact;
    MTH3D_tdstVector_ axis;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* origin;
    MTH3D_tdstVector_* startTranslation;
    MTH3D_tdstVector_* endTranslation;
    s32 any;
    s32 reported;
    s32 found;
    s32 i;
    f32 bestDot;
    f32 dot;
    f32 zero;
    f32 slopeXY;

    bestDot = -1.0f;
    *p_lObstacle = 0;
    found = inv_COL_fn_lTestCharacterAgainstZdxList(p_stObstacle, p_stSupObj, start, end);
    any = 0;
    reported = 0;
    *p_lObstacle = found;

    if (!bBackFace) {
        MEC_fn_vDeleteBaseBackFace(*p_lObstacle, p_stObstacle, p_stDynamic);
    }
    MEC_fn_vDeleteNoCollidedGroundFace(*p_lObstacle, p_stObstacle, p_stDynamic);
    if (*p_lObstacle <= 0) {
        return 0;
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x400000) {
        MEC_fn_vDeleteHangingObstacle(*p_lObstacle, p_stObstacle, p_stDynamic);
    }

    endTranslation = inv_POS_fn_p_stPosMatrixTranslation(end);
    startTranslation = inv_POS_fn_p_stPosMatrixTranslation(start);
    move.xX = startTranslation->xX - endTranslation->xX;
    move.xY = startTranslation->xY - endTranslation->xY;
    i = 0;
    move.xZ = startTranslation->xZ - endTranslation->xZ;
    MEC_fn_vComputeCollisionTranslation(*p_lObstacle, p_stObstacle, &move);

    matrix = p_stSupObj->gu_hGlobalMatrix;
    POS_fn_vGetYAxisVector(matrix, &axis);

    if (*p_lObstacle > 0) {
        zero = 0.0f;
        slopeXY = 0.76604444f;
        origin = inv_POS_fn_p_stPosMatrixTranslation(matrix);

        for (; i < *p_lObstacle; i++) {
            DNM_stMecObstacle* obstacle = &p_stObstacle[i];
            u64 ownFlags;
            u64 otherFlags;
            s32 keep;

            {
                s32 type = obstacle->m_xCollidedEntity;
                if ((type ^ OBSTACLE_TYPE_NONE) != 0) {
                    ownFlags = obstacle->contact.hMyMaterial->collideMaterial.xIdentifier;
                    keep = 1;

                    if (((ownFlags >> 3) & 1) == 1) {
                        u64 flags = obstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;
                        if (!((flags >> 2) & 1)) {
                            do {
                                keep = 0;
                            } while (0);
                        } else {
                            f32 length;
                            f32 inverse;
                            f32 groundDot;
                            s32 nearZero;

                            keep = 0;
                            if ((u8) MEC_fn_bGroundNormal(p_stDynamic, &obstacle->contact.m_stNorm)) {
                                MTH3D_M_vSetXYZ_inv(
                                    &toContact,
                                    obstacle->contact.m_stContact.xX -
                                        inv_POS_fn_p_stPosMatrixTranslation(matrix)->xX,
                                    obstacle->contact.m_stContact.xY - origin->xY,
                                    obstacle->contact.m_stContact.xZ - origin->xZ);

                                nearZero = 0;
                                if (inv_MTH_fn_xAbsoluteTied(toContact.xX - 0.0f) <
                                        VECTOR_EPSILON &&
                                    inv_MTH_fn_xAbsoluteTied(toContact.xY - 0.0f) <
                                        VECTOR_EPSILON &&
                                    inv_MTH_fn_xAbsoluteTied(toContact.xZ - 0.0f) <
                                        VECTOR_EPSILON) {
                                    nearZero = 1;
                                }

                                if (!nearZero) {
                                    length = MTH_fn_xSqrAddSqrAddSqrOpt2(toContact.xX, toContact.xY,
                                                                         toContact.xZ);
                                    length = MTH_fn_xSqrtOpt2(length);
                                    if (length != zero) {
                                        inverse = 1.0f / length;
                                        inv_MTH3D_fn_vKTimesXYZ(&toContact, inverse, &toContact);
                                    }
                                }

                                groundDot = inv_MTH_fn_xMulAdd3Grouped(axis.xX, axis.xY, axis.xZ,
                                                             toContact.xX, toContact.xY,
                                                             toContact.xZ);
                                dot = __builtin_fabsf(groundDot);
                                if (bestDot < dot) {
                                    bestDot = dot;
                                    best = *obstacle;
                                }
                            }
                        }
                    } else {
                        otherFlags =
                            obstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;

                        if (otherFlags & MATERIAL_WATER) {
                            f32 normalZ;

                            lZWaterValidity = 0;
                            g_xZWater = obstacle->contact.m_stContact.xZ;

                            if (!(ownFlags & MATERIAL_WATER) || reported) {
                                keep = 0;
                            } else {
                                normalZ = obstacle->m_stZoneMove.xZ;
                                dot = inv_MEC_fn_xCollisionDot(
                                    obstacle->contact.m_stNorm.xX, obstacle->m_stZoneMove.xX,
                                    obstacle->contact.m_stNorm.xY, obstacle->m_stZoneMove.xY,
                                    obstacle->contact.m_stNorm.xZ, obstacle->m_stZoneMove.xZ);

                                if (0.03f < inv_MTH_fn_xAbsolute(normalZ)) {
                                    if (dot < zero) {
                                        reported = 1;
                                        MEC_fn_vReportCollision(p_stDynamic, p_stSupObj, 1, obstacle,
                                                                0);
                                        keep = p_stDynamic->m_tdstDynamicsBaseBlock.collisionPolicy;
                                    } else {
                                        keep = 0;
                                    }
                                } else {
                                    MTH3D_M_vSetXYZ_inv(&toContact,
                                                        (obstacle->contact.m_stContact.xX -
                                                         obstacle->m_stZonePosition.xX) +
                                                            obstacle->m_stZoneMove.xX,
                                                        (obstacle->contact.m_stContact.xY -
                                                         obstacle->m_stZonePosition.xY) +
                                                            obstacle->m_stZoneMove.xY,
                                                        (obstacle->contact.m_stContact.xZ -
                                                         obstacle->m_stZonePosition.xZ) +
                                                            obstacle->m_stZoneMove.xZ);

                                    dot = inv_MTH_fn_xMulAdd3Grouped(obstacle->contact.m_stNorm.xX,
                                                           obstacle->contact.m_stNorm.xY,
                                                           obstacle->contact.m_stNorm.xZ,
                                                           toContact.xX, toContact.xY,
                                                           toContact.xZ);
                                    if (dot <= zero) {
                                        reported = 1;
                                        MEC_fn_vReportCollision(p_stDynamic, p_stSupObj, 1, obstacle,
                                                                0);
                                        keep = p_stDynamic->m_tdstDynamicsBaseBlock.collisionPolicy;
                                    } else {
                                        keep = 0;
                                    }
                                }

                                if (zero < obstacle->contact.m_stNorm.xZ) {
                                    keep = 0;
                                }
                            }
                        } else if ((otherFlags & 1) == 1) {
                            if (obstacle->m_xCollidedEntity == 3) {
                                MTH3D_tdstVector_* normal = &obstacle->contact.m_stNorm;
                                s32 nearZero;
                                f32 length;
                                f32 inverse;

                                if (MEC_fn_bGroundNormal(p_stDynamic, normal)) {
                                    *(s32*) &normal->xZ = 0;
                                    nearZero = 0;
                                    if (inv_MTH_fn_xAbsoluteTied(normal->xX - normal->xZ) <
                                            VECTOR_EPSILON &&
                                        __builtin_fabsf(normal->xY - normal->xZ) < VECTOR_EPSILON &&
                                        __builtin_fabsf(normal->xZ) < VECTOR_EPSILON) {
                                        nearZero = 1;
                                    }

                                    if (!nearZero) {
                                        length = MTH_fn_xSqrAddSqrAddSqrOpt2(normal->xX, normal->xY,
                                                                             normal->xZ);
                                        length = MTH_fn_xSqrtOpt2(length);
                                        if (length != zero) {
                                            inverse = 1.0f / length;
                                            inv_MTH3D_fn_vKTimesXYZ(normal, inverse, normal);
                                        }
                                        MTH3D_M_vSetXYZ_inv(normal, normal->xX * slopeXY,
                                                            normal->xY * slopeXY, 0.64278764f);
                                    } else {
                                        MTH3D_M_vSetXYZ_inv(normal, slopeXY, 0.0f, 0.64278764f);
                                    }
                                }
                            }
                        } else if (otherFlags & MATERIAL_CRASH) {
                            if (((otherFlags >> 6) & 1) == 1) {
                                keep =
                                    ((s32) p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags >> 6) & 1;
                            } else if (((otherFlags >> 5) & 1) == 1) {
                                keep =
                                    ((s32) p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags >> 4) & 1;
                            } else if (otherFlags & MATERIAL_CRASH_MASK) {
                                MEC_fn_vReportCollision(p_stDynamic, p_stSupObj, 1, obstacle, 0);
                                keep = 0;
                            } else {
                                keep = 0;
                            }
                        }
                    }

                    if (!keep) {
                        obstacle->m_xCollidedEntity = OBSTACLE_TYPE_NONE;
                    } else {
                        any = 1;
                    }
                }
            }
        }
    }

    if (bestDot != -1.0f) {
        MEC_fn_vReportCollision(p_stDynamic, p_stSupObj, 1, &best, 0);
    }
    return any;
}

static inline DNM_stMecObstacle* inv_MEC_fn_p_stReportedObstacle(s32 index)
{
    return &(&g_stReport.m_stObstacle)[index];
}

void MEC_fn_vReportCollideActor(void* hCollideSupObj, DNM_stObstacle* p_stCollideObstacle)
{
    void* other = p_stCollideObstacle->mp_stSupObj;
    DNM_stDynamics* object;
    DNM_stObstacle* destination;
    MTH3D_tdstVector_ normal;

    if (other != NULL) {
        object = fn_p_stDynamGetDynamicsSO(MEC_fn_hGetFatherActor(other));
        if (object != NULL && (object->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)) {
            object->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState |= 0x80;
            destination = &object->m_tdstDynamicsBaseBlock.m_pstReport->m_stCharacter;
            destination->m_xRate = 0.0f;
            MTH3D_M_vSetXYZ_inv(&normal, inv_MTH_fn_xNegate(p_stCollideObstacle->m_stNorm.xX),
                                inv_MTH_fn_xNegate(p_stCollideObstacle->m_stNorm.xY),
                                inv_MTH_fn_xNegate(p_stCollideObstacle->m_stNorm.xZ));
            destination->m_stNorm = normal;
            destination->m_stContact = p_stCollideObstacle->m_stContact;
            destination->mp_stSupObj = hCollideSupObj;
            destination->hCollidedMaterial = p_stCollideObstacle->hMyMaterial;
            destination->hMyMaterial = p_stCollideObstacle->hCollidedMaterial;
        }
    }
}

void MEC_fn_vReportCollision(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj, s32 lNbObstacle,
                             DNM_stMecObstacle* a_stObstacle, s32 lObstacle)
{
    MTH3D_tdstVector_ up;
    s32 obstacleSpecial;
    s32 haveWall;
    s32 haveGround;
    s32 haveActor;
    s32 haveWater;
    s32 ceilingSpecial;
    s32 wallSpecial;
    s32 actorSpecial;
    s32 waterSpecial;
    s32 groundSpecial;
    s32 index;
    s32 haveCeiling;
    s32 stop;
    s32 haveObstacle;
    f32 group;

    stop = 0;
    haveObstacle = 0;
    haveWall = 0;
    haveGround = 0;
    haveActor = 0;
    haveWater = 0;
    obstacleSpecial = 0;
    ceilingSpecial = 0;
    wallSpecial = 0;
    groundSpecial = 0;
    actorSpecial = 0;
    waterSpecial = 0;
    index = 0;
    haveCeiling = 0;
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    group = 2.0f;

    while (index < lNbObstacle && stop == 0) {
        DNM_stMecObstacle* obstacle = &a_stObstacle[index];
        s32 type = obstacle->m_xCollidedEntity;
        u16 unsignedType = obstacle->m_xCollidedEntity;
        s32 special;

        if ((type ^ OBSTACLE_TYPE_NONE) != 0) {
            f32 obstacleGroup = obstacle->contact.m_xRate;

            if (lObstacle >= index || group == obstacleGroup) {
                u16 otherFlags;
                MTH3D_tdstVector_* normal;

                group = obstacleGroup;
                special = (u16) (unsignedType - OBSTACLE_TYPE_STEP) < 2;
                normal = &obstacle->contact.m_stNorm;
                if (!haveObstacle) {
                    *(DNM_stMecObstacle*) inv_MEC_stReportObstacle = *obstacle;
                    obstacleSpecial = special;
                    haveObstacle = 1;
                } else if (obstacleSpecial && !special) {
                    *(DNM_stMecObstacle*) inv_MEC_stReportObstacle = *obstacle;
                    obstacleSpecial = 0;
                }
                if ((obstacle->m_eType & 2) || ((obstacle->m_eType & 9) == 9)) {
                    if (!haveActor) {
                        inv_MEC_stReportCharacter = *obstacle;
                        actorSpecial = special;
                        haveActor = 1;
                    } else if (actorSpecial && !special) {
                        inv_MEC_stReportCharacter = *obstacle;
                        actorSpecial = 0;
                    }
                }
                otherFlags = obstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;
                if (((otherFlags & MATERIAL_WATER) &&
                     (obstacle->contact.hMyMaterial->collideMaterial.xIdentifier &
                      MATERIAL_WATER)) ||
                    ((otherFlags & MATERIAL_CRASH) && (otherFlags & MATERIAL_CRASH_MASK))) {
                    if (!haveWater) {
                        inv_MEC_stReportWater = *obstacle;
                        waterSpecial = special;
                        haveWater = 1;
                    } else if (waterSpecial && !special) {
                        inv_MEC_stReportWater = *obstacle;
                        waterSpecial = 0;
                    }
                } else {
                    f32 dot;
                    f32 slope;

                    dot = inv_MTH_fn_xMulAdd3(normal->xX, up.xX, normal->xY, up.xY, normal->xZ,
                                              up.xZ);
                    slope = p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope;
                    if (slope < dot) {
                        if (!haveGround) {
                            *(DNM_stMecObstacle*) inv_MEC_stReportGround = *obstacle;
                            groundSpecial = special;
                            haveGround = 1;
                        } else if (groundSpecial && !special) {
                            *(DNM_stMecObstacle*) inv_MEC_stReportGround = *obstacle;
                            groundSpecial = 0;
                        }
                    } else {
                        slope = inv_MTH_fn_xNegateTied(slope);
                        if (slope < dot) {
                            if (!haveWall) {
                                inv_MEC_stReportWall = *obstacle;
                                wallSpecial = special;
                                haveWall = 1;
                            } else if (wallSpecial && !special) {
                                inv_MEC_stReportWall = *obstacle;
                                wallSpecial = 0;
                            }
                        } else if (!haveCeiling) {
                            inv_MEC_stReportCeil = *obstacle;
                            ceilingSpecial = special;
                            haveCeiling = 1;
                        } else if (ceilingSpecial && !special) {
                            inv_MEC_stReportCeil = *obstacle;
                            ceilingSpecial = 0;
                        }
                    }
                }
            } else {
                stop = 1;
            }
        }
        index++;
    }

    if (haveActor) {
        g_stReport.m_ulCurrSurfaceState |= 0x10000;
        MEC_fn_vReportCollideActor(hSupObj, &inv_MEC_fn_p_stReportedObstacle(3)->contact);
    }
    if (haveWater) {
        g_stReport.m_ulCurrSurfaceState |= 0x40;
    }
    if (haveGround) {
        g_stReport.m_ulCurrSurfaceState |= 1;
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stGroundNormal =
                inv_MEC_fn_p_stReportedObstacle(1)->contact.m_stNorm;
        }
    }
    if (haveWall) {
        g_stReport.m_ulCurrSurfaceState |= 4;
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal =
                inv_MEC_fn_p_stReportedObstacle(2)->contact.m_stNorm;
        }
    }
    if (haveCeiling) {
        g_stReport.m_ulCurrSurfaceState |= 0x10;
    }
}

void MEC_fn_vFillCollisionReport(DNM_stDynamics* p_stDynamic)
{
    DNM_stReport_* report;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) &&
        g_stReport.m_ulCurrSurfaceState) {
        report = p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport;
        report->m_stObstacle = g_stReport.m_stObstacle.contact;
        if (g_stReport.m_ulCurrSurfaceState & 0x10000) {
            report->m_ulCurrSurfaceState |= 0x10000;
            report->m_stCharacter = g_stReport.m_stCharacter.contact;
        }
        if (g_stReport.m_ulCurrSurfaceState & 0x40) {
            report->m_ulCurrSurfaceState |= 0x40;
            report->m_stWater = g_stReport.m_stWater.contact;
        }
        if (g_stReport.m_ulCurrSurfaceState & 1) {
            report->m_ulCurrSurfaceState |= 1;
            report->m_stGround = g_stReport.m_stGround.contact;
        }
        if (g_stReport.m_ulCurrSurfaceState & 4) {
            report->m_ulCurrSurfaceState |= 4;
            report->m_stWall = g_stReport.m_stWall.contact;
        }
        if (g_stReport.m_ulCurrSurfaceState & 0x10) {
            report->m_ulCurrSurfaceState |= 0x10;
            report->m_stCeil = g_stReport.m_stCeil.contact;
        }
    }
}

static inline u8 inv_MEC_fn_bIsWater(DNM_stMecObstacle* obstacle)
{
    u8 water = 0;

    if ((obstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier & MATERIAL_WATER) &&
        (obstacle->contact.hMyMaterial->collideMaterial.xIdentifier & MATERIAL_WATER)) {
        water = 1;
    }
    return water;
}

static inline s32 inv_MEC_fn_bIsNegative(f32 value)
{
    return value < 0.0f;
}

u8 MEC_fn_vComputeAdjustVector(MTH3D_tdstVector_* p_stAdjustVector, void* object, s32 lObstacle,
                                DNM_stMecObstacle* p_stObstacle, s32 lObstacleMin)
{
    MTH3D_tdstVector_ stTmp;
    MTH3D_tdstVector_ stDiff;
    MTH3D_tdstVector_ stCur;
    MTH3D_tdstVector_ stN;
    MTH3D_tdstVector_ stDir;
    DNM_stMecObstacle* p;
    DNM_stMecObstacle* pChosen;
    s32 i;
    s32 lMask;
    s32 bInit;
    s32 bContinue;
    s32 bOut;
    s32 bNull;
    s32 bSlide;
    s32 lType;
    f32 xDot;
    f32 xDot2;

    bInit = 0;
    pChosen = NULL;
    bOut = 0;
    p_stAdjustVector->xZ = p_stAdjustVector->xY = p_stAdjustVector->xX = 0.0f;

    for (i = 0; i < lObstacle && pChosen == NULL; i++) {
        p = &p_stObstacle[i];
        lType = p->m_xCollidedEntity;
        if ((lType ^ OBSTACLE_TYPE_NONE) != 0) {
            bSlide = 0;
            if ((p->contact.hCollidedMaterial->collideMaterial.xIdentifier & MATERIAL_WATER) &&
                (p->contact.hMyMaterial->collideMaterial.xIdentifier & MATERIAL_WATER)) {
                bSlide = 1;
            }
            if (bSlide) {
                pChosen = p;
            }
        }
    }

    lMask = 1;
    do {
        bContinue = 1;
        for (i = 0; i < lObstacle && bContinue; i++) {
            p = &p_stObstacle[i];
            if ((p->m_eType & lMask) != lMask) {
                continue;
            }
            if (i <= lObstacleMin || inv_MEC_fn_bIsNegative(p->contact.m_xRate)) {
                lType = p->m_xCollidedEntity;
                if ((lType ^ OBSTACLE_TYPE_NONE) == 0) {
                    continue;
                }
                if (inv_MEC_fn_bIsWater(p)) {
                    continue;
                }
                if (bInit) {
                    stCur = p->m_stTranslation;
                    bNull = 0;
                    if (inv_MTH_fn_xAbsoluteTied(p_stAdjustVector->xX - 0.0f) < VECTOR_EPSILON &&
                        inv_MTH_fn_xAbsoluteTied(p_stAdjustVector->xY - 0.0f) < VECTOR_EPSILON &&
                        inv_MTH_fn_xAbsoluteTied(p_stAdjustVector->xZ - 0.0f) < VECTOR_EPSILON) {
                        bNull = 1;
                    }
                    if (!bNull) {
                        f32 xLen;
                        f32 xInv;

                        xLen = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stAdjustVector->xX, p_stAdjustVector->xY, p_stAdjustVector->xZ);
                        xLen = MTH_fn_xSqrtOpt2(xLen);
                        if (xLen != 0.0f) {
                            xInv = 1.0f / xLen;
                            inv_MTH3D_fn_vKTimesXYZ(&stDir, xInv, p_stAdjustVector);
                        } else {
                            stDir = *p_stAdjustVector;
                        }
                    } else {
                        stDir = *p_stAdjustVector;
                    }
                    xDot = inv_MTH_fn_xMulAdd3Grouped(stDir.xX, stDir.xY, stDir.xZ,
                                            stCur.xX, stCur.xY, stCur.xZ);
                    if (0.0f < xDot) {
                        f32 xLen2;

                        xLen2 = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stAdjustVector->xX, p_stAdjustVector->xY, p_stAdjustVector->xZ);
                        xLen2 = MTH_fn_xSqrtOpt2(xLen2);
                        if (xDot < xLen2) {
                            inv_MTH3D_fn_vKTimesXYZ(&stTmp, xDot, &stDir);
                            inv_MTH3D_fn_vSubXYZ(&stDiff, &stCur, &stTmp);
                            inv_MTH3D_fn_vAddXYZ(p_stAdjustVector, p_stAdjustVector, &stDiff);
                        } else {
                            *p_stAdjustVector = stCur;
                        }
                    } else {
                        bOut = 1;
                        bNull = 0;
                        if (inv_MTH_fn_xAbsoluteTied(stCur.xX - 0.0f) < VECTOR_EPSILON &&
                            inv_MTH_fn_xAbsoluteTied(stCur.xY - 0.0f) < VECTOR_EPSILON &&
                            inv_MTH_fn_xAbsoluteTied(stCur.xZ - 0.0f) < VECTOR_EPSILON) {
                            bNull = 1;
                        }
                        if (!bNull) {
                            f32 xLen;
                            f32 xInv;

                            xLen = MTH_fn_xSqrAddSqrAddSqrOpt2(stCur.xX, stCur.xY, stCur.xZ);
                            xLen = MTH_fn_xSqrtOpt2(xLen);
                            if (xLen != 0.0f) {
                                xInv = 1.0f / xLen;
                                inv_MTH3D_fn_vKTimesXYZ(&stN, xInv, &stCur);
                            } else {
                                stN = stCur;
                            }
                        } else {
                            stN = stCur;
                        }
                        if (lMask != 1) {
                            xDot2 = inv_MTH_fn_xMulAdd3Grouped(stDir.xX, stDir.xY, stDir.xZ,
                                                     stN.xX, stN.xY, stN.xZ);
                            if (!(-0.258f < xDot2)) {
                                continue;
                            }
                        }
                        inv_MTH3D_fn_vKTimesXYZ(&stTmp, xDot, &stDir);
                        inv_MTH3D_fn_vSubXYZ(&stDiff, &stCur, &stTmp);
                        inv_MTH3D_fn_vAddXYZ(p_stAdjustVector, p_stAdjustVector, &stDiff);
                    }
                } else {
                    *p_stAdjustVector = p->m_stTranslation;
                    bInit = 1;
                }
            } else {
                bContinue = 0;
            }
        }
        lMask = (lMask == 1) ? 2 : 9;
    } while (lMask != 9);

    if (pChosen != NULL) {
        f32 xLen;
        f32 xInv;

        xLen =
            MTH_fn_xSqrAddSqrAddSqrOpt2(pChosen->contact.m_stNorm.xX, pChosen->contact.m_stNorm.xY,
                                        pChosen->contact.m_stNorm.xZ);
        xLen = MTH_fn_xSqrtOpt2(xLen);
        if (xLen != 0.0f) {
            xInv = 1.0f / xLen;
            MTH3D_M_vKTimesXYZ_inv(&stDir, xInv, &pChosen->contact.m_stNorm);
        } else {
            stDir = pChosen->contact.m_stNorm;
        }
        xDot = inv_MTH_fn_xMulAdd3Grouped(stDir.xX, stDir.xY, stDir.xZ,
                                p_stAdjustVector->xX, p_stAdjustVector->xY, p_stAdjustVector->xZ);
        xDot = xDot - 0.01f;
        inv_MTH3D_fn_vKTimesXYZ(&stTmp, xDot, &stDir);
        inv_MTH3D_fn_vSubXYZ(p_stAdjustVector, p_stAdjustVector, &stTmp);
        MTH3D_M_vAddXYZ_inv(p_stAdjustVector, p_stAdjustVector, &pChosen->m_stTranslation);
    }
    return !bOut;
}

void MEC_fn_vEdgeAnalysis(DNM_stDynamics* p_stDynamic, s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                        DNM_stMecObstacle* p_stGround)
{
    MTH3D_tdstVector_ stPoint;
    MTH3D_tdstVector_ stDelta;
    MTH3D_tdstVector_ stSum;
    MTH3D_tdstVector_ stDir;
    DNM_stMecObstacle* p;
    DNM_stMecObstacle* pChosen;
    MTH3D_tdstVector_* pDir;
    s32 i;
    s32 lNear;
    s32 bFound;
    s32 bNull;
    s32 lType;
    f32 xDot;
    f32 xNx;

    lNear = 0;
    bFound = 0;
    pChosen = NULL;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x40;

    if (p_stGround == NULL) {
        for (i = 0, p = p_stObstacle; i < lObstacle && p_stGround == NULL; i++, p++) {
            lType = p->m_xCollidedEntity;
            if ((lType ^ OBSTACLE_TYPE_NONE) != 0 &&
                (u8) MEC_fn_bGroundNormal(p_stDynamic, &p->contact.m_stNorm)) {
                if ((u16) (p->m_xCollidedEntity - OBSTACLE_TYPE_STEP) >= 2) {
                    return;
                }
                p_stGround = p;
            }
        }
        if (p_stGround == NULL) {
            return;
        }
    } else if ((u16) (p_stGround->m_xCollidedEntity - OBSTACLE_TYPE_STEP) >= 2) {
        return;
    }

    if ((p_stGround->m_eType >> 2) & 1) {
        return;
    }

    stPoint = p_stGround->contact.m_stContact;
    MTH3D_M_vAddXYZ_inv(&stDir, &p_stGround->m_stZonePosition, &p_stGround->m_stTranslation);
    inv_MTH3D_fn_vSubXYZ(&stDir, &stDir, &stPoint);
    {
        f32 xNorm;

        inv_MTH3D_fn_vNormalizeXYZ(&stDir);
    }

    p = p_stObstacle;
    for (i = 0; i < lObstacle; i++, p++) {
        lType = p->m_xCollidedEntity;
        if ((lType ^ OBSTACLE_TYPE_NONE) == 0) {
            continue;
        }
        if ((u8) MEC_fn_bGroundNormal(p_stDynamic, &p->contact.m_stNorm)) {
            if ((u16) (p->m_xCollidedEntity - OBSTACLE_TYPE_STEP) >= 2) {
                return;
            }
            bNull = 0;
            MTH3D_M_vSubXYZ_inv(&stDelta, &stPoint, &p->contact.m_stContact);
            if (inv_MTH_fn_xAbsoluteTied(stDelta.xX - 0.0f) < VECTOR_EPSILON &&
                inv_MTH_fn_xAbsoluteTied(stDelta.xY - 0.0f) < VECTOR_EPSILON &&
                inv_MTH_fn_xAbsoluteTied(stDelta.xZ - 0.0f) < VECTOR_EPSILON) {
                bNull = 1;
            }
            lNear += bNull;
            continue;
        }
        lType = p->m_xCollidedEntity;
        if ((lType ^ OBSTACLE_TYPE_STEP_EDGE) == 0) {
            bNull = 0;
            MTH3D_M_vSubXYZ_inv(&stDelta, &stPoint, &p->contact.m_stContact);
            if (inv_MTH_fn_xAbsoluteTied(stDelta.xX - 0.0f) < VECTOR_EPSILON &&
                inv_MTH_fn_xAbsoluteTied(stDelta.xY - 0.0f) < VECTOR_EPSILON &&
                inv_MTH_fn_xAbsoluteTied(stDelta.xZ - 0.0f) < VECTOR_EPSILON) {
                bNull = 1;
            }
            if (bNull) {
                xNx = p->contact.m_stNorm.xX;
                if (0.5f < p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide) {
                    xDot = inv_MTH_fn_xMulAdd3(p_stGround->contact.m_stNorm.xX, xNx,
                                    p_stGround->contact.m_stNorm.xY, p->contact.m_stNorm.xY,
                                    p_stGround->contact.m_stNorm.xZ, p->contact.m_stNorm.xZ);
                    if (0.707f < xDot) {
                        return;
                    }
                }
                xDot = inv_MTH_fn_xMulAdd3(xNx, stDir.xX, p->contact.m_stNorm.xY,
                                stDir.xY, p->contact.m_stNorm.xZ, stDir.xZ);
                if (pChosen == NULL && 0.0f < xDot) {
                    bFound = 1;
                    pChosen = p;
                }
            }
        } else if ((u8) MEC_fn_bWallNormal(p_stDynamic, &p->contact.m_stNorm)) {
            lType = p->m_xCollidedEntity;
            if ((lType ^ OBSTACLE_TYPE_STEP) != 0) {
                MTH3D_M_vSubXYZ_inv(&stDelta, &stPoint, &p->contact.m_stContact);
                xDot = inv_MTH_fn_xMulAdd3(p_stGround->contact.m_stNorm.xX, stDelta.xX,
                                p_stGround->contact.m_stNorm.xY, stDelta.xY,
                                p_stGround->contact.m_stNorm.xZ, stDelta.xZ);
                if (xDot < 0.0f) {
                    xDot = inv_MTH_fn_xMulAdd3(p->contact.m_stNorm.xX, stDelta.xX,
                                               p->contact.m_stNorm.xY, stDelta.xY,
                                               p->contact.m_stNorm.xZ, stDelta.xZ);
                    if (xDot < -VECTOR_EPSILON) {
                        return;
                    }
                }
            }
        }
    }

    if (lNear >= 2) {
        return;
    }

    if (bFound) {
        bNull = 0;
        MTH3D_M_vAddXYZ_inv(&stSum, &p_stGround->contact.m_stNorm, &pChosen->contact.m_stNorm);
        if (inv_MTH_fn_xAbsoluteTied(stSum.xX - 0.0f) < VECTOR_EPSILON &&
            inv_MTH_fn_xAbsoluteTied(stSum.xY - 0.0f) < VECTOR_EPSILON &&
            inv_MTH_fn_xAbsoluteTied(stSum.xZ - 0.0f) < VECTOR_EPSILON) {
            bNull = 1;
        }
        bFound = bNull ^ 1;
        if (bFound) {
            f32 xNorm;
            f32 xCos;

            inv_MTH3D_fn_vNormalizeXYZ(&stSum);
            xCos = inv_MTH_fn_xMulAdd3(stDir.xX, stSum.xX, stDir.xY, stSum.xY, stDir.xZ, stSum.xZ);
            if (xCos < 0.0f) {
                return;
            }
        }
    }

    if (stDir.xZ < 0.9848f) {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x40;
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
            pDir = &stDir;
            pDir->xZ = 0.0f;
            bNull = 0;
            xNx = stDir.xZ;
            if (inv_MTH_fn_xAbsoluteTied(stDir.xX - xNx) < VECTOR_EPSILON &&
                __builtin_fabsf(stDir.xY - xNx) < VECTOR_EPSILON &&
                __builtin_fabsf(xNx) < VECTOR_EPSILON) {
                bNull = 1;
            }
            if (!bNull) {
                f32 xNorm;

                xNorm =
                    inv_MTH_fn_xMulAdd3(stDir.xX, stDir.xX, stDir.xY, stDir.xY, stDir.xZ, stDir.xZ);
                xNorm = MTH_fn_xSqrtOpt2(xNorm);
                if (xNorm != 0.0f) {
                    f32 xInv = 1.0f / xNorm;

                    inv_MTH3D_fn_vKTimesXYZ(&stDir, xInv, &stDir);
                }
            } else {
                f32 xZero = 0.0f;

                MTH3D_M_vSetXYZ_inv(&stDir, xZero, xZero, xZero);
            }
            p_stDynamic->m_tdstDynamicsComplexBlock.m_stFallTranslation = stDir;
        }
    }
}

void MEC_fn_vCrashAnalysis(DNM_stDynamics* p_stDynamic, s32 lObstacle, DNM_stMecObstacle* p_stObstacle)
{
    DNM_stMecObstacle* p_stTmpObstacle;
    DNM_stMecObstacle* p_stGroundObstacle;
    s32 result;
    u32 flags;

    p_stTmpObstacle = NULL;
    p_stGroundObstacle = NULL;
    flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
    flags &= ~0x80000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = flags;
    {
        s32 i;

        for (i = 0; i < lObstacle; i++, p_stObstacle++) {
            s32 type = p_stObstacle->m_xCollidedEntity;

            if ((type ^ OBSTACLE_TYPE_NONE) != 0) {
                if (p_stTmpObstacle == NULL) {
                    MTH3D_tdstVector_* normal = &p_stObstacle->contact.m_stNorm;

                    p_stTmpObstacle = p_stObstacle;
                    result = MEC_fn_bGroundNormal(p_stDynamic, normal);
                    if ((result & 0xFF) == 0) {
                        p_stTmpObstacle = NULL;
                    }
                }
                if (p_stGroundObstacle == NULL) {
                    result = MEC_fn_bCeilObstacle(p_stDynamic, &p_stObstacle->contact);
                    p_stGroundObstacle = NULL;
                    if (result != 0) {
                        p_stGroundObstacle = p_stObstacle;
                    }
                }
            }
        }
    }
    if (p_stGroundObstacle == NULL || p_stTmpObstacle == NULL) {
        return;
    }
    if ((p_stTmpObstacle->m_eType & 2) != 0 && MEC_fn_bCanCrashActor(p_stTmpObstacle) != 0) {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x80000;
        return;
    }
    if ((p_stGroundObstacle->m_eType & 2) != 0 && MEC_fn_bCanCrashActor(p_stGroundObstacle) != 0) {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x80000;
    }
}
