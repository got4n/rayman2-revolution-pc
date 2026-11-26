#include "common.h"

#include "COL/COL.h"

static inline COL_tdstElementSpheres_* inv_GEO_fn_p_stSpheres(COL_tdstCollideObject_* object, s16 element)
{
    return object->d_stListOfElements[element];
}

static inline COL_tdstElementAlignedBoxes_* inv_GEO_fn_p_stAlignedBoxes(COL_tdstCollideObject_* object, s16 element)
{
    return object->d_stListOfElements[element];
}

GMT_tdstGameMaterial_* GEO_hGetGameMaterialOfIndexedSphere(COL_tdstCollideObject_* object, s16 element, s16 sphere)
{
    return inv_GEO_fn_p_stSpheres(object, element)->d_stListOfSpheres[sphere].hMaterial;
}

s32 GEO_lGetIndexedSphereNbSpheres(COL_tdstCollideObject_* object, s16 element)
{
    return inv_GEO_fn_p_stSpheres(object, element)->xNbSpheres;
}

void GEO_vSetRadiusOfIndexedSphere(COL_tdstCollideObject_* hObject, s16 hElement, s16 xNumberOfSphere, f32 xRadius)
{
    inv_GEO_fn_p_stSpheres(hObject, hElement)->d_stListOfSpheres[xNumberOfSphere].xRadius = xRadius;
}

void GEO_vGetMinMaxPointOfIndexedAlignedBox(COL_tdstCollideObject_* object, s16 element, s16 box,
                                            u16* minimum, u16* maximum)
{
    *minimum = inv_GEO_fn_p_stAlignedBoxes(object, element)->d_stListOfAlignedBoxes[box].xMinPoint;
    *maximum = inv_GEO_fn_p_stAlignedBoxes(object, element)->d_stListOfAlignedBoxes[box].xMaxPoint;
}

GMT_tdstGameMaterial_* GEO_vGetGameMaterialOfIndexedAlignedBox(COL_tdstCollideObject_* object, s16 element, s16 box)
{
    return inv_GEO_fn_p_stAlignedBoxes(object, element)->d_stListOfAlignedBoxes[box].hMaterial;
}

s32 GEO_lGetAlignedBoxNbBoxes(COL_tdstCollideObject_* object, s16 element)
{
    return inv_GEO_fn_p_stAlignedBoxes(object, element)->xNbAlignedBoxes;
}
