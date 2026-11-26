#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "AI/AI.h"

#define STANDARD_GAME_ALWAYS_ACTIVE 0x40

extern tdstAlwaysActiveCharacter_* dword_25260C[];

static inline gu_tdstStandardGame* inv_GAM_fn_p_stStandardGameOf(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return object->h_StandardGame;
}

void inv_ALWAYS_fn_vSetCharacterInNode(tdstAlwaysActiveCharacter_* node,
                                       HIE_tdstSuperObject_* character)
{
    node->hAlwActSuperObject = character;
}

HIE_tdstSuperObject_* fn_hGetCharacterInAlwaysActiveCharacterNode(tdstAlwaysActiveCharacter_* hCharNode)
{
    return hCharNode->hAlwActSuperObject;
}

tdstAlwaysActiveCharacter_* inv_ALWAYS_fn_hFindFirstOccupiedCharacterSlot(void)
{
    tdstAlwaysActiveCharacter_* node = dword_25260C[0];

    while (node != NULL && fn_hGetCharacterInAlwaysActiveCharacterNode(node) == NULL) {
        node = node->hPrevBrotherDyn;
    }
    return node != NULL ? node->hNextBrotherDyn : NULL;
}

void fn_vAddCharacterInAlwaysActiveCharacterList(HIE_tdstSuperObject_* character)
{
    tdstAlwaysActiveCharacter_* node;
    tdstAlwaysActiveCharacter_* n1;
    LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn* c1;
    LST2_tdstAnchorTotdxHandleToAlwaysActiveCharacterNodeDyn* list;
    tdstAlwaysActiveCharacter_* head;
    tdstAlwaysActiveCharacter_* tail;

    node = inv_ALWAYS_fn_hFindFirstOccupiedCharacterSlot();
    if (node == NULL) {
        n1 = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstAlwaysActiveCharacter_));
        n1->hNextBrotherDyn = NULL;
        n1->hPrevBrotherDyn = NULL;
        n1->hFatherDyn = NULL;
        node = n1;
        c1 = ((volatile tdstAlwaysActiveCharacter_*) node)->hFatherDyn;
        if (c1 != NULL) {
            c1->lNumberOfElementsDyn = c1->lNumberOfElementsDyn - 1;
            c1 = node->hFatherDyn;
            head = c1->hFirstElementDyn;
            if (head == node) {
                c1->hFirstElementDyn = node->hNextBrotherDyn;
                c1 = node->hFatherDyn;
            }
            if (c1->hLastElementDyn == node) {
                c1->hLastElementDyn = node->hPrevBrotherDyn;
            }
        }
        n1->hPrevBrotherDyn = NULL;
        list = &inv_GAM_hAlwaysActiveCharactersList[0];
        n1->hFatherDyn = NULL;
        n1->hNextBrotherDyn = NULL;
        tail = list->hLastElementDyn;
        n1->hFatherDyn = list;
        n1->hPrevBrotherDyn = tail;
        if (inv_GAM_hAlwaysActiveCharactersList[0].hFirstElementDyn == NULL) {
            inv_GAM_hAlwaysActiveCharactersList[0].hFirstElementDyn = n1;
        } else {
            list->hLastElementDyn->hNextBrotherDyn = n1;
        }
        inv_GAM_hAlwaysActiveCharactersList[0].hLastElementDyn = n1;
        inv_GAM_hAlwaysActiveCharactersList[0].lNumberOfElementsDyn++;
    }
    inv_ALWAYS_fn_vSetCharacterInNode(node, character);
    inv_GAM_fn_p_stStandardGameOf(character)->ucMiscFlags |= STANDARD_GAME_ALWAYS_ACTIVE;
}

void inv_ALWAYS_fn_vRemoveCharacterFromAlwaysActiveCharacterList(HIE_tdstSuperObject_* character)
{
    tdstAlwaysActiveCharacter_* node;

    for (node = fn_hGetFirstAlwaysActiveCharNode(); node != NULL;
         node = fn_hGetNextAlwaysActiveCharNode(node)) {
        if (fn_hGetCharacterInAlwaysActiveCharacterNode(node) == character) {
            LST2_M_vAddElementToList(inv_GAM_hAlwaysActiveCharactersList, node);
            inv_ALWAYS_fn_vSetCharacterInNode(node, NULL);
            break;
        }
    }
    inv_GAM_fn_p_stStandardGameOf(character)->ucMiscFlags &= ~STANDARD_GAME_ALWAYS_ACTIVE;
}

s32 fn_bGetIfCharacterIsInAlwaysActiveCharacterList(HIE_tdstSuperObject_* hCharacter)
{
    return (inv_GAM_fn_p_stStandardGameOf(hCharacter)->ucMiscFlags >> 6) & 1;
}

tdstAlwaysActiveCharacter_* fn_hGetFirstAlwaysActiveCharNode(void)
{
    return inv_GAM_hAlwaysActiveCharactersList[0].hFirstElementDyn;
}

tdstAlwaysActiveCharacter_* fn_hGetNextAlwaysActiveCharNode(tdstAlwaysActiveCharacter_* hCharNode)
{
    tdstAlwaysActiveCharacter_* hNextCharNode = hCharNode->hNextBrotherDyn;
    tdstAlwaysActiveCharacter_* result;

    if (hNextCharNode != NULL && fn_hGetCharacterInAlwaysActiveCharacterNode(hNextCharNode) != NULL) {
        result = hNextCharNode;
    } else {
        result = NULL;
    }
    return result;
}
