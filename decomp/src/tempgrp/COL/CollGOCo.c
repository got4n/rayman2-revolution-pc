#include "common.h"

#include "COL/COL.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "MEC/MEC.h"

extern COL_tdstCollisionCase_* D_00393E04;

void COL_fn_vAddInStaticCollisionTable(f32 _xT, MTH3D_tdstVector_* _p_stTranslation, MTH3D_tdstVector_* _p_stHit,
                   MTH3D_tdstVector_* _p_stNormal, GMT_tdstGameMaterial_* _hDynMat,
                   GMT_tdstGameMaterial_* _hStatMat, s16 _xDynamicGeomEntity, s16 _xStaticGeomEntity,
                   void* _p_vParameter1, f32 _xSphereRadius, s16 _sParameter2,
                   MTH3D_tdstVector_* _p_stMovement, MTH3D_tdstVector_* _p_stEndPosition)
{
    COL_tdstCollisionCase_* p_stCollisionCase;
    COL_tdstCollisionCase_* end;
    COL_tdstCollisionCase_* p;
    COL_tdstCollisionCase_* pNext;
    s32 i;
    s32 iNumberOfElementToCopy;
    GMT_tdstCollideMaterial_* hCollideMaterial;

    hCollideMaterial = GMT_fn_hGetCollideMaterial(_hDynMat);
    if (hCollideMaterial != (GMT_tdstCollideMaterial_*) -1 &&
        (GMT_fn_hGetCollideMaterialIdentifier(hCollideMaterial) & 8)) {
        hCollideMaterial = GMT_fn_hGetCollideMaterial(_hStatMat);
        if (hCollideMaterial == (GMT_tdstCollideMaterial_*) -1 ||
            !(GMT_fn_hGetCollideMaterialIdentifier(hCollideMaterial) & 4)) {
            return;
        }
    }

    if (_xStaticGeomEntity == 7 || _xStaticGeomEntity == 8) {
        p_stCollisionCase = COL_g_stCollisionCaseReal;
        for (i = 0; i < g_NbCollidedEdges; i++, p_stCollisionCase++) {
            if (p_stCollisionCase->xStaticGeomEntity == _xStaticGeomEntity &&
                p_stCollisionCase->p_vParameter1 == _p_vParameter1 &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->xCollisionTime - _xT) < 0.001f &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->stCollisionPoint.xX - _p_stHit->xX) <
                    0.005f &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->stCollisionPoint.xY - _p_stHit->xY) <
                    0.005f &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->stCollisionPoint.xZ - _p_stHit->xZ) <
                    0.005f &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->stCollisionNormal.xX - _p_stNormal->xX) <
                    0.005f &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->stCollisionNormal.xY - _p_stNormal->xY) <
                    0.005f &&
                inv_MTH_fn_xAbsolute(p_stCollisionCase->stCollisionNormal.xZ - _p_stNormal->xZ) <
                    0.005f) {
                p_stCollisionCase->sParameter2 |= 2;
                return;
            }
        }
    }

    p_stCollisionCase = COL_g_stCollisionCaseReal;
    for (i = 0; i < g_NbCollidedEdges; i++, p_stCollisionCase++) {
        if (inv_MTH_fn_xAbsolute(_xT - p_stCollisionCase->xCollisionTime) < 0.0001f) {
            if (_p_stNormal->xX < p_stCollisionCase->stCollisionNormal.xX) {
                end = &COL_g_stCollisionCaseReal[g_NbCollidedEdges];
                iNumberOfElementToCopy = 0;
                p = p_stCollisionCase;
                pNext = p_stCollisionCase + 1;
                while (p < end) {
                    iNumberOfElementToCopy++;
                    p = pNext;
                    pNext++;
                }
                if (pNext == &D_00393E04[76]) {
                    iNumberOfElementToCopy--;
                }
                if (iNumberOfElementToCopy != 0) {
                    memmove(p_stCollisionCase + 1, p_stCollisionCase,
                            iNumberOfElementToCopy * sizeof(COL_tdstCollisionCase_));
                }
                break;
            }
        } else if (_xT < p_stCollisionCase->xCollisionTime) {
            end = &COL_g_stCollisionCaseReal[g_NbCollidedEdges];
            iNumberOfElementToCopy = 0;
            p = p_stCollisionCase;
            pNext = p_stCollisionCase + 1;
            while (p < end) {
                iNumberOfElementToCopy++;
                p = pNext;
                pNext++;
            }
            if (pNext == &D_00393E04[76]) {
                iNumberOfElementToCopy--;
            }
            if (iNumberOfElementToCopy != 0) {
                memmove(p_stCollisionCase + 1, p_stCollisionCase,
                        iNumberOfElementToCopy * sizeof(COL_tdstCollisionCase_));
            }
            break;
        }
    }
    if (i != D_00393E10) {
        p_stCollisionCase->xCollisionTime = _xT;
        p_stCollisionCase->stTranslation = *_p_stTranslation;
        p_stCollisionCase->stCollisionPoint = *_p_stHit;
        p_stCollisionCase->stCollisionNormal = *_p_stNormal;
        p_stCollisionCase->stMovement = *_p_stMovement;
        p_stCollisionCase->stEndPosition = *_p_stEndPosition;
        p_stCollisionCase->hDynamicMaterial = _hDynMat;
        p_stCollisionCase->hStaticMaterial = _hStatMat;
        p_stCollisionCase->xDynamicGeomEntity = _xDynamicGeomEntity;
        p_stCollisionCase->xStaticGeomEntity = _xStaticGeomEntity;
        p_stCollisionCase->xSphereRadius = _xSphereRadius;
        p_stCollisionCase->p_vParameter1 = _p_vParameter1;
        p_stCollisionCase->sParameter2 = _sParameter2;
        if (g_NbCollidedEdges != D_00393E10) {
            g_NbCollidedEdges++;
        }
    }
}
