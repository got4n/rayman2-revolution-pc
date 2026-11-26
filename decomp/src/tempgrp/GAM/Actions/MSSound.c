#include "common.h"

#include "GAM/GAM.h"
#include "snd/snd.h"
#include "HIE/HIE.h"
#include "AI/AI.h"

RevoMSSoundAttenuation* fn_h_MSSoundRealAlloc(void);

void fn_vMSSoundAlloc(tdstEngineObject_* object)
{
    object->h_MSSound = fn_h_MSSoundRealAlloc();
}

RevoMSSoundAttenuation* fn_h_MSSoundRealAlloc(void)
{
    return AI_fn_p_vTrueAlloc(gp_6F74, sizeof(RevoMSSoundAttenuation));
}

void fn_vMSSoundInitValueSAI(RevoMSSoundAttenuation* sound);

void fn_vMSSoundInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    if (eObjectInit == 1)
        fn_vMSSoundInitValueSAI(p_stObject->h_MSSound);
}

void fn_vMSSoundDesinit(void)
{
}

void fn_vMSSoundCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    *_p_stObjectDest->h_MSSound = *_p_stObjectSrc->h_MSSound;
}

s32 fn_xMSSoundGetSaturationDistance(RevoMSSoundAttenuation* h_MSSound)
{
    return h_MSSound->saturationDistance;
}

void fn_vMSSoundSetSaturationDistance(RevoMSSoundAttenuation* h_MSSound, s32 _xSaturationDistance)
{
    h_MSSound->saturationDistance = _xSaturationDistance;
}

s32 fn_xMSSoundGetBackGroundDistance(RevoMSSoundAttenuation* h_MSSound)
{
    return h_MSSound->backgroundDistance;
}

void fn_vMSSoundSetBackGroundDistance(RevoMSSoundAttenuation* h_MSSound, s32 _xBackGroundDistance)
{
    h_MSSound->backgroundDistance = _xBackGroundDistance;
}

s32 fn_bRetRollOffFactor(HIE_tdstSuperObject_* superObject, u32* distances)
{
    tdstEngineObject_* object;
    RevoMSSoundAttenuation* sound;

    if ((u32) superObject + 1 < 2)
        return 0;
    object = superObject->hLinkedObject;
    sound = object->h_MSSound;
    if (sound == NULL)
        return 0;
    distances[0] = sound->saturationDistance;
    distances[1] = sound->backgroundDistance;
    return 1;
}

void fn_vMSSoundInitValueSAI(RevoMSSoundAttenuation* sound)
{
}
