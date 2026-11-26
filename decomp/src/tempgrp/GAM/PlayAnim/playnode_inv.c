#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "PO/PO.h"
#include "renderware.h"

#define MORPH_GEOMETRY_LOCK_ALL 6

s32 RpGeometryGetNumVertices(void* geometry);
MTH3D_tdstVector_* RpMorphTargetGetVertices(void* morphTarget);
MTH3D_tdstVector_* RpMorphTargetGetVertexNormals(void* morphTarget);
tdstMorphChannelList_* inv_fn_p_stFindOrCreateMorphListElement(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* morphList, u16 id, s32 create);
void inv_MOR_fn_vResetMorphedObjectVertices(GeometricObjectView* object);
void inv_MOR_fn_vBlendMorphVertices(tdstMorphChannelList_* element);

s32 MOR_fn_bRemoveMorphedObjectFromHierarchy(HIE_tdstSuperObject_* superObject, void* linkedObject, void* replacement)
{
    HIE_tdstSuperObject_* child;

    if (superObject->hLinkedObject == linkedObject) {
        superObject->hLinkedObject = replacement;
        superObject->lTypeOfLinkedObject = replacement ? 8 : 1;
        return 1;
    }

    child = superObject->hFirstElementSta;
    while (child != NULL) {
        if (MOR_fn_bRemoveMorphedObjectFromHierarchy(child, linkedObject, replacement)) {
            return 1;
        }
        child = child->hNextBrotherSta;
    }
    return 0;
}

void inv_MOR_fn_vResetMorphedObjectVertices(GeometricObjectView* object)
{
    MTH3D_tdstVector_* vertices = object->vertices;
    MTH3D_tdstVector_* normals = object->normals;
    RW3ObjectView* rw3Object = object->rw3Object;
    u16** vertexIndices = rw3Object->vertexIndices;
    RpAtomic** atomics = rw3Object->atomics;
    MTH3D_tdstVector_* vertexOut;
    MTH3D_tdstVector_* normalOut;
    RpGeometry* geometry;
    RpMorphTarget* morphTarget;
    u16* index;
    s32 count;
    s32 i;
    s32 j;

    if (vertexIndices == NULL) {
        return;
    }
    for (i = 0; i < rw3Object->atomicCount; i++) {
        geometry = (*atomics)->geometry;
        morphTarget = RpGeometryGetMorphTarget(geometry, 0);
        if (morphTarget != NULL) {
            vertexOut = RpMorphTargetGetVertices(morphTarget);
            normalOut = RpMorphTargetGetVertexNormals(morphTarget);
            RpGeometryLock(geometry, MORPH_GEOMETRY_LOCK_ALL);
            count = RpGeometryGetNumVertices(geometry);
            index = *vertexIndices;
            for (j = 0; j < count; j++) {
                MTH3D_M_vCopyXYZ_inv(vertexOut, &vertices[*index]);
                index++;
                vertexOut++;
            }
            if (normalOut != NULL) {
                index = *vertexIndices;
                for (j = 0; j < count; j++) {
                    MTH3D_M_vCopyXYZ_inv(normalOut, &normals[*index]);
                    index++;
                    normalOut++;
                }
            }
            RpGeometryUnlock(geometry);
        }
        atomics++;
        vertexIndices++;
    }
}

s16 MOR_fn_xStopMorph(HIE_tdstSuperObject_* _hActorSuperObject, s16 _xChannelIndex)
{
    tdstEngineObject_* object = _hActorSuperObject->hLinkedObject;
    gu_tdst3dData_* data = object->h_3dData;
    LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* morphList = data->hMorphChannelList;
    tdstMorphChannelList_* element = inv_fn_p_stFindOrCreateMorphListElement(morphList, _xChannelIndex, 0);

    if (element == NULL) {
        return -1;
    }
    inv_MOR_fn_vResetMorphedObjectVertices(element->to->_hVisualSet);
    fn_vRemoveChannelFromMorphList(morphList, element);
    return 0;
}

tdstMorphChannelList_* inv_MOR_fn_p_stStartMorph(HIE_tdstSuperObject_* superObject, s16 fromIndex, s16 toIndex,
                            s32 level, s16 id)
{
    tdstEngineObject_* object = superObject->hLinkedObject;
    gu_tdst3dData_* data = object->h_3dData;
    tdstObjectsTablesList_* objectTable = data->h_CurrentObjectsTable;
    ObjectTableEntry* entries = objectTable->d_stObjectsTable;
    PO_tdstPhysicalObject_* from = (PO_tdstPhysicalObject_*) entries[fromIndex].stElement.h_Target;
    PO_tdstPhysicalObject_* to = (PO_tdstPhysicalObject_*) entries[toIndex].stElement.h_Target;
    GeometricObjectView* fromObject = from->_hVisualSet;
    GeometricObjectView* toObject = to->_hVisualSet;
    tdstMorphChannelList_* element;
    f32 percent;

    if (fromObject == toObject) {
        return NULL;
    }
    if (fromObject->vertexCount != toObject->vertexCount) {
        return NULL;
    }

    element = inv_fn_p_stFindOrCreateMorphListElement(data->hMorphChannelList, id, 1);
    element->from = from;
    element->to = to;
    percent = 100.0f;
    element->blend = level / percent;
    return element;
}

void inv_MOR_fn_vBlendMorphVertices(tdstMorphChannelList_* element)
{
    f32 weight;
    GeometricObjectView* to;
    GeometricObjectView* from;
    RW3ObjectView* rw3Object;
    RpAtomic** atomics;
    u16** vertexIndices;
    u16* index;
    MTH3D_tdstVector_* fromVertices;
    MTH3D_tdstVector_* toVertices;
    MTH3D_tdstVector_* fromNormals;
    MTH3D_tdstVector_* toNormals;
    MTH3D_tdstVector_* vertexOut;
    MTH3D_tdstVector_* normalOut;
    MTH3D_tdstVector_* target;
    MTH3D_tdstVector_* source;
    RpGeometry* geometry;
    RpMorphTarget* morphTarget;
    s32 count;
    s32 i;
    s32 k;

    weight = element->blend;
    if (weight <= 0.0f) {
        weight = 0.0f;
    } else if (weight >= 1.0f) {
        weight = 1.0f;
    }
    to = element->to->_hVisualSet;
    from = element->from->_hVisualSet;
    toVertices = to->vertices;
    toNormals = to->normals;
    fromVertices = from->vertices;
    fromNormals = from->normals;
    rw3Object = to->rw3Object;
    atomics = rw3Object->atomics;
    vertexIndices = rw3Object->vertexIndices;
    if (vertexIndices == NULL) {
        return;
    }
    for (i = 0; i < rw3Object->atomicCount; atomics++, vertexIndices++, i++) {
        geometry = (*atomics)->geometry;
        morphTarget = RpGeometryGetMorphTarget(geometry, 0);
        if (morphTarget == NULL) {
            continue;
        }
        vertexOut = RpMorphTargetGetVertices(morphTarget);
        normalOut = RpMorphTargetGetVertexNormals(morphTarget);
        RpGeometryLock(geometry, MORPH_GEOMETRY_LOCK_ALL);
        count = RpGeometryGetNumVertices(geometry);
        index = *vertexIndices;
        for (k = 0; k < count; k++, vertexOut++) {
            source = &fromVertices[*index];
            target = &toVertices[*index];
            index++;
            MTH3D_M_vSetXYZ_inv(vertexOut, (target->xX - source->xX) * weight + source->xX,
                                (target->xY - source->xY) * weight + source->xY,
                                (target->xZ - source->xZ) * weight + source->xZ);
        }
        if (normalOut != NULL) {
            index = *vertexIndices;
            for (k = 0; k < count; k++, normalOut++) {
                source = &fromNormals[*index];
                target = &toNormals[*index];
                index++;
                MTH3D_M_vSetXYZ_inv(normalOut, (target->xX - source->xX) * weight + source->xX,
                                    (target->xY - source->xY) * weight + source->xY,
                                    (target->xZ - source->xZ) * weight + source->xZ);
            }
        }
        RpGeometryUnlock(geometry);
    }
}

void inv_MOR_fn_vStopAllMorphsInHierarchy(HIE_tdstSuperObject_* superObject)
{
    HIE_tdstSuperObject_* child;
    HIE_tdstSuperObject_* nextChild;
    gu_tdst3dData_* data;
    LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* morphList;
    tdstMorphChannelList_* element;
    tdstMorphChannelList_* nextElement;

    for (child = superObject->hFirstElementSta,
        nextChild = HIE_fn_hForceGetSuperObjectNextBrother(child);
         child != NULL;
         child = nextChild, nextChild = HIE_fn_hForceGetSuperObjectNextBrother(child)) {
        if (child->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv &&
            (data = ((tdstEngineObject_*) child->hLinkedObject)->h_3dData) != NULL &&
            (morphList = data->hMorphChannelList) != NULL) {
            for (element = morphList->hFirstElementDyn,
                nextElement = element ? element->hNextBrotherDyn : NULL;
                 element != NULL;
                 element = nextElement, nextElement = element ? element->hNextBrotherDyn : NULL) {
                MOR_fn_bRemoveMorphedObjectFromHierarchy(child, element->from, element->to);
                fn_vRemoveChannelFromMorphList(morphList, element);
            }
        }
        inv_MOR_fn_vStopAllMorphsInHierarchy(child);
    }
}

void* inv_MOR_fn_p_vGetMorphTarget(tdstMorphChannelList_* owner)
{
    return owner->to;
}

void inv_MOR_fn_vUpdateMorph(tdstMorphChannelList_* element)
{
    inv_MOR_fn_vBlendMorphVertices(element);
}
