#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "AI/AI.h"
#include "snd/snd.h"

#define ALL_CHANNELS 0xFFFF
#define CHANNEL_SUPER_OBJECT_FLAGS 0x83U

tdstMorphChannelList_* fn_p_stCreateMorphListElement(void);
s32 fn_vFreeMorphListElement(tdstMorphChannelList_* element);
s32 inv_MEM_fn_lFreeIntView_inv(MemoryHeapView* heap, void* block);

static inline HIE_tdstSuperObject_* inv_GAM_fn_p_stChannelSuperObjectOf(tdstAChannel_* record)
{
    HIE_tdstSuperObject_* superObject = record->hZoomSupObject;

    if (superObject == NULL) {
        superObject = record->hSupObject;
    }
    return superObject;
}

static inline void inv_GAM_fn_vInsertElementBefore(tdstMorphChannelList_* element,
                                       tdstMorphChannelList_* position)
{
    element->hNextBrotherDyn = position;
    element->hPrevBrotherDyn = position->hPrevBrotherDyn;
    if (position->hPrevBrotherDyn != NULL) {
        position->hPrevBrotherDyn->hNextBrotherDyn = element;
        element->hFatherDyn = position->hPrevBrotherDyn->hFatherDyn;
    } else {
        position->hFatherDyn->hFirstElementDyn = element;
    }
    if (position != NULL) {
        position->hPrevBrotherDyn = element;
        element->hFatherDyn = position->hFatherDyn;
    } else {
        position->hPrevBrotherDyn->hFatherDyn->hLastElementDyn = element;
    }
    element->hFatherDyn->lNumberOfElementsDyn++;
}

void fn_vInitCSOList(tdstAChannel_** p_hChannelArray, u32 ulNumberOfChannels)
{
    u32 ulI;

    *p_hChannelArray = AI_fn_p_vTrueAlloc(gp_6F74, ulNumberOfChannels * sizeof(tdstAChannel_));
    for (ulI = 0; ulI < ulNumberOfChannels; ulI++) {
        (*p_hChannelArray)[ulI].bActiveChannel = 1;
        (*p_hChannelArray)[ulI].bControlledChannel = 0;
    }
}

s32 fn_vFreeCSOList(tdstAChannel_** p_hChannelArray, u32 ulNumberOfChannels)
{
    tdstAChannel_* hArray = *p_hChannelArray;

    if (hArray != NULL) {
        tdstAChannel_* end = hArray + ulNumberOfChannels;
        tdstAChannel_* p_stChannel;

        for (p_stChannel = hArray; p_stChannel < end; p_stChannel++) {
            SND_fn_vKillObjectSound2((s32) p_stChannel, inv_SND_moduleObjectSoundType);
        }
        return inv_MEM_fn_lFreeIntView_inv(gp_6F74, hArray);
    }
}

void fn_vTakeControlOfChannel(tdstEngineObject_* _hActor, u16 _uwChannel, u8 _bControlled)
{
    gu_tdst3dData_* hCurrent3dData = _hActor->h_3dData;
    tdstAChannel_* hArray = hCurrent3dData->hArrayOfChannels;

    if (hArray != NULL) {
        s32 ulNumberOfChannels = hCurrent3dData->ulNumberOfChannels;

        if (_uwChannel != ALL_CHANNELS) {
            hArray[_uwChannel].bControlledChannel = _bControlled;
        } else {
            tdstAChannel_* end = hArray + ulNumberOfChannels;
            tdstAChannel_* p_stChannel;

            for (p_stChannel = hArray; p_stChannel < end; p_stChannel++) {
                p_stChannel->bControlledChannel = _bControlled;
            }
        }
    }
}

void fn_vUpdateSPOFlagsOfChannelForActivation(HIE_tdstSuperObject_* _hSOChannel, u8 _bActivate)
{
    s32 ulSpoFlags;

    if (_hSOChannel == NULL) {
        return;
    }
    ulSpoFlags = _hSOChannel->flags;
    if (_bActivate) {
        _hSOChannel->flags = ulSpoFlags & ~CHANNEL_SUPER_OBJECT_FLAGS;
    } else {
        _hSOChannel->flags = ulSpoFlags | CHANNEL_SUPER_OBJECT_FLAGS;
    }
}

void fn_vActivateChannel(HIE_tdstSuperObject_* _hPerso, u16 _uwChannel, u8 _bActivate)
{
    tdstEngineObject_* object = _hPerso->hLinkedObject;
    gu_tdst3dData_* h_3dData = object->h_3dData;
    tdstAChannel_* hArray = h_3dData->hArrayOfChannels;
    s32 ulNumberOfChannels;

    if (hArray == NULL) {
        return;
    }
    ulNumberOfChannels = h_3dData->ulNumberOfChannels;
    if (_uwChannel != ALL_CHANNELS) {
        tdstAChannel_* p_stChannel = &hArray[_uwChannel];

        p_stChannel->bActiveChannel = _bActivate;
        fn_vUpdateSPOFlagsOfChannelForActivation(inv_GAM_fn_p_stChannelSuperObjectOf(p_stChannel), _bActivate);
    } else {
        tdstAChannel_* end = hArray + ulNumberOfChannels;
        tdstAChannel_* p_stChannel;

        for (p_stChannel = hArray; p_stChannel < end; p_stChannel++) {
            p_stChannel->bActiveChannel = _bActivate;
            fn_vUpdateSPOFlagsOfChannelForActivation(inv_GAM_fn_p_stChannelSuperObjectOf(p_stChannel), _bActivate);
        }
    }
}

tdstMorphChannelList_* fn_p_stCreateMorphListElement(void)
{
    tdstMorphChannelList_* element = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstMorphChannelList_));

    element->hNextBrotherDyn = NULL;
    element->hPrevBrotherDyn = NULL;
    element->hFatherDyn = NULL;
    return element;
}

s32 fn_vFreeMorphListElement(tdstMorphChannelList_* element)
{
    return inv_MEM_fn_lFreeIntView_inv(gp_6F74, element);
}

void fn_vInitMorphList(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn** p_hMorphList)
{
    if (*p_hMorphList == NULL) {
        *p_hMorphList =
            AI_fn_p_vTrueAlloc(gp_6F74, sizeof(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn));
        (*p_hMorphList)->hFirstElementDyn = NULL;
        (*p_hMorphList)->hLastElementDyn = NULL;
        (*p_hMorphList)->lNumberOfElementsDyn = 0;
    }
}

void fn_vFreeMorphList(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn** p_hMorphList)
{
    if (*p_hMorphList == NULL) {
        return;
    }
    while ((*p_hMorphList)->lNumberOfElementsDyn > 0) {
        tdstMorphChannelList_* p_hMorphElt = (*p_hMorphList)->hFirstElementDyn;

        inv_LST2_M_vIsolateElement(p_hMorphElt);
        fn_vFreeMorphListElement(p_hMorphElt);
    }
    inv_MEM_fn_vFreeInHeap(gp_6F74, *p_hMorphList);
    *p_hMorphList = NULL;
}

void fn_vPutMorphElementInChannel(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hMorphList,
                                  u16 uwChannel, void* from, void* to)
{
    tdstMorphChannelList_* position;
    tdstMorphChannelList_* element;

    if (hMorphList == NULL) {
        return;
    }
    for (position = hMorphList->hFirstElementDyn; position != NULL;
         position = position->hNextBrotherDyn) {
        if (position->uwChannelNumber == uwChannel) {
            position->from = from;
            position->to = to;
            break;
        }
        if (uwChannel < position->uwChannelNumber) {
            break;
        }
    }
    if (position == NULL) {
        element = fn_p_stCreateMorphListElement();
        element->uwChannelNumber = uwChannel;
        element->from = from;
        element->to = to;
        LST2_M_vAddElementToList(hMorphList, element);
    } else if (position->uwChannelNumber != uwChannel) {
        element = fn_p_stCreateMorphListElement();
        element->uwChannelNumber = uwChannel;
        element->from = from;
        element->to = to;
        inv_LST2_M_vIsolateElement(element);
        inv_GAM_fn_vInsertElementBefore(element, position);
    }
}

void fn_vRemoveChannelFromMorphList(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hMorphList,
                                    tdstMorphChannelList_* element)
{
    inv_LST2_M_vIsolateElement(element);
    fn_vFreeMorphListElement(element);
}

tdstMorphChannelList_*
inv_fn_p_stFindOrCreateMorphListElement(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hMorphList,
                                        u16 uwChannel, u8 create)
{
    tdstMorphChannelList_* p_stMorph;
    tdstMorphChannelList_* element;

    if (hMorphList == NULL) {
        return NULL;
    }
    for (p_stMorph = hMorphList->hFirstElementDyn; p_stMorph != NULL;
         p_stMorph = p_stMorph->hNextBrotherDyn) {
        if (p_stMorph->uwChannelNumber == uwChannel) {
            return p_stMorph;
        }
        if (uwChannel < p_stMorph->uwChannelNumber) {
            break;
        }
    }
    if (create) {
        element = fn_p_stCreateMorphListElement();
        element->uwChannelNumber = uwChannel;
        if (p_stMorph == NULL) {
            LST2_M_vAddElementToList(hMorphList, element);
        } else {
            inv_LST2_M_vIsolateElement(element);
            inv_GAM_fn_vInsertElementBefore(element, p_stMorph);
        }
        return element;
    }
    return NULL;
}

void* fn_hGetMorphedObjectInChannel(LST2_tdstAnchorToCHN_tdxHandleToMorphElementDyn* hMorphList,
                                    u16 uwChannel)
{
    tdstMorphChannelList_* element =
        inv_fn_p_stFindOrCreateMorphListElement(hMorphList, uwChannel, 0);

    if (element != NULL) {
        return inv_MOR_fn_p_vGetMorphTarget(element);
    }
    return NULL;
}
