#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "AI/AI.h"

extern StandardGameList inv_GAM_characterList0;
extern StandardGameList inv_GAM_characterList1;

#define RUNTIME_FLAG_QUEUED 0x01
#define RUNTIME_BIT_TREAT 2
#define RUNTIME_BIT_PERMANENT 4
#define RUNTIME_FLAG_TOO_FAR 0x80
#define CUSTOM_BIT_FROZEN_UPDATE 0x100000
#define CUSTOM_BITS_TOO_FAR_TEST 0x4C0000
#define ALWAYS_PERSONAL_TYPE_BASE 0x8000
#define NO_QUEUE_PRIORITY -1
#define MAX_QUEUE_PRIORITY 0xFF

extern s32 inv_GAM_lQueuePriority;
extern StandardGameList inv_GAM_inactiveCharacterList;

void inv_fn_vDeactivateEngineObjectNode(gu_tdstStandardGame* node);
void loc_10B8D8(void);

static inline u8 inv_GAM_fn_ucQueuePriorityOf(gu_tdstStandardGame* node)
{
    return (node->ucPlatFormType << 7) | (node->queueKey0C & 0x7F);
}

void inv_fn_vClearEngineObjectLists(void)
{
    inv_GAM_lQueuePriority = NO_QUEUE_PRIORITY;
    inv_fn_vIsolateCurrentCharacterList();
    inv_fn_vIsolatePreviousCharacterList();
    loc_10B8D8();
}

s32 inv_fn_bIsQueuePriorityActive(void)
{
    return inv_GAM_lQueuePriority != NO_QUEUE_PRIORITY;
}

void inv_fn_vInitEngineObjectLists(void)
{
    inv_GAM_characterList0.first = NULL;
    inv_GAM_characterList0.last = NULL;
    inv_GAM_characterList0.count = 0;
    inv_GAM_characterList1.first = NULL;
    inv_GAM_characterList1.last = NULL;
    inv_GAM_characterList1.count = 0;
    inv_GAM_currentCharacterList = &inv_GAM_characterList0;
    inv_GAM_previousCharacterList = &inv_GAM_characterList1;
    inv_GAM_inactiveCharacterList.first = NULL;
    inv_GAM_inactiveCharacterList.last = NULL;
    inv_GAM_inactiveCharacterList.count = 0;
}

void loc_10B8D8(void)
{
    gu_tdstStandardGame* node;
    gu_tdstStandardGame* next;

    node = inv_GAM_inactiveCharacterList.first;
    while (node != NULL) {
        next = node->listNext;
        inv_fn_vIsolateStandardGame(node);
        node = next;
    }
}

void inv_fn_vMoveToInactiveCharacterList(gu_tdstStandardGame* node)
{
    if (node->ucMiscFlags >> RUNTIME_BIT_PERMANENT & 1) {
        return;
    }
    inv_fn_vIsolateStandardGame(node);
    inv_fn_vIsolateStandardGame(node);
    node->listPrev = inv_GAM_inactiveCharacterList.last;
    node->listAnchor = &inv_GAM_inactiveCharacterList;
    if (inv_GAM_inactiveCharacterList.first == NULL) {
        inv_GAM_inactiveCharacterList.first = node;
    } else {
        inv_GAM_inactiveCharacterList.last->listNext = node;
    }
    inv_GAM_inactiveCharacterList.last = node;
    inv_GAM_inactiveCharacterList.count++;
}

void inv_fn_vRecycleAllActiveEngineObjectNodes(void)
{
    StandardGameList* list;
    gu_tdstStandardGame* node;
    gu_tdstStandardGame* next;

    list = inv_GAM_previousCharacterList;
    next = list->first;
    if (next != NULL) {
        node = next;
        do {
            next = next->listNext;
            inv_fn_vMoveToInactiveCharacterList(node);
            node = next;
        } while (next != NULL);
        list = inv_GAM_previousCharacterList;
    }
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
}

static inline u8 inv_GAM_fn_ucStandardGameQueued(gu_tdstStandardGame* standardGame)
{
    return standardGame->ucMiscFlags & RUNTIME_FLAG_QUEUED;
}

void inv_fn_vDeactivateEngineObjectNode(gu_tdstStandardGame* node)
{
    tdstEngineObject_* object;
    gu_tdstStandardGame* standardGame;

    object = ((HIE_tdstSuperObject_*) node->h_SuperObject)->hLinkedObject;
    standardGame = object->h_StandardGame;
    if (inv_GAM_fn_ucStandardGameQueued(standardGame)) {
        if (standardGame->eInitFlagWhenDeadOrTaken == 0) {
            goto reinit;
        }
        if (standardGame->eInitFlagWhenDeadOrTaken == 1) {
            goto reinit;
        }
    }
    if (inv_GAM_fn_ucStandardGameQueued(standardGame) || standardGame->eInitFlagWhenOutOfZone) {
        goto unlink;
    }
reinit:
    if (!((object->h_StandardGame->ucMiscFlags >> 4) & 1)) {
        fn_vInitOneObject(object, 4);
    }
unlink:
    inv_fn_vIsolateStandardGame(node);
}

void inv_fn_vDeactivateAllEngineObjectNodes(void)
{
    gu_tdstStandardGame* node;

    node = inv_GAM_inactiveCharacterList.first;
    while (node != NULL) {
        inv_fn_vDeactivateEngineObjectNode(node);
        node = inv_GAM_inactiveCharacterList.first;
    }
}

void inv_fn_vQueueEngineObjectByPriority(gu_tdstStandardGame* node)
{
    StandardGameList* list;
    gu_tdstStandardGame* next;
    gu_tdstStandardGame* previous;
    u8 priority;

    if (node == NULL) {
        return;
    }

    priority = inv_GAM_fn_ucQueuePriorityOf(node);
    if (inv_GAM_lQueuePriority > 0 && inv_GAM_lQueuePriority < priority) {
        list = inv_GAM_previousCharacterList;
    } else {
        list = inv_GAM_currentCharacterList;
    }

    inv_fn_vIsolateStandardGame(node);

    previous = NULL;
    next = list->first;
    if (next != NULL && inv_GAM_fn_ucQueuePriorityOf(next) > inv_GAM_fn_ucQueuePriorityOf(node)) {
        do {
            previous = next;
            next = previous->listNext;
        } while (next != NULL && inv_GAM_fn_ucQueuePriorityOf(node) < inv_GAM_fn_ucQueuePriorityOf(next));
    }

    if (previous == NULL) {
        inv_fn_vIsolateStandardGame(node);
        node->listNext = list->first;
        node->listAnchor = list;
        if (list->last == NULL) {
            list->last = node;
        } else {
            list->first->listPrev = node;
        }
        list->first = node;
        list->count++;
    } else if (next == NULL) {
        inv_fn_vIsolateStandardGame(node);
        node->listPrev = list->last;
        node->listAnchor = list;
        if (list->first == NULL) {
            list->first = node;
        } else {
            list->last->listNext = node;
        }
        list->last = node;
        list->count++;
    } else {
        inv_fn_vIsolateStandardGame(node);
        node->listNext = next;
        node->listPrev = previous;
        if (previous != NULL) {
            previous->listNext = node;
            node->listAnchor = previous->listAnchor;
        } else {
            next->listAnchor->first = node;
        }
        if (next != NULL) {
            next->listPrev = node;
            node->listAnchor = next->listAnchor;
        } else {
            previous->listAnchor->last = node;
        }
        node->listAnchor->count++;
    }
}

void inv_fn_vIsolateCurrentCharacterList(void)
{
    gu_tdstStandardGame* node;
    gu_tdstStandardGame* next;

    node = inv_GAM_currentCharacterList->first;
    while (node != NULL) {
        next = node->listNext;
        inv_fn_vIsolateStandardGame(node);
        node = next;
    }
}

void inv_fn_vIsolatePreviousCharacterList(void)
{
    gu_tdstStandardGame* node;
    gu_tdstStandardGame* next;

    node = inv_GAM_previousCharacterList->first;
    while (node != NULL) {
        next = node->listNext;
        inv_fn_vIsolateStandardGame(node);
        node = next;
    }
    inv_GAM_previousCharacterList->first = NULL;
    inv_GAM_previousCharacterList->last = NULL;
    inv_GAM_previousCharacterList->count = 0;
}

void inv_fn_vTreatQueuedCharactersBySector(HIE_tdstSuperObject_* excluded,
                                           MTH3D_tdstVector_* reference)
{
    gu_tdstStandardGame* node;
    StandardGameList* treated;
    HIE_tdstSuperObject_* superObject;
    HIE_tdstSuperObject_* father;
    MTH3D_tdstVector_ difference;
    u8* used;
    s32 stamp;
    u8 priority;
    f32 distance;

    node = inv_GAM_currentCharacterList->first;
    priority = MAX_QUEUE_PRIORITY;
    inv_GAM_lQueuePriority = priority;
    HIE_fn_vPushSuperObjectMatrix(gp_stDynamicWorld);
    while (node != NULL) {
        priority = inv_GAM_fn_ucQueuePriorityOf(node);
        superObject = node->h_SuperObject;
        if (priority < inv_GAM_lQueuePriority) {
            inv_GAM_lQueuePriority = priority;
        }
        if (node->ucMiscFlags >> RUNTIME_BIT_TREAT & 1) {
            stamp = node->ulLastTrame;
            if (stamp != inv_fn_ulGetEngineTrameNumber() && (g_stEngineStructure.bEngineFrozen == 0 ||
                                             (node->ulCustomBits & CUSTOM_BIT_FROZEN_UPDATE))) {
                node->ucMiscFlags &= ~RUNTIME_FLAG_TOO_FAR;
                if (superObject != excluded && (node->ulCustomBits & CUSTOM_BITS_TOO_FAR_TEST)) {
                    difference.xX = inv_HIE_fn_p_stGetGlobalTranslation(superObject)->xX - reference->xX;
                    distance = inv_MTH_fn_xAbsolute(difference.xX);
                    difference.xY = inv_HIE_fn_p_stGetGlobalTranslation(superObject)->xY - reference->xY;
                    distance += inv_MTH_fn_xAbsolute(difference.xY);
                    difference.xZ = inv_HIE_fn_p_stGetGlobalTranslation(superObject)->xZ - reference->xZ;
                    distance += inv_MTH_fn_xAbsolute(difference.xZ);
                    if (distance < node->ucTooFarLimit) {
                        node->ucMiscFlags &= ~RUNTIME_FLAG_TOO_FAR;
                    } else {
                        node->ucMiscFlags |= RUNTIME_FLAG_TOO_FAR;
                    }
                }
                if (node->ucMiscFlags >> RUNTIME_BIT_PERMANENT & 1) {
                    used = &g_stAlways.used[node->otObjectPersonalType - ALWAYS_PERSONAL_TYPE_BASE];
                    if (*used != 0) {
                        *used = 0;
                        inv_AI_fn_vEvalInitComport(superObject);
                    }
                }
                father = superObject->hFatherSta;
                if (father->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
                    HIE_fn_vPushSuperObjectMatrix(father);
                    HIE_fn_vPushSuperObjectMatrix(superObject);
                    fn_vTreatCharactersInSector(superObject);
                    HIE_fn_vPopMatrix();
                    HIE_fn_vPopMatrix();
                } else {
                    HIE_fn_vPushSuperObjectMatrix(superObject);
                    fn_vTreatCharactersInSector(superObject);
                    HIE_fn_vPopMatrix();
                }
                node->ulLastTrame = inv_fn_ulGetEngineTrameNumber();
            }
        }
        if ((node->listAnchor != NULL || !(node->ucMiscFlags & RUNTIME_FLAG_QUEUED)) &&
            inv_GAM_currentCharacterList->first == node) {
            inv_fn_vIsolateStandardGame(node);
            inv_fn_vIsolateStandardGame(node);
            node->listPrev = inv_GAM_previousCharacterList->last;
            node->listAnchor = inv_GAM_previousCharacterList;
            if (inv_GAM_previousCharacterList->first == NULL) {
                inv_GAM_previousCharacterList->first = node;
            } else {
                inv_GAM_previousCharacterList->last->listNext = node;
            }
            inv_GAM_previousCharacterList->last = node;
            inv_GAM_previousCharacterList->count++;
        }
        node = inv_GAM_currentCharacterList->first;
    }
    HIE_fn_vPopMatrix();
    treated = inv_GAM_currentCharacterList;
    inv_GAM_lQueuePriority = NO_QUEUE_PRIORITY;
    inv_GAM_currentCharacterList = inv_GAM_previousCharacterList;
    inv_GAM_previousCharacterList = treated;
    treated->first = NULL;
    treated->last = NULL;
    treated->count = 0;
}

void inv_fn_vInitComportOfCurrentCharacters(void)
{
    gu_tdstStandardGame* node;

    node = inv_GAM_currentCharacterList->first;
    inv_GAM_lQueuePriority = MAX_QUEUE_PRIORITY;
    HIE_fn_vPushSuperObjectMatrix(gp_stDynamicWorld);
    while (node != NULL) {
        inv_AI_fn_vEvalInitComport(node->h_SuperObject);
        inv_fn_vIsolateStandardGame(node);
        node = inv_GAM_currentCharacterList->first;
    }
    HIE_fn_vPopMatrix();
    inv_GAM_lQueuePriority = NO_QUEUE_PRIORITY;
}
