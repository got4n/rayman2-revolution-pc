#include "common.h"

#include "CAM/CAM.h"
#include "HIE/HIE.h"
#include "GAM/GAM.h"

#define CAMERA_DRAW_MODE_OPAQUE 8

void func_00108088(tdstCineinfo_* cineinfo);

static inline f32 inv_GAM_fn_xMaxFloat(f32 a, f32 b)
{
    if (a < b) {
        return b;
    }
    return a;
}

void inv_CAM_fn_vInitTransparentSuperObjectArray(tdstCineinfo_* cineinfo)
{
    tdstTransparentSuperObjectForCamera_* object = cineinfo->a10_stTransparentSuperObjectForCamera;
    tdstTransparentSuperObjectForCamera_* end = &cineinfo->a10_stTransparentSuperObjectForCamera[CAMERA_TRANSPARENT_OBJECT_COUNT];

    for (; object < end; object++) {
        object->hSuperObject = NULL;
        object->lInitialDrawMask = 0;
        object->fInitialTransparenceLevel = 0.0f;
        object->fCurrentTransparencyLevel = 255.0f;
        object->bIncrease = 0;
    }
    cineinfo->xNumberOfTransparentSuperObjectForCamera = 0;
}

void CAM_fn_vResetArrayOfTransparentSuperObject(tdstCineinfo_* _hCineinfo)
{
    tdstTransparentSuperObjectForCamera_* object = _hCineinfo->a10_stTransparentSuperObjectForCamera;
    tdstTransparentSuperObjectForCamera_* end = &object[_hCineinfo->xNumberOfTransparentSuperObjectForCamera];

    for (; object < end; object++) {
        object->bIncrease = 0;
    }
}

void inv_CAM_fn_vMarkSuperObjectForTransparency(tdstCineinfo_* cineinfo,
                                                HIE_tdstSuperObject_* superObject)
{
    tdstTransparentSuperObjectForCamera_* object = cineinfo->a10_stTransparentSuperObjectForCamera;
    tdstTransparentSuperObjectForCamera_* end = &object[cineinfo->xNumberOfTransparentSuperObjectForCamera];
    tdstTransparentSuperObjectForCamera_* freeSlot = NULL;
    s32 found = 0;

    while (object < end) {
        if (object->hSuperObject == superObject) {
            object->bIncrease = 1;
            found = 1;
            break;
        }
        if (freeSlot == NULL && object->hSuperObject == NULL)
            freeSlot = object;
        object++;
    }
    if (!found) {
        if (freeSlot == NULL)
            freeSlot = &cineinfo->a10_stTransparentSuperObjectForCamera[cineinfo->xNumberOfTransparentSuperObjectForCamera++];
        freeSlot->fCurrentTransparencyLevel = 255.0f;
        freeSlot->hSuperObject = superObject;
        freeSlot->bIncrease = 1;
    }
}

void func_00108088(tdstCineinfo_* cineinfo)
{
    tdstTransparentSuperObjectForCamera_* objects = cineinfo->a10_stTransparentSuperObjectForCamera;
    tdstTransparentSuperObjectForCamera_* hole = objects;
    tdstTransparentSuperObjectForCamera_* last = &objects[cineinfo->xNumberOfTransparentSuperObjectForCamera];
    tdstTransparentSuperObjectForCamera_* object;
    HIE_tdstSuperObject_* lastSuperObject;
    s32 more = hole < last;

    do {
        lastSuperObject = last->hSuperObject;
        if (hole->hSuperObject != NULL && more) {
            do {
                hole++;
                more = hole < last;
                if (hole->hSuperObject == NULL) {
                    break;
                }
            } while (more);
        }
        if (lastSuperObject == NULL) {
            if (!more) {
                break;
            }
            do {
                last--;
                more = hole < last;
                if (last->hSuperObject != NULL) {
                    break;
                }
            } while (more);
        }
        if (!more) {
            break;
        }
        *hole = *last;
        last->hSuperObject = NULL;
        hole++;
        last--;
    } while ((more = hole < last));

    object = objects;
    cineinfo->xNumberOfTransparentSuperObjectForCamera = 0;
    if (cineinfo->a10_stTransparentSuperObjectForCamera[0].hSuperObject != NULL) {
        do {
            object++;
            cineinfo->xNumberOfTransparentSuperObjectForCamera++;
        } while (object->hSuperObject != NULL);
    }
}

void CAM_fn_vApplyTransparencyOnSuperObject(tdstCineinfo_* _hCineinfo)
{
    tdstTransparentSuperObjectForCamera_* object = _hCineinfo->a10_stTransparentSuperObjectForCamera;
    tdstTransparentSuperObjectForCamera_* end = object + _hCineinfo->xNumberOfTransparentSuperObjectForCamera;
    HIE_tdstSuperObject_* superObject;
    f32 step;
    f32 level;
    f32 opaque;
    s32 removed;

    step = (f32) inv_fn_ulGetEngineClock() * _hCineinfo->fTransparencySpeed;
    removed = 0;
    for (; object < end; object++) {
        opaque = 255.0f;
        superObject = object->hSuperObject;
        if (superObject == NULL) {
            continue;
        }
        if (object->bIncrease) {
            object->fCurrentTransparencyLevel =
                inv_GAM_fn_xMaxFloat(_hCineinfo->fTransparency, object->fCurrentTransparencyLevel - step);
        } else {
            level = object->fCurrentTransparencyLevel + step;
            if (!(level < opaque)) {
                level = 255.0f;
            }
            object->fCurrentTransparencyLevel = level;
            if (level >= opaque) {
                object->hSuperObject = NULL;
                removed = 1;
                continue;
            }
        }
        object->lInitialDrawMask = superObject->gu_lDrawModeMask;
        object->fInitialTransparenceLevel = superObject->gu_fTransparenceLevel;
        superObject->gu_lDrawModeMask &= ~CAMERA_DRAW_MODE_OPAQUE;
        object->hSuperObject->gu_fTransparenceLevel = object->fCurrentTransparencyLevel;
    }
    if (removed) {
        func_00108088(_hCineinfo);
    }
}

void CAM_fn_vRestoreTransparencyOnSuperObject(tdstCineinfo_* _hCineinfo)
{
    tdstTransparentSuperObjectForCamera_* object = _hCineinfo->a10_stTransparentSuperObjectForCamera;
    tdstTransparentSuperObjectForCamera_* end = &object[_hCineinfo->xNumberOfTransparentSuperObjectForCamera];

    for (; object < end; object++) {
        if (object->hSuperObject != NULL) {
            object->hSuperObject->gu_lDrawModeMask = object->lInitialDrawMask;
            object->hSuperObject->gu_fTransparenceLevel = object->fInitialTransparenceLevel;
        }
    }
}
