#include "common.h"
#include "HIE/HIE.h"
#include "ps2/libc.h"
#include "GAM/GAM.h"
#include "AI/AI.h"
#include "MTH/MTH.h"
#include "snd/snd.h"
#include "ps2/copy_types.h"
#include "include_asm.h"

void fn_vInitAllEvents(HIE_tdstSuperObject_* p_stSuperObject, u32 uwFirstFrame)
{
    gu_tdst3dData_* owner;
    tdstState_* node;
    tdstAnim3d_* list;
    tdstEvent_* cursor;
    u8* buf;
    u8 count;
    s32 index;
    s32 reg;
    s32 lim;
    tdstEventInTable_* scan;

    owner = inv_HIE_fn_p_stGetEngineObject(p_stSuperObject)->h_3dData;
    node = owner->h_CurrentState;
    uwFirstFrame &= 0xFFFF;
    if (node == NULL) {
        return;
    }
    list = node->p_stAnim;
    cursor = list->d_stAnimEvent;
    if (cursor == NULL) {
        return;
    }
    buf = AI_fn_p_vTrueAlloc(gp_6F74, list->ucNumberOfEvents);
    owner->d_ucEventActivation = buf;
    if (uwFirstFrame != 0) {
        reg = list->ucNumberOfEvents;
        index = 0;
        if (reg != 0 && cursor->uwFrameNumber < uwFirstFrame) {
            lim = reg;
            reg = (s32) &cursor->uwFrameNumber;
            do {
                index += 1;
                if (index >= lim) {
                    break;
                }
                reg += 0xC;
            } while (*(u16*) reg < uwFirstFrame);
        }
        owner->ucNextEvent = (u8) index;
    } else if (node->scSpeedAnim > 0) {
        owner->ucNextEvent = 0;
    } else {
        owner->ucNextEvent = (u8) (list->ucNumberOfEvents - 1);
    }

    count = list->ucNumberOfEvents;
    index = 0;
    if (count != 0) {
        do {
            scan = fn_p_GetEventInTable(owner, cursor);
            buf = owner->d_ucEventActivation;
            buf[index] = scan->ucFirstCall;
            index += 1;
            cursor++;
        } while (index < list->ucNumberOfEvents);
    }
}

void fn_vDesInitAllEvents(HIE_tdstSuperObject_* p_stSuperObject, tdstState_* hNextState,
                          s32 bStopEvents)
{
    gu_tdst3dData_* data = ((tdstEngineObject_*) p_stSuperObject->hLinkedObject)->h_3dData;
    tdstState_* state = data->h_CurrentState;
    tdstAnim3d_* current;
    tdstAnim3d_* wanted;
    tdstEvent_* event;
    tdstEvent_* next;
    tdstEvent_* copy;
    tdstEvent_* other;
    tdstEventInTable_* definition;
    tdstEventInTable_* otherDefinition;
    u8* activation;
    s32 i;
    s32 j;
    s32 shared;

    if (state == NULL)
        return;
    current = state->p_stAnim;
    if (current != NULL && current->ucNumberOfEvents != 0 && (event = current->d_stAnimEvent) != NULL &&
        bStopEvents && hNextState != NULL) {
        wanted = hNextState->p_stAnim;
        if (wanted != NULL && wanted->ucNumberOfEvents != 0) {
            other = wanted->d_stAnimEvent;
            if (other != NULL) {
                copy = AI_fn_p_vTrueAlloc(gp_6F74, wanted->ucNumberOfEvents * sizeof(tdstEvent_));
                i = 0;
                memcpy(copy, other, wanted->ucNumberOfEvents * sizeof(tdstEvent_));
                event = current->d_stAnimEvent;
                if (current->ucNumberOfEvents != 0) {
                    do {
                        shared = 0;
                        definition = fn_p_GetEventInTable(data, event);
                        if (definition->ucFirstCall != 0xFF)
                            break;
                        j = 0;
                        i++;
                        other = copy;
                        do {
                            next = event + 1;
                        } while (0);
                        goto test;
                        for (;;) {
                            j++;
                            other++;
                        test:
                            if (j >= wanted->ucNumberOfEvents)
                                break;
                            otherDefinition = fn_p_GetEventInTable(data, other);
                            if (otherDefinition->ucFirstCall != 0xFF)
                                break;
                            if (otherDefinition->ucFirstCall & 0x100)
                                i = i * i * i;
                            if (definition->p_xEvent.sound == otherDefinition->p_xEvent.sound) {
                                shared = 1;
                                break;
                            }
                        }
                        if (shared == 0)
                            fn_vPlayEvent(event, 2, p_stSuperObject);
                        event = next;
                    } while (i < current->ucNumberOfEvents);
                }
                inv_MEM_fn_vFreeInHeap(gp_6F74, copy);
            } else {
                for (i = 0; i < current->ucNumberOfEvents; i++, event++) {
                    if (fn_p_GetEventInTable(data, event)->ucFirstCall == 0xFF)
                        fn_vPlayEvent(event, 2, p_stSuperObject);
                }
            }
        } else {
            for (i = 0; i < current->ucNumberOfEvents; i++, event++) {
                if (fn_p_GetEventInTable(data, event)->ucFirstCall == 0xFF)
                    fn_vPlayEvent(event, 2, p_stSuperObject);
            }
        }
    }
    activation = data->d_ucEventActivation;
    data->ucNextEvent = 0;
    if (activation != NULL) {
        inv_MEM_fn_vFreeInHeap(gp_6F74, activation);
        data->d_ucEventActivation = NULL;
    }
}

#define ANIMATION_EVENT_SOUND 0
#define ANIMATION_EVENT_USER 3
#define ANIMATION_EVENT_FIRST_USER_FLAG 6
#define ANIMATION_EVENT_USER_FLAG_COUNT 8
#define ANIMATION_EVENT_NO_OBJECT 0xFFFF
#define ANIMATION_EVENT_COUNTER_OFF 0xFF
#define CUSTOM_BIT_EVENT_UPDATE 2

static inline gu_tdst3dData_* inv_GAM_fn_p_stDataOf(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return object->h_3dData;
}

static inline HIE_tdstSuperObject_* inv_GAM_fn_p_stCsoObject(gu_tdst3dData_* data, u16 index)
{
    tdstAChannel_* records = (tdstAChannel_*) data->hArrayOfChannels;

    return records != NULL ? records[index].hSupObject : NULL;
}

void fn_vPlayEventsSinceLastOneNoLoop(HIE_tdstSuperObject_* pOwner)
{
    gu_tdst3dData_* pState = *(gu_tdst3dData_**) pOwner->hLinkedObject;
    tdstState_* pAnim = pState->h_CurrentState;
    tdstAnim3d_* pList;
    tdstEvent_* pEvts;
    u8 idx;
    s32 ff;
    u8 *p;

    if (pAnim == NULL)
        return;
    pList = pAnim->p_stAnim;
    pEvts = pList->d_stAnimEvent;
    if (pEvts == NULL)
        return;

    idx = pState->ucNextEvent;
    if (pAnim->scSpeedAnim > 0) {
        while (idx != pList->ucNumberOfEvents && pState->uwCurrentFrame >= pEvts[idx].uwFrameNumber) {
            {
                u8 *pCnt;
                s32 c;
                pCnt = pState->d_ucEventActivation;
                if (idx & 0x100) {
                    if (pCnt[1] != (pCnt[2] ^ pCnt[3])) { c = pCnt[idx]; pCnt[idx] = 0; *(u16 *)(pCnt + idx) = 0; ff = 0xFF; if (ff) goto nz1; }
                }
                ff = 0xFF;
                c = pCnt[idx];
                if (c == 0) {
                    fn_vPlayEvent(&pEvts[idx], 0, pOwner);
                    {
                        u8 j = pState->ucNextEvent;
                        tdstEventInTable_* r = fn_p_GetEventInTable(pState, &pEvts[j]);
                        p = &pState->d_ucEventActivation[j]; *p = r->ucPeriod;
                    }
                } else {
                nz1:
                    if (c != ff) {
                        p = &pCnt[idx];
                        *p += 0xFF;
                    }
                }
            }
            pState->ucNextEvent += 1;
            idx = pState->ucNextEvent;
        }
    } else {
        while (pState->ucNextEvent != 0xFF && pEvts[pState->ucNextEvent].uwFrameNumber >= pState->uwCurrentFrame) {
            {
                u8 *pCnt;
                s32 c;
                pCnt = pState->d_ucEventActivation;
                if (idx & 0x100) {
                    if (pCnt[2] != pCnt[3]) { c = pCnt[idx]; pCnt[idx] = 0; *(u16 *)(pCnt + idx) = 0; if (pState) goto nz2; }
                }
                c = pCnt[idx];
                if (c == 0) {
                    fn_vPlayEvent(&pEvts[idx], 1, pOwner);
                    {
                        u8 j = pState->ucNextEvent;
                        tdstEventInTable_* r = fn_p_GetEventInTable(pState, &pEvts[j]);
                        p = &pState->d_ucEventActivation[j];
                        *p = r->ucPeriod;
                    }
                } else {
                nz2:
                    if (c != 0xFF) {
                        p = &pCnt[idx];
                        *p += 0xFF;
                    }
                }
            }
            pState->ucNextEvent += 0xFF;
            idx = pState->ucNextEvent;
        }
    }
}

void fn_vPlayLastEventsBeforeRestart(HIE_tdstSuperObject_* superObject)
{
    gu_tdst3dData_* data = inv_GAM_fn_p_stDataOf(superObject);
    tdstState_* state = data->h_CurrentState;
    tdstAnim3d_* animation;
    tdstEvent_* events;
    s16 i;

    if (state == NULL) {
        return;
    }
    animation = state->p_stAnim;
    events = animation->d_stAnimEvent;
    if (events == NULL) {
        return;
    }
    if (state->scSpeedAnim > 0) {
        for (i = data->ucNextEvent; i < animation->ucNumberOfEvents; i++) {
            u8 counter = data->d_ucEventActivation[i];

            if (counter == 0) {
                fn_vPlayEvent(&events[i], 0, superObject);
                data->d_ucEventActivation[i] = fn_p_GetEventInTable(data, &events[i])->ucPeriod;
            } else if (counter != ANIMATION_EVENT_COUNTER_OFF) {
                data->d_ucEventActivation[i] = counter + 0xFF;
            }
        }
        data->ucNextEvent = 0;
    } else {
        s32 start = data->ucNextEvent;

        if (start != ANIMATION_EVENT_COUNTER_OFF) {
            for (i = start; i >= 0; i--) {
                u8 counter = data->d_ucEventActivation[i];

                if (counter == 0) {
                    fn_vPlayEvent(&events[i], 1, superObject);
                    data->d_ucEventActivation[i] = fn_p_GetEventInTable(data, &events[i])->ucPeriod;
                } else if (counter != ANIMATION_EVENT_COUNTER_OFF) {
                    data->d_ucEventActivation[i] = counter + 0xFF;
                }
            }
        }
        data->ucNextEvent = animation->ucNumberOfEvents - 1;
    }
}

void fn_vPlayEvent(tdstEvent_* event, s32 backwards, HIE_tdstSuperObject_* superObject)
{
    tdstEventInTable_* definition = fn_p_GetEventInTable(inv_GAM_fn_p_stDataOf(superObject), event);

    switch (definition->ucTypeOfEvent) {
    case ANIMATION_EVENT_SOUND: {
        s32 sound = definition->p_xEvent.sound;

        if (event->uwChannelNumber == ANIMATION_EVENT_NO_OBJECT) {
            SND_fn_lSendRequestSound((s32) superObject, g_lSoundObjectTypeAnim, sound, 0,
                                     0);
        } else {
            SND_fn_lSendRequestSound((s32) inv_GAM_fn_p_stCsoObject(inv_GAM_fn_p_stDataOf(superObject), event->uwChannelNumber),
                                     inv_SND_moduleObjectSoundType, sound, 0, 0);
        }
        break;
    }

    case 2:
        break;

    case ANIMATION_EVENT_USER: {
        s32 flag = definition->p_xEvent.userFlag - ANIMATION_EVENT_FIRST_USER_FLAG;
        HIE_tdstSuperObject_* target;
        s32 customBits;
        POS_tdstMatrix_* matrix;
        MTH3D_tdstVector_ position;

        if ((u32) (flag & 0xFF) >= ANIMATION_EVENT_USER_FLAG_COUNT) {
            break;
        }
        target = inv_GAM_fn_p_stCsoObject(inv_GAM_fn_p_stDataOf(superObject), event->uwChannelNumber);
        if (target == NULL) {
            break;
        }
        customBits = fn_ulStandardGameGetCustomBitsSO(superObject);
        if ((customBits & CUSTOM_BIT_EVENT_UPDATE) == 0) {
            fn_vStandardGameSetCustomBitsSO(superObject, customBits | CUSTOM_BIT_EVENT_UPDATE);
            PLA_fn_vRefreshGlobalMatrixUnderCharacter(superObject);
            fn_vStandardGameSetCustomBitsSO(superObject, customBits);
        }
        matrix = target->gu_hGlobalMatrix;
        position = *(MTH3D_tdstVector_*) &matrix->stMatrix.stCol_3.xX;
        fn_v3dDataSetUserEventFlag(inv_GAM_fn_p_stDataOf(superObject), (u8) flag, 1, &position);
        break;
    }
    }
}
