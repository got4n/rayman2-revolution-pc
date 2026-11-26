#include "common.h"

#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "GAM/GAM.h"

#define PLATFORM_LINK_FORCED 0x40000000
#define PLATFORM_KEEP_ORIENTATION 0x1000
#define CONTACT_ON_PLATFORM 0x10000
#define CONTACT_CEILING 0x10
#define CONTACT_WALL 0x40
#define IDCARD_PLATFORM_DYNAMICS 1

void MEC_fn_vDynamicComputeAbsoluteMatrix(DNM_stDynamics* p_stDynamic,
                                          HIE_tdstSuperObject_* hSupObj,
                                          POS_tdstMatrix_* hPreviousMatrix,
                                          POS_tdstMatrix_* hInvPreviousMatrix,
                                          POS_tdstMatrix_* hCurrentMatrix);
void MEC_fn_vSetPlatformLink(DNM_stDynamics* dynamics, HIE_tdstSuperObject_* child,
                             HIE_tdstSuperObject_* platform);
void MEC_fn_vCutPlatformLink(DNM_stDynamics* dynamics, HIE_tdstSuperObject_* superObject);

void MEC_fn_vUpdatePlatform(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj,
                            POS_tdstMatrix_* previous, POS_tdstMatrix_* motion)
{
    POS_tdstMatrix_* current = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    HIE_tdstSuperObject_* hChild = hSupObj->hFirstElementSta;
    DNM_stDynamics* p_stChildDynamic;

    while (hChild != NULL) {
        if (hChild->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            p_stChildDynamic = fn_p_stDynamGetDynamicsSO(hChild);
            if (p_stChildDynamic != NULL && p_stChildDynamic->m_tdstDynamicsBaseBlock.m_pCurrentIdCard != NULL &&
                DNM_fn_eIdentityCardGetType(p_stChildDynamic->m_tdstDynamicsBaseBlock.m_pCurrentIdCard) ==
                    IDCARD_PLATFORM_DYNAMICS) {
                MEC_fn_vDynamicComputeAbsoluteMatrix(p_stChildDynamic, hChild, previous, motion, current);
            }
        }
        hChild = hChild->hNextBrotherSta;
    }
}

s32 MEC_fn_vBasePlatformManagement(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj)
{
    DNM_stObstacle* contact;
    u32 optionFlags;
    HIE_tdstSuperObject_* contactObject;
    HIE_tdstSuperObject_* father;
    u8 hasNormal;
    s32 platformDynamics = 0;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.m_pCurrentIdCard != NULL) {
        platformDynamics =
            DNM_fn_eIdentityCardGetType(p_stDynamic->m_tdstDynamicsBaseBlock.m_pCurrentIdCard) ==
            IDCARD_PLATFORM_DYNAMICS;
    }
    if (!platformDynamics) {
        return 0;
    }
    if (g_stReport.m_ulCurrSurfaceState & CONTACT_ON_PLATFORM) {
        optionFlags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
        contact = &g_stReport.m_stCharacter.contact;
        contactObject = contact->mp_stSupObj;
        if (optionFlags & CONTACT_CEILING) {
            if (!MEC_fn_bCeilObstacle(p_stDynamic, contact)) {
                return 0;
            }
        } else if (optionFlags & CONTACT_WALL) {
            hasNormal = MEC_fn_bWallNormal(p_stDynamic, &g_stReport.m_stCharacter.contact.m_stNorm);
            if (!hasNormal) {
                return 0;
            }
        } else {
            hasNormal =
                MEC_fn_bGroundNormal(p_stDynamic, &g_stReport.m_stCharacter.contact.m_stNorm);
            if (!hasNormal) {
                return 0;
            }
        }
        if (contactObject != 0) {
            father = MEC_fn_hGetFatherActor(contactObject);
            if (father != NULL && inv_fn_ucSuperObjectGetPlatFormType(father)) {
                if (inv_fn_ucSuperObjectGetPlatFormType(hSupObj)) {
                    return 0;
                }
                if (p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform != NULL) {
                    if (p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform == father) {
                        return 0;
                    }
                    MEC_fn_vCutPlatformLink(p_stDynamic, hSupObj);
                }
                MEC_fn_vSetPlatformLink(p_stDynamic, hSupObj, father);
                return 1;
            }
        }
    }
    if (p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform != NULL) {
        MEC_fn_vCutPlatformLink(p_stDynamic, hSupObj);
        return 1;
    }
    return 0;
}

void MEC_fn_vSetPlatformLink(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj,
                             HIE_tdstSuperObject_* hFatherSupObj)
{
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & PLATFORM_LINK_FORCED) {
        return;
    }
    p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform = hFatherSupObj;
    if (hSupObj->hNextBrotherSta != NULL) {
        hSupObj->hNextBrotherSta->hPrevBrotherSta = hSupObj->hPrevBrotherSta;
    }
    if (hSupObj->hPrevBrotherSta != NULL) {
        hSupObj->hPrevBrotherSta->hNextBrotherSta = hSupObj->hNextBrotherSta;
    }
    if (hSupObj->hFatherSta != NULL) {
        hSupObj->hFatherSta->lNumberOfElementsSta--;
        if (hSupObj->hFatherSta->hFirstElementSta == hSupObj) {
            hSupObj->hFatherSta->hFirstElementSta = hSupObj->hNextBrotherSta;
        }
        if (hSupObj->hFatherSta->hLastElementSta == hSupObj) {
            hSupObj->hFatherSta->hLastElementSta = hSupObj->hPrevBrotherSta;
        }
    }
    hSupObj->hFatherSta = NULL;
    hSupObj->hNextBrotherSta = NULL;
    hSupObj->hPrevBrotherSta = NULL;
    HIE_fn_vSuperObjectAddTail(hFatherSupObj, hSupObj);
}

void MEC_fn_vCutPlatformLink(DNM_stDynamics* dynamics, HIE_tdstSuperObject_* superObject)
{
    if (!(dynamics->m_tdstDynamicsBaseBlock.ulFlags & PLATFORM_LINK_FORCED)) {
        dynamics->m_tdstDynamicsComplexBlock.m_hPlatform = NULL;
        fn_vReputCharacterSuperObjectAtTheWorld(superObject);
    }
}

void MEC_fn_vForcePlatformLink(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj,
                               HIE_tdstSuperObject_* hFatherSupObj)
{
    MEC_fn_vSetPlatformLink(p_stDynamic, hSupObj, hFatherSupObj);
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= PLATFORM_LINK_FORCED;
}

void MEC_fn_vFreePlatformLink(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj)
{
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~PLATFORM_LINK_FORCED;
    MEC_fn_vCutPlatformLink(p_stDynamic, hSupObj);
}

void MEC_fn_vDynamicComputeAbsoluteMatrix(DNM_stDynamics* p_stDynamic,
                                          HIE_tdstSuperObject_* hSupObj,
                                          POS_tdstMatrix_* hPreviousMatrix,
                                          POS_tdstMatrix_* hInvPreviousMatrix,
                                          POS_tdstMatrix_* hCurrentMatrix)
{
    POS_tdstMatrix_ stMatrix;
    MTH3D_tdstVector_ stVector;
    MTH3D_tdstVector_ stI;
    MTH3D_tdstVector_ stJ;
    MTH3D_tdstVector_ stK;
    POS_tdstMatrix_* hPreviousDynamicMatrix =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    POS_tdstMatrix_* hCurrentDynamicMatrix =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH3D_tdstVector_* position = inv_POS_fn_p_stPosMatrixTranslation(hCurrentDynamicMatrix);

    _POS_fn_vMulMatrixMatrix(&stMatrix, hCurrentMatrix, hInvPreviousMatrix);
    stVector = *position;
    stVector.xX = 0.0f;
    stVector.xY = 0.0f;
    stVector.xZ -= p_stDynamic->m_tdstDynamicsComplexBlock.m_stContact.xZ;
    _POS_fn_vMulMatrixMatrix(hPreviousDynamicMatrix, &stMatrix, hPreviousDynamicMatrix);
    _POS_fn_vMulMatrixMatrix(hCurrentDynamicMatrix, &stMatrix, hCurrentDynamicMatrix);

    if (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & PLATFORM_KEEP_ORIENTATION) &&
        !MEC_fn_bIsEqualMatrix(hPreviousMatrix, hCurrentMatrix)) {
        POS_fn_vGetRotationVectors(hPreviousDynamicMatrix, &stI, &stJ, &stK);
        stJ.xZ = 0.0f;
        inv_MTH3D_fn_vNormalizeXYZ(&stJ);
        MTH3D_M_vSetXYZ_inv(&stK, 0.0f, 0.0f, 1.0f);
        DNM_p_stVectorCrossProduct(&stI, &stJ, &stK);
        inv_MTH3D_fn_vNormalizeXYZ(&stI);
        inv_POS_fn_vInitMatrixFromVectors(hPreviousDynamicMatrix, 5, &stI, &stJ, &stK,
                                          inv_POS_fn_p_stPosMatrixTranslation(hPreviousDynamicMatrix));
        POS_fn_vSetScaleMatrix(hPreviousDynamicMatrix,
                               &p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale);

        POS_fn_vGetRotationVectors(hCurrentDynamicMatrix, &stI, &stJ, &stK);
        stJ.xZ = 0.0f;
        inv_MTH3D_fn_vNormalizeXYZ(&stJ);
        MTH3D_M_vSetXYZ_inv(&stK, 0.0f, 0.0f, 1.0f);
        DNM_p_stVectorCrossProduct(&stI, &stJ, &stK);
        inv_MTH3D_fn_vNormalizeXYZ(&stI);
        inv_POS_fn_vInitMatrixFromVectors(hCurrentDynamicMatrix, 5, &stI, &stJ, &stK, position);
        POS_fn_vSetScaleMatrix(hCurrentDynamicMatrix,
                               &p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale);
    }

    *hSupObj->gu_hGlobalMatrix = *hCurrentDynamicMatrix;
    HIE_fn_vComputeNewRelativeMatrix(hSupObj);
}
