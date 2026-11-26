#include "common.h"

#include "GAM/GAM.h"
#include "GEO/GEO.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "PO/PO.h"
#include "SCT/SCT.h"
#include "CAM/CAM.h"
#include "ps2/PS2.h"

void fn_vSendOneModuleToViewPort(void* _hVpt, HIE_tdstSuperObject_* _hSprObj, s32 _lDrawMask)
{
    f32 fCullingTransparency;

    GLI_vSetLensFlareRef(_hSprObj);
    fCullingTransparency = GLI_vGetGlobalAlpha();
    GLI_vSetGlobalAlpha(GEO_xGetZFarTransparencyLevel() * _hSprObj->gu_fTransparenceLevel);
    if (_hSprObj->lTypeOfLinkedObject == HIE_TYPE_PHYSICAL_OBJECT_inv) {
        PO_tdstPhysicalObject_* hPhysicalObject = _hSprObj->hLinkedObject;
        void* hRad = hPhysicalObject->_hVisualBoundingVolume;
        void* hVisualSet = hPhysicalObject->_hVisualSet;

        if (hRad != NULL) {
            func_00214F98(_hVpt, hVisualSet, hRad, _lDrawMask);
        } else {
            GLI_lSendVisualSetToViewport2(_hVpt, hVisualSet, _lDrawMask);
        }
    } else if (_hSprObj->lTypeOfLinkedObject & 0x480) {
        GLI_xSendObjectToViewportWithLights(_hVpt, _hSprObj->hLinkedObject, _lDrawMask);
    }
    GLI_vSetGlobalAlpha(fCullingTransparency);
}

s32 HIE_fn_bIsSegmentIntersectedInCameraSector(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* target)
{
    MTH3D_tdstVector_ origin;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ segment;
    MTH3D_tdstVector_ hit1;
    MTH3D_tdstVector_ hit0;
    POS_tdstMatrix_ inverse;
    HIE_tdstSuperObject_* hitData;
    HIE_tdstSuperObject_* camera;
    tdstEngineObject_* object;
    HIE_tdstSuperObject_* sectorSuperObject;
    SECT_tdstSector_* sector;
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    s16 i;

    camera = CAM_fn_hGetActiveCamera(1);
    POS_fn_vInvertIsoMatrix(&inverse, matrix);
    origin = *inv_POS_fn_p_stPosMatrixTranslation(&inverse);
    segment = *target;
    inv_MTH3D_fn_vSubXYZ(&direction, &segment, &origin);
    object = camera->hLinkedObject;
    sectorSuperObject = object->h_SectInfo->hCurrentSector;
    segment = direction;
    if (HIE_bIntersectSegmentWithFirstSuperObject(&origin, &segment, sectorSuperObject, &hit0,
                                                  &hit1, &hitData) != 0) {
        return 1;
    }
    sector = sectorSuperObject->hLinkedObject;
    neighbor = sector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    for (i = 0; i < sector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt;
         i++, neighbor++) {
        segment = direction;
        if (HIE_bIntersectSegmentWithFirstSuperObject(
                &origin, &segment, neighbor->hPointerOfSectorSO, &hit0, &hit1, &hitData) != 0) {
            return 1;
        }
    }
    return 0;
}
