#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"

#include "ps2/libc.h"

#define LIST_LAST 0xFF

extern f32 flt_393DF0[];
extern f32 flt_393DF4[];
extern HIE_tdstSuperObject_* g_a_hPersoInZone[];

static inline tdstBrain_* inv_AI_fn_p_stListEntryBrain(HIE_tdstSuperObject_* object)
{
    return inv_HIE_fn_p_stGetEngineObject(object)->h_Brain;
}

void fn_vPutInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso)
{
    p_List->values[p_List->ucNbElt++] = p_Perso;
}

void fn_vInsertPersoAtPositionInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso, u8 ucPosition)
{
    u8 ucI;

    ucPosition = ucPosition < p_List->ucNbElt + 1 ? ucPosition : p_List->ucNbElt + 1;
    for (ucI = p_List->ucNbElt; ucI > ucPosition; ucI--) {
        p_List->values[ucI] = p_List->values[ucI - 1];
    }
    p_List->values[ucPosition] = p_Perso;
    p_List->ucNbElt++;
}

s32 fn_bFindPersoInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso)
{
    u8 ucPosition;

    if (p_List->ucNbElt == 0) {
        return 0;
    }
    for (ucPosition = 0; ucPosition < p_List->ucNbElt && p_List->values[ucPosition] != p_Perso; ucPosition++) {
    }
    return ucPosition < p_List->ucNbElt;
}

s32 fn_bFindModelInList(tdstList_* p_List, void* _p_stModel)
{
    u8 ucPosition;

    if (p_List->ucNbElt == 0) {
        return 0;
    }
    for (ucPosition = 0;
         ucPosition < p_List->ucNbElt && inv_AI_fn_p_stListEntryBrain(p_List->values[ucPosition])->p_stMind->p_stAIModel != _p_stModel;
         ucPosition++) {
    }
    return ucPosition < p_List->ucNbElt;
}

s32 fn_bFindFamilyInList(tdstList_* p_List, tdstFamilyListView* hFamily)
{
    u8 ucPosition;

    if (p_List->ucNbElt == 0) {
        return 0;
    }
    for (ucPosition = 0;
         ucPosition < p_List->ucNbElt && inv_HIE_fn_p_stGetEngineObject(p_List->values[ucPosition])->h_3dData->h_Family != hFamily;
         ucPosition++) {
    }
    return ucPosition < p_List->ucNbElt;
}

s32 fn_bFindPersoAndDeleteInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso)
{
    u8 index;
    u8 next;

    for (index = 0; index < p_List->ucNbElt; index++) {
        if (p_List->values[index] == p_Perso) {
            break;
        }
    }
    if (p_List->values[index] == p_Perso) {
        for (next = index; next <= p_List->ucNbElt - 2; next++) {
            p_List->values[next] = p_List->values[next + 1];
        }
        p_List->ucNbElt--;
        return 1;
    }
    return 0;
}

void fn_vDeletePersoAtPositionInList(tdstList_* p_List, u8 ucPosition)
{
    u8 index;

    if (ucPosition == LIST_LAST) {
        ucPosition = p_List->ucNbElt - 1;
    }
    for (index = ucPosition; index <= p_List->ucNbElt - 2; index++) {
        p_List->values[index] = p_List->values[index + 1];
    }
    p_List->ucNbElt--;
}

void fn_vSortListByFamily(tdstList_* p_List, tdstFamilyListView* hFamily)
{
    u8 index;
    u8 count = 0;
    HIE_tdstSuperObject_* selected[32];

    for (index = 0; index < p_List->ucNbElt; index++) {
        if (inv_HIE_fn_p_stGetEngineObject(p_List->values[index])->h_3dData->h_Family == hFamily) {
            selected[count++] = p_List->values[index];
        }
    }
    p_List->ucNbElt = 0;
    for (index = 0; index < count; index++) {
        fn_vPutInList(p_List, selected[index]);
    }
}

void fn_vSortListByModel(tdstList_* p_stList, void* _p_stModel)
{
    u8 index = 0;
    u8 count = 0;
    HIE_tdstSuperObject_* entry;
    tdstBrain_* brain;
    HIE_tdstSuperObject_* selected[32];

    for (entry = fn_p_stGetPersoInList(p_stList, 0); entry != NULL;
         entry = fn_p_stGetPersoInList(p_stList, ++index)) {
        brain = inv_AI_fn_p_stListEntryBrain(entry);
        if (brain != NULL && brain->p_stMind->p_stAIModel == _p_stModel) {
            selected[count++] = entry;
        }
    }
    p_stList->ucNbElt = 0;
    for (index = 0; index < count; index++) {
        fn_vPutInList(p_stList, selected[index]);
    }
}

void fn_vUnionList(tdstList_* p_ListA, tdstList_* p_ListB, tdstList_* p_ListC)
{
    u8 i;
    u8 j;
    HIE_tdstSuperObject_* value;

    for (i = 0; i < p_ListB->ucNbElt; i++) {
        fn_vPutInList(p_ListA, p_ListB->values[i]);
    }
    for (i = 0; i < p_ListC->ucNbElt; i++) {
        value = p_ListC->values[i];
        for (j = 0; j < p_ListB->ucNbElt; j++) {
            if (p_ListB->values[j] == value) {
                break;
            }
        }
        if (p_ListB->values[j] != value) {
            fn_vPutInList(p_ListA, value);
        }
    }
}

void fn_vInterList(tdstList_* p_ListA, tdstList_* p_ListB, tdstList_* p_ListC)
{
    u8 index;
    u8 other;
    HIE_tdstSuperObject_* value;

    for (index = 0; index < p_ListB->ucNbElt; index++) {
        value = p_ListB->values[index];
        for (other = 0; other < p_ListC->ucNbElt; other++) {
            if (p_ListC->values[other] == value) {
                fn_vPutInList(p_ListA, value);
                break;
            }
        }
    }
}

void fn_vDiffList(tdstList_* p_ListA, tdstList_* p_ListB, tdstList_* p_ListC)
{
    u8 i;
    u8 j;
    u8 secondCount;
    HIE_tdstSuperObject_* value;

    for (i = 0; i < p_ListB->ucNbElt; i++) {
        value = p_ListB->values[i];
        secondCount = p_ListC->ucNbElt;
        for (j = 0; j < secondCount; j++) {
            if (p_ListC->values[j] == value) {
                break;
            }
        }
        if (p_ListC->values[j] != value) {
            fn_vPutInList(p_ListA, value);
        }
    }
}

void fn_vAddList(tdstList_* p_ListA, tdstList_* p_ListB)
{
    u8 i;
    u8 j;
    u8 count;
    HIE_tdstSuperObject_* value;

    count = p_ListA->ucNbElt;
    for (i = 0; i < p_ListB->ucNbElt; i++) {
        value = p_ListB->values[i];
        for (j = 0; j < count; j++) {
            if (p_ListA->values[j] == value) {
                break;
            }
        }
        if (p_ListA->values[j] != value) {
            fn_vPutInList(p_ListA, value);
        }
    }
}

void fn_vAffectZddInList(tdstList_* p_List, HIE_tdstSuperObject_* p_stPerso, u8 ucModulOrCharact)
{
    u16 uwNbDetectedPerso;
    u16 uwI;

    p_List->ucNbElt = 0;
    uwNbDetectedPerso = p_List->ucMaxSize;
    fn_vGetAllObjectsInZdd(p_stPerso, ucModulOrCharact, g_a_hPersoInZone, &uwNbDetectedPerso);
    for (uwI = 0; uwI < uwNbDetectedPerso; uwI++) {
        fn_vPutInList(p_List, g_a_hPersoInZone[uwI]);
    }
}

void fn_vAffectZdeInList(tdstList_* p_List, HIE_tdstSuperObject_* p_stPerso, u8 ucModulOrCharact, u16 xMask)
{
    u16 uwNbDetectedPerso;
    u16 uwI;

    p_List->ucNbElt = 0;
    uwNbDetectedPerso = p_List->ucMaxSize;
    fn_vGetAllCharactersInContactWithThisZde(p_stPerso, ucModulOrCharact, g_a_hPersoInZone, &uwNbDetectedPerso, xMask);
    for (uwI = 0; uwI < uwNbDetectedPerso; uwI++) {
        fn_vPutInList(p_List, g_a_hPersoInZone[uwI]);
    }
}

void fn_vAffectTypeZdeWithTypeZdeInList(tdstList_* p_List, HIE_tdstSuperObject_* p_stPerso,
                                        u16 xPersoMask, u16 xOthersMask)
{
    u16 uwNbDetectedPerso;
    u16 uwI;

    p_List->ucNbElt = 0;
    uwNbDetectedPerso = p_List->ucMaxSize;
    fn_vThisTypeVsThisType(p_stPerso, xPersoMask, xOthersMask, g_a_hPersoInZone, &uwNbDetectedPerso);
    for (uwI = 0; uwI < uwNbDetectedPerso; uwI++) {
        fn_vPutInList(p_List, g_a_hPersoInZone[uwI]);
    }
}

s32 fn_ucAtLeastOnePersoInZdd(HIE_tdstSuperObject_* p_stPerso, u8 ucModulOrCharact)
{
    u16 uwNbDetectedPerso = 1;

    fn_vGetAllObjectsInZdd(p_stPerso, ucModulOrCharact, g_a_hPersoInZone, &uwNbDetectedPerso);
    return uwNbDetectedPerso != 0;
}

s32 fn_ucAtLeastOnePersoInZde(HIE_tdstSuperObject_* p_stPerso, u8 ucModulOrCharact, u16 xMask)
{
    u16 uwNbDetectedPerso = 1;

    fn_vGetAllCharactersInContactWithThisZde(p_stPerso, ucModulOrCharact, g_a_hPersoInZone, &uwNbDetectedPerso, xMask);
    return uwNbDetectedPerso != 0;
}

HIE_tdstSuperObject_* fn_p_stGetPersoInList(tdstList_* p_List, s32 ucPosition)
{
    u8 index = ucPosition;

    if (p_List->ucNbElt == 0 || (index >= p_List->ucNbElt && index != LIST_LAST)) {
        return NULL;
    }
    if (index == LIST_LAST) {
        return p_List->values[p_List->ucNbElt - 1];
    }
    return p_List->values[index];
}

tdstNodeInterpret_* fn_p_stListSelect(tdstList_* p_List, s8 b_cInverse, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_** p_p_stPerso;
    tdstNodeInterpret_* p_stTreeSv;
    tdstGetSetParam_* scratch;
    tdstList_ selected;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTreeSv = p_stTree;
    selected.ucNbElt = 0;
    for (p_p_stPerso = p_List->values; p_p_stPerso < &p_List->values[p_List->ucNbElt]; p_p_stPerso++) {
        p_stTree = fn_p_stEvalTree(*p_p_stPerso, p_stTreeSv, scratch);
        if ((s8) scratch->u.lValue != b_cInverse) {
            fn_vPutInList(&selected, *p_p_stPerso);
        }
    }
    memcpy(p_List, &selected, selected.ucNbElt * 4 + 4);
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stListSort(tdstList_* p_List, u8 ucWantedNbElt, s8 b_cIncrease,
                                    tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_** entry;
    HIE_tdstSuperObject_* saved;
    u8 i;
    u8 j;
    f32 keys[32];
    f32* output;
    f32 bound;
    u8 selected;
    tdstNodeInterpret_* expression;
    tdstGetSetParam_* scratch;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    selected = 0;
    expression = p_stTree;
    output = keys;
    for (entry = p_List->values; entry < &p_List->values[p_List->ucNbElt]; entry++) {
        p_stTree = fn_p_stEvalTree(*entry, expression, scratch);
        *output++ = scratch->ucType == E_vt_Float ? scratch->u.xValue : (f32) scratch->u.lValue;
    }
    if (b_cIncrease == 0) {
        bound = flt_393DF0[0];
        for (i = 0; i < p_List->ucNbElt - 1 && i < ucWantedNbElt; i++) {
            for (j = i; j < p_List->ucNbElt; j++) {
                if (keys[j] > bound) {
                    bound = keys[j];
                    selected = j;
                }
            }
            keys[selected] = keys[i];
            saved = p_List->values[selected];
            p_List->values[selected] = p_List->values[i];
            p_List->values[i] = saved;
        }
    } else {
        bound = flt_393DF4[0];
        for (i = 0; i < p_List->ucNbElt - 1 && i < ucWantedNbElt; i++) {
            for (j = i; j < p_List->ucNbElt; j++) {
                if (keys[j] < bound) {
                    bound = keys[j];
                    selected = j;
                }
            }
            keys[selected] = keys[i];
            saved = p_List->values[selected];
            p_List->values[selected] = p_List->values[i];
            p_List->values[i] = saved;
        }
    }
    p_List->ucNbElt = p_List->ucNbElt > ucWantedNbElt ? ucWantedNbElt : p_List->ucNbElt;
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}
