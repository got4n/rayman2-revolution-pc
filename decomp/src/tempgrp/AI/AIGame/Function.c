#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"

#include "COL/COL.h"
#include "AI/AI.h"
#include "SHW/SHW.h"

#define SHADOW_CASTER 0x100

extern f32 D_0026F4A4[];

static inline void inv_AI_fn_vGetTranslation(MTH3D_tdstVector_* translation, POS_tdstMatrix_* matrix)
{
    *translation = *(MTH3D_tdstVector_*) &matrix->stMatrix.stCol_3;
}

void fn_vComputeCollideResult(HIE_tdstSuperObject_* superObject, HIE_tdstSuperObject_* sectorObject)
{
    MTH3D_tdstVector_ normal;
    MTH3D_tdstVector_ origin;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ contact;
    MTH3D_tdstVector_ borderMinimum;
    MTH3D_tdstVector_ borderMaximum;
    MTH3D_tdstVector_ unused;
    MTH3D_tdstVector_ corners[4];
    MTH3D_tdstVector_ forward;
    MTH3D_tdstVector_ doubled;
    HIE_tdstSuperObject_* element;
    POS_tdstMatrix_* matrix;
    s32 found;
    Data3dShadow_inv* shadow;
    SECT_tdstListOfSOCharacters_* node;
    HIE_tdstSuperObject_* other;
    s32 i;
    SECT_tdstListOfSOSectorsInCollisionInteraction_* objects;
    SECT_tdstListOfSOSectorsInActivityInteraction_* borders;
    HIE_tdstSuperObject_* borderObject;
    SECT_tdstSector_* border;
    f32 scaleX;
    f32 inverseNormalZ;
    f32 attenuation;
    f32 scaleY;
    f32 yScaledX;
    f32 xScaledX;
    f32 yScaledY;
    f32 alpha;
    s32 facesUp;
    s8 mode;
    s32 one;

    found = 0;
    shadow = &inv_HIE_fn_p_stGetEngineObject(superObject)->h_3dData->shadow;
    if (shadow->source.texture == NULL) {
        return;
    }
    matrix = superObject->gu_hGlobalMatrix;
    inv_AI_fn_vGetTranslation(&origin, matrix);
    scaleX = shadow->scaleX;
    scaleY = shadow->scaleY;
    origin.xZ += 1.8f;
    MTH3D_M_vSetXYZ_inv(&direction, 0.0f, 0.0f, -14.5f);
    MTH3D_M_vSetXYZ_inv(&normal, direction.xX, direction.xX, direction.xX);
    MTH3D_M_vSetXYZ_inv(&contact, direction.xX, direction.xX, direction.xX);
    inv_HIE_ulPickExcludeFlags = 0x80;
    other = superObject->hFatherSta;
    element = NULL;
    if (other->lTypeOfLinkedObject & HIE_TYPE_ACTOR_inv) {
        if ((inv_HIE_fn_p_stGetEngineObject(other)->h_3dData->shadow.control.flags & SHADOW_CASTER) &&
            ((inv_HIE_fn_p_stGetEngineObject(other)->h_StandardGame->ucMiscFlags >> 2) & 1) &&
            HIE_bIntersectSegmentWithFirstSuperObject(&origin, &direction, other, &contact,
                                                             &normal, &element)) {
            facesUp = normal.xZ > 0.5f;
            found = facesUp;
        }
    } else {
        for (node = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfCharacters.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
            other = node->hPointerOfCharacter;
            if (other != superObject &&
                (inv_HIE_fn_p_stGetEngineObject(other)->h_3dData->shadow.control.flags & SHADOW_CASTER) &&
                ((inv_HIE_fn_p_stGetEngineObject(other)->h_StandardGame->ucMiscFlags >> 2) & 1) &&
                HIE_bIntersectSegmentWithFirstSuperObject(&origin, &direction, other,
                                                                 &contact, &normal, &element)) {
                facesUp = normal.xZ > 0.5f;
                found |= facesUp;
            }
        }
        inv_HIE_ulPickExcludeFlags = 0x80;
        if (HIE_bIntersectSegmentWithFirstSuperObject(&origin, &direction, sectorObject,
                                                             &contact, &normal, &element)) {
            facesUp = normal.xZ > 0.5f;
            found |= facesUp;
        }
        inv_HIE_ulPickExcludeFlags = 0x80;
        borders = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInActivityInteraction.hFirstElementOpt;
        for (i = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt; i > 0; i--, borders++) {
            borderObject = borders->hPointerOfSectorSO;
            border = inv_HIE_fn_p_stGetSector(borderObject);
            SECT_fn_vGetMinPointInBorder(border, &borderMinimum);
            SECT_fn_vGetMaxPointInBorder(border, &borderMaximum);
            if (INT_fn_bDetectIntersectSegmentWithBox(&origin, &direction, &borderMinimum,
                                                       &borderMaximum, NULL)) {
                for (node = inv_HIE_fn_p_stGetSector(borderObject)->stListOfCharacters.hFirstElementDyn; node != NULL;
                     node = node->hNextBrotherDyn) {
                    other = node->hPointerOfCharacter;
                    if ((inv_HIE_fn_p_stGetEngineObject(other)->h_3dData->shadow.control.flags & SHADOW_CASTER) &&
                        ((inv_HIE_fn_p_stGetEngineObject(other)->h_StandardGame->ucMiscFlags >> 2) & 1) &&
                        HIE_bIntersectSegmentWithFirstSuperObject(
                            &origin, &direction, other, &contact, &normal, &element)) {
                        facesUp = normal.xZ > 0.5f;
                        found |= facesUp;
                    }
                }
            }
        }
        inv_HIE_ulPickExcludeFlags = 0x80;
        objects = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInCollisionInteraction.hFirstElementOpt;
        for (i = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt; i > 0; i--, objects++) {
            if (HIE_bIntersectSegmentWithFirstSuperObject(&origin, &direction, objects->hPointerOfSectorSO,
                                                                 &contact, &normal, &element)) {
                facesUp = normal.xZ > 0.5f;
                found |= facesUp;
            }
        }
    }
    if (!found) {
        return;
    }
    if (normal.xZ > 0.5f) {
        inverseNormalZ = 1.0f / normal.xZ;
        POS_fn_vGetRotationVectors(matrix, &forward, &unused, &unused);
        forward.xZ = 0.0f;
        if (inv_MTH_fn_xAbsolute(forward.xX) < 0.1f && inv_MTH_fn_xAbsolute(forward.xY) < 0.1f) {
            forward.xX = 0.70710677f;
            forward.xY = 0.70710677f;
        }
        inv_MTH3D_fn_vNormalizeXYZ(&forward);
        one = 1;
        mode = g_cSinusEffectState;
        attenuation = 1.0f / ((origin.xZ - contact.xZ) * 0.3f + 1.0f);
        scaleX *= attenuation;
        scaleY *= attenuation;
        yScaledX = scaleY * forward.xX;
        yScaledY = scaleY * forward.xY;
        do {
            scaleY = scaleX * forward.xY;
            xScaledX = scaleX * forward.xX;
        } while (0);
        if ((mode == one && element->gu_lDrawModeMask >= 0) || (mode == 2 && element->gu_lDrawModeMask < 0) ||
            mode == 3) {
            contact.xZ += D_0026F4A4[0];
            if (origin.xZ - contact.xZ < 0.3f) {
                contact.xZ = origin.xZ - 0.3f;
            }
        }
        corners[0].xX = contact.xX + (xScaledX - yScaledY);
        corners[0].xY = contact.xY + (scaleY + yScaledX);
        contact.xZ += 0.03f;
        corners[0].xZ =
            inv_SHW_fn_xGetPlaneZAt(&contact, &normal, inverseNormalZ, corners[0].xX, corners[0].xY);
        inv_MTH3D_fn_vAddXYZ(&doubled, &contact, &contact);
        corners[1].xX = contact.xX + (xScaledX + yScaledY);
        corners[1].xY = contact.xY + (scaleY - yScaledX);
        inv_MTH3D_fn_vSubXYZ(&corners[2], &doubled, &corners[0]);
        corners[1].xZ =
            inv_SHW_fn_xGetPlaneZAt(&contact, &normal, inverseNormalZ, corners[1].xX, corners[1].xY);
        inv_MTH3D_fn_vSubXYZ(&corners[3], &doubled, &corners[1]);
        alpha = GLI_vGetGlobalAlpha();
        GLI_vSetGlobalAlpha(inv_fn_xGetSuperObjectTransparencyByte(superObject));
        GLI_vResetTableOfActiveLightsForChar(0);
        GLI_vDrawWorldQuadOpt(&corners[1], &corners[0], &corners[3], &corners[2], shadow->source.texture);
        GLI_vSetGlobalAlpha(alpha);
    }
}
