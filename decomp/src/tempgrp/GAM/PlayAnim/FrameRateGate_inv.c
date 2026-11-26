#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "AI/AI.h"

typedef struct FrameRateEntryList {
    void* entries;
    s16 count;
} FrameRateEntryList;

typedef struct GEO_tdstIndexedConeView {
    s16 xTopPoint;
    s16 xBasePoint;
    u8 pad04[4];
} GEO_tdstIndexedConeView;

typedef struct GEO_tdstElementConesView {
    GEO_tdstIndexedConeView* d_stListOfCones;
} GEO_tdstElementConesView;

void ANI_fn_vFreeze(HIE_tdstSuperObject_* hSuperObjPerso)
{
    tdstEngineObject_* object = hSuperObjPerso->hLinkedObject;
    gu_tdst3dData_* data = object->h_3dData;

    if (data->ucFrameRate != 0) {
        fn_v3dDataSetFrameRate(data, 0);
    }
}

void ANI_fn_vUnFreeze(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;
    gu_tdst3dData_* data = object->h_3dData;

    if (data->ucFrameRate != 0) {
        return;
    }
    fn_v3dDataSetFrameRate(data, (u8) data->h_CurrentState->scSpeedAnim);
    PLA_fn_vSetCurrFrame(superObject, data->uwCurrentFrame + 1);
}

FrameRateEntryList* inv_GAM_fn_p_stCreateFrameRateEntryList(s16 count)
{
    FrameRateEntryList* list = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(FrameRateEntryList));

    list->entries = AI_fn_p_vTrueAlloc(gp_6F74, count << 4);
    list->count = count;
    return list;
}

void inv_GEO_fn_vSetFaceOfElementIndexedTriangles(GeometricObjectView* hObject, s16 hElement, s16 xIndexOfFace, s16 xPoint1,
                   s16 xPoint2, s16 xPoint3)
{
    GeometricElement* element = hObject->elements[hElement];

    element->d_stListOfFacesTripled[xIndexOfFace].a3_xIndex[0] = xPoint1;
    element->d_stListOfFacesTripled[xIndexOfFace].a3_xIndex[1] = xPoint2;
    element->d_stListOfFacesTripled[xIndexOfFace].a3_xIndex[2] = xPoint3;
}

void inv_GEO_fn_vGetFaceOfElementIndexedTriangles(GeometricObjectView* hObject, s16 hElement, s16 xIndexOfFace, s16* p_xPoint1,
                   s16* p_xPoint2, s16* p_xPoint3)
{
    GeometricElement* element = hObject->elements[hElement];

    *p_xPoint1 = element->d_stListOfFacesTripled[xIndexOfFace].a3_xIndex[0];
    *p_xPoint2 = element->d_stListOfFacesTripled[xIndexOfFace].a3_xIndex[1];
    *p_xPoint3 = element->d_stListOfFacesTripled[xIndexOfFace].a3_xIndex[2];
}

void inv_GEO_fn_vSetElementVisualMaterial(GeometricObjectView* hObject, s16 hElement, GliMaterialData* hVisualMaterial)
{
    hObject->elements[hElement]->material = hVisualMaterial;
}

void inv_GEO_fn_vGetElementVisualMaterial(GeometricObjectView* hObject, s16 hElement, GliMaterialData** p_hVisualMaterial)
{
    *p_hVisualMaterial = hObject->elements[hElement]->material;
}

void inv_GEO_fn_vSetConeOfElementCones(GeometricObjectView* hObject, s16 hElement, s16 xNumberOfCone, s16 xTopPoint,
                   s16 xBasePoint)
{
    ((GEO_tdstElementConesView*) hObject->elements[hElement])->d_stListOfCones[xNumberOfCone].xTopPoint =
        xTopPoint;
    ((GEO_tdstElementConesView*) hObject->elements[hElement])->d_stListOfCones[xNumberOfCone].xBasePoint =
        xBasePoint;
}

GeometricObjectView* inv_GEO_fn_hCreateGeometricObject(s16 xNbPoints, s16 xNbElements)
{
    GeometricObjectView* object = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(GeometricObjectView));

    object->elementCount = xNbElements;
    object->vertexCount = xNbPoints;
    object->elementTypes = AI_fn_p_vTrueAlloc(gp_6F74, xNbElements * sizeof(s16));
    object->elements = AI_fn_p_vTrueAlloc(gp_6F74, xNbElements * sizeof(GeometricElement*));
    object->vertices = AI_fn_p_vTrueAlloc(gp_6F74, xNbPoints * sizeof(MTH3D_tdstVector_));
    return object;
}
