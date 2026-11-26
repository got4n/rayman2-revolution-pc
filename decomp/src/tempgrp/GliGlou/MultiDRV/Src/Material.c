#include "common.h"

#include "GLI/GLI.h"
#include "PO/PO.h"

#include "ps2/libc.h"
#include "ps2/r5900.h"
#include "renderware.h"

#include "include_asm.h"
#include "AI/AI.h"
#include "GAM/GAM.h"
#include "AGO/AGO.h"

#define MATERIAL_TYPE_TEXTURED 0x400
#define MATERIAL_TYPE_NO_BACKFACE 0x400000

#define MATERIAL_ANIMATION_LOCKED 0x1
#define MATERIAL_ANIMATION_ENABLED 0x2

#define TEXTURE_FLAG_08 0x8
#define TEXTURE_FLAG_20 0x20
#define TEXTURE_FLAG_100 0x100
#define TEXTURE_FLAG_200 0x200

extern f32 gs_fCoefOfMultiplicationForScrolledTextures;
extern f32 gs_fCoefOfMultiplicationForAnimatedTexture;

void inv_GLI_vSetMaterialAnimationEnabled(GliMaterialData* material, s32 enabled);
void inv_GLI_vInitMaterialLayers(GliMaterialData* material);
void GLI_xSetMaterialTexture(GliMaterialData* material, GliTexture* texture);
void GLI_xGetMaterialType(GliMaterialData* material, u32* type);

void GLI_xCreateMaterial(GliMaterialData** hMaterial)
{
    GliMaterialData* material;
    f32 initialValue;

    material = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(GliMaterialData));
    initialValue = 0.8f;
    *hMaterial = material;
    (*hMaterial)->color.xR = initialValue;
    (*hMaterial)->color.xG = initialValue;
    (*hMaterial)->color.xB = initialValue;
    (*hMaterial)->type = -1;
    (*hMaterial)->animatedFirst = NULL;
    (*hMaterial)->animatedCurrent = NULL;
    (*hMaterial)->animatedCount = 0;
    (*hMaterial)->time = gs_fCoefOfMultiplicationForAnimatedTexture;
    (*hMaterial)->frame = gs_lActualRefreshNumberForScolledTextures;
    GLI_vUnLockAnimatedTexture(*hMaterial);
    inv_GLI_vSetMaterialAnimationEnabled(*hMaterial, 1);
    inv_GLI_vInitMaterialLayers(*hMaterial);
}

void GLI_xDuplicateMaterial(GliMaterialData** copy, GliMaterialData* source)
{
    GEO_tdstColor_ color;
    GliTexture* texture;
    u32 type;

    GLI_xCreateMaterial(copy);
    **copy = *source;
    GLI_xGetMaterialAmbientCoef(source, &color);
    GLI_xSetMaterialAmbientCoef(*copy, &color);
    GLI_xGetMaterialColor(source, &color);
    GLI_xSetMaterialColor(*copy, &color);
    GLI_xGetMaterialTexture(source, &texture);
    GLI_xSetMaterialTexture(*copy, texture);
    GLI_xGetMaterialType(source, &type);
    GLI_xSetMaterialType(*copy, type);
}

void GLI_xSetMaterialAmbientCoef(GliMaterialData* hMaterial, GEO_tdstColor_* p_stColor)
{
}

void GLI_xGetMaterialAmbientCoef(GliMaterialData* hMaterial, void* p_stColor)
{
}

void GLI_xSetMaterialColor(GliMaterialData* hMaterial, GEO_tdstColor_* p_stColor)
{
    u8 i;

    hMaterial->color = *p_stColor;
    for (i = 0; i < MATERIAL_LAYER_COUNT; i++) {
        inv_RW3_fn_vSetRpMaterialColor(hMaterial->layers[i].rpMaterial, p_stColor);
    }
}

void GLI_xGetMaterialColor(const GliMaterialData* hMaterial, GEO_tdstColor_* p_stColor)
{
    *p_stColor = hMaterial->color;
}

void GLI_xGetMaterialType(GliMaterialData* material, u32* type)
{
    *type = material->type;
}

void GLI_xSetMaterialType(GliMaterialData* hMaterial, u32 xMType)
{
    hMaterial->type = xMType;
}

void inv_GLI_vSetMaterialBackfaceFlag(GliMaterialData* material, s32 enabled)
{
    if (enabled != 0) {
        material->type &= ~MATERIAL_TYPE_NO_BACKFACE;
    } else {
        material->type |= MATERIAL_TYPE_NO_BACKFACE;
    }
}

s32 func_00119EA8(const GliMaterialData* material)
{
    return (((s32) material->type >> 26) ^ 1) & 1;
}

void GLI_xSetMaterialTexture(GliMaterialData* hMaterial, GliTexture* p_stTexture)
{
    hMaterial->layers[0].texture = p_stTexture;
    hMaterial->layers[0].unk06 = 0;
    if (p_stTexture != NULL) {
        if (p_stTexture->flags & TEXTURE_FLAG_08) {
            hMaterial->type &= ~MATERIAL_TYPE_TEXTURED;
        } else if (p_stTexture->flags & TEXTURE_FLAG_100) {
            hMaterial->layers[0].mode = 0;
        }
        if (p_stTexture->flags & TEXTURE_FLAG_08) {
            hMaterial->layers[0].mode = 1;
        } else if (p_stTexture->flags & TEXTURE_FLAG_20) {
            hMaterial->layers[0].mode = 2;
        } else if (p_stTexture->flags & TEXTURE_FLAG_200) {
            hMaterial->layers[0].mode = 1;
        } else {
            hMaterial->layers[0].mode = 0;
        }
    } else {
        hMaterial->layers[0].mode = 0;
    }
    if (p_stTexture != NULL && p_stTexture->rwTexture != NULL) {
        RpMaterialSetTexture(hMaterial->layers[0].rpMaterial, p_stTexture->rwTexture);
    }
}

void GLI_xGetMaterialTexture(GliMaterialData* material, GliTexture** texture)
{
    *texture = material->layers[0].texture;
}

void _GLI_xSetMaterialTextureScrollingCoef(GliMaterialData* hMaterial, s32 lUnused, s32 bSwitch,
                                           f32 fUSpeed, f32 fVSpeed)
{
    if (bSwitch != 0) {
        if (fUSpeed != 0.0f) {
            hMaterial->layers[0].flags |= MATERIAL_SCROLL_U;
            hMaterial->layers[0].scrollU = fUSpeed;
        }
        if (fVSpeed != 0.0f) {
            hMaterial->layers[0].flags |= MATERIAL_SCROLL_V;
            hMaterial->layers[0].scrollV = fVSpeed;
        }
    }
}

void GLI_xSetMaterialTextureScrollingCoef(GliMaterialData* hMaterial, s32 lUnused, s32 bSwitch,
                                          f32 fUSpeed, f32 fVSpeed)
{
    if (bSwitch != 0) {
        hMaterial->layers[0].constantScrollU = fUSpeed;
        hMaterial->layers[0].scrollU = fUSpeed;
        hMaterial->layers[0].constantScrollV = fVSpeed;
        hMaterial->layers[0].scrollV = fVSpeed;
        if (fUSpeed == 0.0f && fVSpeed == 0.0f) {
            return;
        }
        hMaterial->layers[0].flags |= MATERIAL_SCROLL_CONSTANT;
    }
}

void GLI_xGetMaterialTextureScrollingCoef(GliMaterialData* material, s32 layerIndex, s32* mode, f32* scrollU, f32* scrollV)
{
    u8 flags;

    *scrollU = material->layers[0].scrollU;
    *scrollV = material->layers[0].scrollV;
    flags = material->layers[0].flags;
    if (flags & (MATERIAL_SCROLL_U | MATERIAL_SCROLL_V)) {
        *mode = 1;
        return;
    }
    if (flags & MATERIAL_SCROLL_CONSTANT) {
        *mode = 2;
        return;
    }
    *mode = 0;
}

void GLI_xSetMaterialTextureScrollingOnOff(GliMaterialData* hMaterial, s32 bSwitch)
{
    if (bSwitch == 0 || hMaterial->layers[0].scrollU != 0.0f || hMaterial->layers[0].scrollV != 0.0f) {
        switch (bSwitch) {
        case 0:
            hMaterial->layers[0].flags &= ~MATERIAL_SCROLL_ANY;
            break;
        case 1:
            hMaterial->layers[0].flags &= ~MATERIAL_SCROLL_CONSTANT;
            break;
        case 2:
            hMaterial->layers[0].flags &= ~(MATERIAL_SCROLL_U | MATERIAL_SCROLL_V);
            break;
        }
    }
}

void inv_GLI_p_stAllocAnimatedTextureElement(AnimatedTextureElement** handle)
{
    AnimatedTextureElement* element;

    element = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(AnimatedTextureElement));
    *handle = element;
    element->texture = NULL;
    (*handle)->xDisplayTime = 0;
    (*handle)->next = NULL;
}

void inv_GLI_p_stAllocAnimatedTextureElement(AnimatedTextureElement** handle);

void inv_GLI_vInsertAnimatedTextureElement(GliMaterialData* material, s32 position,
                                           GliTexture* texture, f32 value)
{
    AnimatedTextureElement* element;
    AnimatedTextureElement* current;
    AnimatedTextureElement* previous;

    material->animatedCount++;
    inv_GLI_p_stAllocAnimatedTextureElement(&element);
    element->texture = texture;
    element->xDisplayTime = value;
    if (material->animatedCount == 1 || position == 0) {
        GLI_xSetMaterialTexture(material, texture);
        element->next = material->animatedFirst;
        material->animatedFirst = element;
        material->animatedCurrent = element;
        return;
    }
    previous = current = material->animatedFirst;
    if (current != NULL) {
        position--;
        do {
            previous = current;
            current = current->next;
        } while (current != NULL && position-- != 0);
    }
    element->next = current;
    previous->next = element;
}

u16 GLI_lGetMaterialNumberOfAnimatedTexture(GliMaterialData* hMaterial)
{
    return hMaterial->animatedCount;
}

s32 GLI_lGetCurrentAnimatedTextureNumber(GliMaterialData* hMaterial)
{
    AnimatedTextureElement* current = hMaterial->animatedCurrent;
    AnimatedTextureElement* element = hMaterial->animatedFirst;
    AnimatedTextureElement* end;
    s32 number = 1;

    if (element != current) {
        R5900_MMI_UN("move", end, current);
        do {
            element = element->next;
            number++;
        } while (element != end);
    }
    return element == current ? number : 0;
}

void GLI_vSetCurrentAnimatedTextureNumber(GliMaterialData* hMaterial, u32 lTextureNumber)
{
    AnimatedTextureElement* first = hMaterial->animatedFirst;
    AnimatedTextureElement* element = first;
    u32 number = 1;

    while (element != NULL && number != lTextureNumber) {
        element = element->next;
        number++;
    }
    if (element == NULL) {
        element = first;
    }
    hMaterial->animatedCurrent = element;
    if (element == NULL) {
        return;
    }
    GLI_xSetMaterialTexture(hMaterial, element->texture);
}

void GLI_vLockAnimatedTexture(GliMaterialData* _hMaterial)
{
    _hMaterial->animationFlags |= MATERIAL_ANIMATION_LOCKED;
}

void GLI_vUnLockAnimatedTexture(GliMaterialData* _hMaterial)
{
    _hMaterial->animationFlags &= ~MATERIAL_ANIMATION_LOCKED;
}

void inv_GLI_vSetMaterialAnimationEnabled(GliMaterialData* material, s32 enabled)
{
    if (enabled) {
        material->animationFlags |= MATERIAL_ANIMATION_ENABLED;
    } else {
        material->animationFlags &= ~MATERIAL_ANIMATION_ENABLED;
    }
}

s32 inv_GLI_bIsMaterialAnimationEnabled(GliMaterialData* material)
{
    return material->animationFlags & MATERIAL_ANIMATION_ENABLED;
}

void GLI_vRefreshAllCyclingTextures(f32 fCoefOfMultiplication)
{
    gs_fCoefOfMultiplicationForScrolledTextures = fCoefOfMultiplication;
    gs_fCoefOfMultiplicationForAnimatedTexture += fCoefOfMultiplication;
    gs_lActualRefreshNumberForScolledTextures++;
}

static inline f32 inv_GLI_fn_xMul(f32 a, f32 b)
{
    return a * b;
}

void GLI_xRefreshAnimatedTexture(GliMaterialData* hMaterial)
{
    AnimatedTextureElement* element;
    u8* table;
    GliDisplayStateView* speedTable;
    f32 duration;
    f32 scale;
    f32 limit;
    u32 type;

    if (hMaterial->frame != gs_lActualRefreshNumberForScolledTextures && hMaterial->layers[0].texture != NULL &&
        hMaterial->animatedCount >= 2 && (hMaterial->animationFlags & MATERIAL_ANIMATION_LOCKED) == 0) {
        g_bHasTextureAnimationFinished = 0;
        if (g_bTakeCareOfAnimationCoordination != 0) {
            element = hMaterial->animatedFirst;
            duration = g_AGO_AnimTimeScale;
            table = (u8*) &inv_GLI_stDisplayState;
            hMaterial->animatedCurrent = element;
            do {
                speedTable = (GliDisplayStateView*) table;
                scale = duration;
            } while (0);
            limit = gs_fCoefOfMultiplicationForAnimatedTexture;
            duration = inv_GLI_fn_xMul(element->xDisplayTime * ((GliDisplayStateView*) table)->frameRate, duration);
        loop:
            if (duration < limit - g_BaseTimeForParticleAnimation) {
                table = (u8*) hMaterial->animatedCurrent->next;
                g_BaseTimeForParticleAnimation = g_BaseTimeForParticleAnimation + duration;
                if (table != NULL) {
                    hMaterial->animatedCurrent = (AnimatedTextureElement*) table;
                    duration = (((AnimatedTextureElement*) table)->xDisplayTime * speedTable->frameRate) * scale;
                    goto loop;
                }
                if (g_bOneShotAnim == 0) {
                    hMaterial->animatedCurrent = hMaterial->animatedFirst;
                } else {
                    g_bHasTextureAnimationFinished = 1;
                }
                goto refresh;
            }
        } else {
            element = hMaterial->animatedCurrent;
            duration = element->xDisplayTime;
            if (0.0f < duration) {
                table = (u8*) ((u32) &inv_GLI_stDisplayState);
                scale = gs_fCoefOfMultiplicationForAnimatedTexture;
                limit = hMaterial->time;
                duration = duration * ((GliDisplayStateView*) table)->frameRate;
                if (duration < scale - limit) {
                    speedTable = (GliDisplayStateView*) table;
                    do {
                        hMaterial->time += duration;
                        if (hMaterial->animatedCurrent->next) {
                            hMaterial->animatedCurrent = hMaterial->animatedCurrent->next;
                        } else {
                            hMaterial->animatedCurrent = hMaterial->animatedFirst;
                        }
                        table = (u8*) hMaterial->animatedCurrent;
                        duration = speedTable->frameRate;
                        limit = hMaterial->time;
                        duration = inv_GLI_fn_xMul(((AnimatedTextureElement*) table)->xDisplayTime, duration);
                    } while (duration < scale - limit);
                }
            }
        }
    refresh:
        GLI_xGetMaterialType(hMaterial, &type);
        GLI_xSetMaterialTexture(hMaterial, hMaterial->animatedCurrent->texture);
        GLI_xSetMaterialType(hMaterial, type);
    }
    if (g_bTakeCareOfAnimationCoordination == 0) {
        hMaterial->frame = gs_lActualRefreshNumberForScolledTextures;
    }
}

void inv_GLI_vInitMaterialLayers(GliMaterialData* material)
{
    MaterialLayer* layer;

    memset(material->layers, 0, sizeof(material->layers));
    for (layer = material->layers; layer < &material->layers[MATERIAL_LAYER_COUNT]; layer++) {
        layer->rpMaterial = inv_RW3_fn_pCreateDefaultRpMaterial();
    }
}

GliMaterialData* GLI_pGetVisualMaterialForLetter(u32 letter)
{
    tdstObjectsTablesList_* letters;
    PO_tdstPhysicalObject_* physicalObject;
    GeometricObjectView* object;

    if (g_AlphabetCharacterPointer == NULL) {
        return NULL;
    }
    letters = g_AlphabetCharacterPointer->h_3dData->h_CurrentObjectsTable;
    if (letters->wNumberOfElement < letter) {
        return NULL;
    }
    physicalObject = (PO_tdstPhysicalObject_*) letters->d_stObjectsTable[letter].stElement.h_Target;
    if (physicalObject == NULL) {
        return NULL;
    }
    object = physicalObject->_hVisualSet;
    if (object == NULL) {
        return NULL;
    }
    if (object->elementCount == 1 && object->elementTypes[0] == object->elementCount) {
        return object->elements[0]->material;
    }
    return NULL;
}
