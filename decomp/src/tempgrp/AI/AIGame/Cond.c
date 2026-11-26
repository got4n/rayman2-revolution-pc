#include "common.h"

#include "AI/AI.h"
#include "COL/COL.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "IPT/IPT.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"
#include "WP/WP.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#include "ps2/copy_types.h"

#include "include_asm.h"
#ifdef PC_PORT
#include "PC/PC.h"
void fn_vOperatorGetDsgVar(u8 index, u8 subIndex, HIE_tdstSuperObject_* perso, tdstGetSetParam_* param);
#endif
#include "GMT/GMT.h"
#include "snd/snd.h"

s32 fn_bPerso1Point1IntersectPerso2Point2(HIE_tdstSuperObject_* character, MTH3D_tdstVector_* from,
                  HIE_tdstSuperObject_* other, MTH3D_tdstVector_* to);

s32 fn_bTestTypeOfGameMaterial(GMT_tdstGameMaterial_* material, u32 requestedMask)
{
    GMT_tdstCollideMaterial_* handle;
    u32 mask;

    mask = requestedMask & 0xFFFF;
    if (!GMT_fn_b_ulIsValid(material)) {
        return 0;
    }
    handle = GMT_fn_hGetCollideMaterial(material);
    if (handle == (GMT_tdstCollideMaterial_*) -1) {
        return 0;
    }
    return (GMT_fn_hGetCollideMaterialIdentifier(handle) & mask) == mask;
}

s32 fn_bIsTypeOfGMT(void* context, s32 requestedMask)
{
    s32 mask;
    DNM_stReport_* record;

    mask = requestedMask & 0xFFFF;
    record = fn_pstGetDNMReport(context);
    if (record == 0) {
        return 0;
    }
    return fn_bTestTypeOfGameMaterial(record->m_stGround.hCollidedMaterial, mask);
}

tdstNodeInterpret_* fn_p_stComparisonCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                               tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    void* scratch;
    tdstGetSetParam_* param;
    tdstGetSetParam_* param2;
    u16 op;
    s32 bResult;
    f32 val1;
    f32 val2;

    scratch = inv_AI_parameterPoolCursor;
    param = scratch;
    param2 = param + 1;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch + 2;

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param2);

    if (param->ucType == param2->ucType) {
        if (param2->ucType == E_vt_Float) {
            val1 = param[0].u.xValue;
            val2 = param[1].u.xValue;
            goto do_float;
        }
        if (param2->ucType == E_vt_Vector) {
            switch (op) {
            case eCond_Equal:
                bResult = 0;
                if (param[0].u.stVector.xX != param[1].u.stVector.xX ||
                    param[0].u.stVector.xY != param2->u.stVector.xY ||
                    param[0].u.stVector.xZ != param2->u.stVector.xZ)
                    goto vstore;
                bResult = 1;
                *p_lValue = bResult;
                goto done;
            case eCond_Different:
                bResult = 0;
                if (param[0].u.stVector.xX != param[1].u.stVector.xX)
                    goto one;
                if (param[0].u.stVector.xY != param2->u.stVector.xY)
                    goto oneY;
                if (param[0].u.stVector.xZ != param2->u.stVector.xZ) {
                one:
                    bResult = 1;
                }
            vstore:
                *p_lValue = bResult;
                goto done;
            default:
                *p_lValue = 0;
                goto out;
            }
        }
        {
            s32 lval1 = param[0].u.lValue;
            s32 lval2 = param[1].u.lValue;

            switch (op) {
            case eCond_Equal:
                *p_lValue = (lval1 == lval2);
                goto done;
            case eCond_Different:
                *p_lValue = (lval1 != lval2);
                goto done;
            case eCond_Lesser:
                *p_lValue = (lval1 < lval2);
                goto done;
            case eCond_Greater:
                *p_lValue = (lval2 < lval1);
                goto done;
            case eCond_LesserOrEqual:
                *p_lValue = (lval1 <= lval2);
                goto done;
            case eCond_GreaterOrEqual:
                *p_lValue = (lval2 <= lval1);
                goto done;
            default:
                *p_lValue = 0;
                goto out;
            }
        }
    }
    if (param->ucType == E_vt_Float) {
        val1 = param[0].u.xValue;
        val2 = (f32) param[1].u.lValue;
    } else {
        val1 = (f32) param[0].u.lValue;
        if (param2->ucType == E_vt_Float)
            val2 = param[1].u.xValue;
        else
            val2 = (f32) param[1].u.lValue;
    }

do_float:
    switch (op) {
    case eCond_Equal:
        bResult = (val1 == val2);
    fstore:
        *p_lValue = bResult;
        goto out;
    case eCond_Different:
        *p_lValue = (val1 != val2);
        goto done;
    case eCond_Lesser:
        bResult = (val1 < val2);
        goto fstore;
    case eCond_Greater:
        bResult = (val2 < val1);
        goto fstore;
    case eCond_LesserOrEqual:
        bResult = (val1 <= val2);
        goto fstore;
    case eCond_GreaterOrEqual:
        bResult = (val2 <= val1);
        goto fstore;
    default:
        goto zero;
    }
oneY:
    bResult = 1;
    *p_lValue = bResult;
    goto done;
out:
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
zero:
    *p_lValue = 0;
done:
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stBooleanCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree, u32* p_lValue)
{
    tdstGetSetParam_* scratch = inv_AI_parameterPoolCursor;
    s32 kind = AI_NODE_ID(p_stTree);
    u32 state;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    state = scratch->u.ulValue;
    switch (kind) {
    case eCond_Et:
        if (!state)
            p_stTree += p_stTree->uwNodeToSkip;
        else {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
            state = scratch->u.ulValue;
        }
        break;
    case eCond_Ou:
        if (state)
            p_stTree += p_stTree->uwNodeToSkip;
        else {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
            state = scratch->u.ulValue;
        }
        break;
    case eCond_Not:
        state = !state;
        break;
    case eCond_XOr: {
        kind = state;
        state = 0;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        kind ^= scratch->u.ulValue;
        state = (kind != 0);
        break;
    }
    default:
        state = 0;
        break;
    }
    inv_AI_parameterPoolCursor = scratch;
    *p_lValue = state;
    return p_stTree;
}

void* fn_p_stButtonCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                             s32* p_lValue)
{
    tdstGetSetParam_* scratch = inv_AI_parameterPoolCursor;
    u16 op = AI_NODE_ID(p_stTree);

    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    {
        IPT_tdstEntryElement_* entry = scratch->u.pvValue;

        switch (op) {
        case eCond_PressedBut:
            *p_lValue = IPT_fn_bIsValidated(entry);
            break;
        case eCond_JustPressedBut:
            *p_lValue = IPT_fn_bIsJustValidated(entry);
            break;
        case eCond_ReleasedBut:
            *p_lValue = IPT_fn_bIsInvalidated(entry);
            break;
        case eCond_JustReleasedBut:
            *p_lValue = IPT_fn_bIsJustInvalidated(entry);
            break;
        default:
            *p_lValue = 0;
            break;
        }
#ifdef PC_PORT
        *p_lValue = pc_mod_button_condition(op - eCond_PressedBut, (PcAddress) entry, *p_lValue);
#endif
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stZDDCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          s32* p_lValue)
{
    tdstGetSetParam_* scratch;
    void* stream;
    void* ctx;
    void* p;
    void* hCharacterTwo;
    u16 op;

    scratch = inv_AI_parameterPoolCursor;
    stream = p_stTree;
    ctx = p_SuperObjPerso;
    op = AI_NODE_ID(stream);
    inv_AI_parameterPoolCursor = scratch + 1;
    stream = fn_p_stEvalTree(ctx, stream, scratch);
    p = (*(tdstEngineObject_**) scratch)->h_StandardGame->h_SuperObject;
    stream = fn_p_stEvalTree(ctx, stream, scratch);

    switch (op) {
    case eCond_CollidePersoZDDNoWithPerso: {
        u8 count;
        count = *(u8*) scratch;
        stream = fn_p_stEvalTree(ctx, stream, scratch);
        hCharacterTwo = (*(tdstEngineObject_**) scratch)->h_StandardGame->h_SuperObject;
        if (p == 0) {
            *p_lValue = 0;
            break;
        }
        *p_lValue = fn_bIsCharacterTWODetectedByThisZddOfCharacterOne(p, count, hCharacterTwo);
        break;
    }
    case eCond_CollidePersoAllZDDWithPerso: {
        hCharacterTwo = (*(tdstEngineObject_**) scratch)->h_StandardGame->h_SuperObject;
        if (p == 0 || hCharacterTwo == 0) {
            *p_lValue = 0;
            break;
        }
        *p_lValue = fn_bIsCharacterTWODetectedByCharacterOne(p, hCharacterTwo);
        break;
    }
    case eCond_CollidePersoZDDWithAnyPerso: {
        u8 sel = *(u8*) scratch;
        if (p == 0) {
            *p_lValue = 0;
            break;
        }
        *p_lValue = fn_ucAtLeastOnePersoInZdd(p, sel);
        break;
    }
    default:
        *p_lValue = 0;
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* fn_p_stZDECondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                          s32* p_lValue)
{
    u16 mask1;
    u16 mask2;
    tdstGetSetParam_* scratch;
    u32 op;
    void* perso1;
    void* perso2;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;

    switch (op) {
    case eCond_CollidePersoZDENoWithPersoZDENo: {
        u8 idx;
        u8 idx2;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        perso1 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        idx = scratch->u.ucValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        perso2 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        idx2 = scratch->u.ucValue;
        if (perso1 != NULL && perso2 != NULL) {
            *p_lValue = fn_bThisZdeFromOneVsThisZdeFromTwo(perso1, idx, perso2, idx2);
        } else {
            *p_lValue = 0;
        }
        break;
    }

    case eCond_CollidePersoZDENoWithPersoTypeZDE: {
        u8 idx;
        u16 mask;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        perso1 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        idx = scratch->u.ucValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        perso2 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        mask = scratch->u.uwValue;
        if (perso1 != NULL && perso2 != NULL) {
            *p_lValue = fn_bThisZdeFromOneVsAllZdeFromTwo(perso1, idx, perso2, mask);
        } else {
            *p_lValue = 0;
        }
        break;
    }

    case eCond_CollidePersoTypeZDEWithPersoTypeZDE:
    case eCond_CollidePersoAllZDEWithPersoAllZDE:
    case eCond_CollidePersoTypeZDEWithPersoAllZDE:
    case eCond_CollidePersoAllZDEWithPersoTypeZDE:
        GMT_fn_vResetEntireMask(&mask1);
        GMT_fn_vResetEntireMask(&mask2);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        perso1 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        if (op == eCond_CollidePersoAllZDEWithPersoAllZDE) {
            perso2 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        } else if (op == eCond_CollidePersoAllZDEWithPersoTypeZDE) {
            perso2 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
            mask2 = *(u16*) scratch;
        } else if (op == eCond_CollidePersoTypeZDEWithPersoAllZDE) {
            mask1 = *(u16*) scratch;
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
            perso2 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        } else if (op == eCond_CollidePersoTypeZDEWithPersoTypeZDE) {
            mask1 = *(u16*) scratch;
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
            perso2 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
            mask2 = *(u16*) scratch;
        } else {
            perso2 = NULL;
        }
        if (perso1 == NULL) {
            *p_lValue = 0;
        } else if (perso2 == NULL) {
            *p_lValue = 0;
        } else {
            *p_lValue = fn_bThisTypeFromOneVsThisTypeFromTwo(perso1, mask1, perso2, mask2);
        }
        break;

    case eCond_CollidePersoZDENoWithTypeZDE: {
        u8 idx;
        u16 mask;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        perso1 = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        idx = scratch->u.ucValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        mask = scratch->u.uwValue;
        if (perso1 != NULL) {
            *p_lValue = fn_ucAtLeastOnePersoInZde(perso1, idx, mask);
        } else {
            *p_lValue = 0;
        }
        break;
    }

    default:
        *p_lValue = 0;
        break;
    }

    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch;
    return p_stTree;
}

void* fn_p_stReportOnSurfaceCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                      s32* p_lValue)
{
    s32* result;
    void* node;
    u16 op;
    HIE_tdstSuperObject_* spo;
    DNM_stReport_* obj;
    s32 flags;
    u32 v;

    result = p_lValue;
    node = p_stTree;
    op = AI_NODE_ID(node);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &spo);
    obj = fn_pstGetDNMReport(spo);
    if (obj != 0) {
        flags = obj->m_ulCurrSurfaceState;
        switch (op) {
        case eCond_CollideWithGround:
            *result = flags & 1;
            goto done;
        case eCond_CollideWithWall:
            v = flags >> 2;
            break;
        case eCond_CollideWithNothing:
            *result = flags == 0;
            goto done;
        case eCond_CollideWithCeiling:
            v = flags >> 4;
            break;
        case eCond_CollideWithPerso:
            v = flags >> 16;
            break;
        case eCond_CollideWithWater:
            v = flags >> 6;
            break;
        default:
            *result = 0;
            goto done;
        }
        *result = v & 1;
    } else {
        *result = 0;
    }
done:
    return node;
}

tdstNodeInterpret_* fn_p_stReportThisPerso(HIE_tdstSuperObject_* p_SuperObjPerso,
                                           tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    HIE_tdstSuperObject_* object;
    tdstGetSetParam_* saved;

    void* reference;
    DNM_stReport_* report;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &object);
    saved = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = saved + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, saved);
    reference = ((tdstEngineObject_*) saved->u.hSuperObject->hLinkedObject)->h_MSWay;
    *p_lValue = 0;
    report = fn_pstGetDNMReport(object);
    if (report != 0) {
        if (report->m_ulCurrSurfaceState & 0x10000) {
            if (report->m_stCharacter.mp_stSupObj != 0) {
                *p_lValue = MEC_fn_hGetFatherActor(report->m_stCharacter.mp_stSupObj) == reference;
            }
        }
    }
    inv_AI_parameterPoolCursor = saved;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stReportOnZDMSurfaceCondition(u32 flags, tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    s32 sel;
    DNM_stReport_* z;
    u16* rec;
    s32 v;

    sel = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso((HIE_tdstSuperObject_*) flags, &hNewSuperObjPerso);
    z = fn_pstGetDNMReport(hNewSuperObjPerso);
    if (z == 0) {
        *p_lValue = 0;
        return p_stTree;
    }
    {
        HIE_tdstSuperObject_* p_SuperObjPerso = (HIE_tdstSuperObject_*) flags;
        flags = z->m_ulCurrSurfaceState;
        rec = (u16*) inv_AI_parameterPoolCursor;
        inv_AI_parameterPoolCursor += 1;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, (tdstGetSetParam_*) rec);
        v = rec[0];
        switch (sel) {
        case eCond_ZDMCollideWithGround:
            if (flags & 1) {
                *p_lValue = fn_bTestTypeOfGameMaterial(z->m_stGround.hMyMaterial, v);
            } else {
                *p_lValue = 0;
            }
            break;
        case eCond_ZDMCollideWithWall:
            if (flags & 4) {
                *p_lValue = fn_bTestTypeOfGameMaterial(z->m_stWall.hMyMaterial, v);
            } else {
                *p_lValue = 0;
            }
            break;
        case eCond_ZDMCollideWithNothing:
            if (flags == 0) {
                *p_lValue = 1;
            } else if (flags & 1) {
                *p_lValue = fn_bTestTypeOfGameMaterial(z->m_stGround.hMyMaterial, v) == 0;
            } else if (flags & 4) {
                *p_lValue = fn_bTestTypeOfGameMaterial(z->m_stWall.hMyMaterial, v) == 0;
            } else if (flags & 0x10) {
                *p_lValue = fn_bTestTypeOfGameMaterial(z->m_stCeil.hMyMaterial, v) == 0;
            } else {
                *p_lValue = 1;
            }
            break;
        case eCond_ZDMCollideWithCeiling:
            if (flags & 0x10) {
                *p_lValue = fn_bTestTypeOfGameMaterial(z->m_stCeil.hMyMaterial, v);
            } else {
                *p_lValue = 0;
            }
            break;
        default:
            *p_lValue = 0;
            break;
        }
    }
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) rec;
    return p_stTree;
}

void* fn_p_stListCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                           s32* p_lValue)
{
    tdstGetSetParam_* left;
    tdstGetSetParam_* right;
    s32 op;

    left = (tdstGetSetParam_*) inv_AI_parameterPoolCursor;
    right = left + 1;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) ((s32) (left + 2));

    switch (op) {
    case eCond_IsPersoInList:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, left);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, right);
        *p_lValue =
            fn_bFindPersoInList(left->u.pvValue, inv_fn_hGetOwnerSuperObject(right->u.pvValue));
        break;

    case eCond_IsModelInList:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, left);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, right);
        *p_lValue = fn_bFindModelInList(left->u.pvValue, right->u.pvValue);
        break;

    case eCond_IsFamilyInList:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, left);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, right);
        *p_lValue = fn_bFindFamilyInList(left->u.pvValue, right->u.pvValue);
        break;

    case eCond_ListEmptyTest:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, left);
        *p_lValue = *(u8*) left->u.pvValue == 0;
        break;

    default:
        *p_lValue = 0;
        break;
    }

    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) left;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stTimeCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                         tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    tdstGetSetParam_* stParam;
    u32 ulCurrentTime;
    u32 ulSavedTime;
    u32 ulLimitDeltaTime;
    u32 ulDeltaTime;

    ulCurrentTime = (u32) inv_fn_ulGetEngineClock();
    stParam = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = stParam + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, stParam);
    ulSavedTime = (u32) stParam->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, stParam);
    ulLimitDeltaTime = (u32) stParam->u.lValue;
    ulDeltaTime = (ulCurrentTime >= ulSavedTime) ? (ulCurrentTime - ulSavedTime)
                                                 : (ulSavedTime - ulCurrentTime);
    *p_lValue = ulDeltaTime >= ulLimitDeltaTime;
    inv_AI_parameterPoolCursor = stParam;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stLipsSynchroCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    HIE_tdstSuperObject_* obj;
    tdstGetSetParam_* s;
    u16 type;
    gu_tdst3dData_* data3d;
    tdstAChannel_* channel;
    s32 idx;

    type = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &obj);
    s = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = s + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, s);

    if (type == eCond_LSY_IsSpeechOver) {
        idx = s->u.lValue;
        data3d = ((tdstEngineObject_*) obj->hLinkedObject)->h_3dData;
        if (idx >= 0 && (u32) idx < (u32) data3d->ulNumberOfChannels &&
            data3d->hArrayOfChannels != 0) {
            channel = &data3d->hArrayOfChannels[idx];
            *p_lValue =
                channel != 0 && channel->bActiveChannel != 0 && channel->hSupObject != 0;
        } else {
            *p_lValue = 0;
        }
    } else {
        *p_lValue = 0;
    }

    inv_AI_parameterPoolCursor = s;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stValidityCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    tdstGetSetParam_* scratch;
    u16 op;
    tdstNodeInterpret_* p_stNext;
    tdstEngineObject_* object;
    gu_tdstStandardGame* standardGame;
    s32 lValidity;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    switch (op) {
    case eCond_IsValidText:
        lValidity = ~scratch->u.lValue;
        goto cmp;
    case eCond_IsValidObject:
        object = (tdstEngineObject_*) scratch->u.lValue;
        lValidity = 0;
        if (object != NULL) {
            standardGame = object->h_StandardGame;
            if (standardGame != NULL) {
                lValidity = (standardGame->ucMiscFlags & 1) ^ 1;
            }
        }
        *p_lValue = lValidity;
        goto done;
    case eCond_IsValidWayPoint:
        lValidity = scratch->u.lValue;
        goto cmp;
    case eCond_IsValidGMT:
        lValidity = scratch->u.lValue;
        goto cmp;
    case eCond_IsValidVMT:
        lValidity = scratch->u.lValue;
        goto cmp;
    case eCond_IsValidAction:
        lValidity = scratch->u.lValue;
    cmp:
        *p_lValue = lValidity != 0;
        goto done;
    }
    *p_lValue = 0;
done:
    inv_AI_parameterPoolCursor = scratch;
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stMiscCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                         tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    tdstGetSetParam_* p_stParam;
    u16 op;
    HIE_tdstSuperObject_* hPerso;

    p_stParam = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = p_stParam + 1;

    switch (op) {
    case eCond_EngineReinitRequested:
        *p_lValue = g_stEngineStructure.eEngineMode != 9;
        break;

    case eCond_CmtIdentifierContainsMask: {
        u32 uwMask;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        uwMask = p_stParam->u.uwValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        uwMask &= p_stParam->u.uwValue;
        *p_lValue = uwMask != 0;
        break;
    }

    case eCond_HitByCollider: {
        CollisionSet* p_stCollSet = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_CollSet;
        s32 bHit = 0;
        if (p_stCollSet != 0) {
            bHit = p_stCollSet->stColliderInfo.ucColliderPriority != 0;
        }
        *p_lValue = bHit;
        if (bHit != 0) {
            p_stCollSet->stColliderInfo.ucColliderPriority = 0;
        }
        break;
    }

    case eCond_IsTypeOfGMTCollide: {
        void* p_stReport;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        p_stReport = fn_pstGetDNMReport(p_SuperObjPerso);
        if (p_stReport != 0 && ((DNM_stReport_*) p_stReport)->m_ulCurrSurfaceState != 0) {
            *p_lValue = fn_bIsTypeOfGMT(p_SuperObjPerso, p_stParam->u.uwValue);
        } else {
            *p_lValue = 0;
        }
        break;
    }

    case eCond_IsInComport:
    case eCond_IsInReflexComport: {
        HIE_tdstSuperObject_* hNewSuperObjPerso;
        tdstIntelligence_* p_stIntell;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        hNewSuperObjPerso = inv_fn_hGetOwnerSuperObject(((tdstEngineObject_*) p_stParam->u.pvValue));
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        if (op == eCond_IsInComport) {
            p_stIntell = inv_HIE_fn_p_stGetEngineObject(hNewSuperObjPerso)->h_Brain->p_stMind->p_stIntelligence;
        } else {
            p_stIntell = inv_HIE_fn_p_stGetEngineObject(hNewSuperObjPerso)->h_Brain->p_stMind->p_stReflex;
        }
        if (p_stIntell != 0) {
            *p_lValue = (u32) p_stIntell->p_stCurrentComport == p_stParam->u.ulValue;
        } else {
            *p_lValue = 0;
        }
        break;
    }

    case eCond_ChangeActionEnable:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        *p_lValue = PLA_fn_bTestNewState(p_SuperObjPerso, p_stParam->u.tdxstHandleToState);
        break;

    case eCond_IsThereMechEvent: {
        u8 ucDsgVarIndex;
        u16 id;
        void* p_stMind;
        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree + 1, &hPerso);
        } else {
            hPerso = p_SuperObjPerso;
        }
        p_stMind = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) hPerso)->hLinkedObject)
                  ->h_Brain
                 ->p_stMind;
        ucDsgVarIndex = (u8) p_stTree->param;
        id = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_3dData->uwNbEngineFrameSinceLastMechEvent;
        p_stTree++;
        if (id == 0xFFFF) {
            *p_lValue = 0;
            break;
        }
        p_stParam->u.ulValue = id;
        p_stParam->ucType = E_vt_Integer;
        fn_ucSetDsgVar(ucDsgVarIndex, 0, p_stMind, p_stParam);
        *p_lValue = 1;
        break;
    }

    default:
        *p_lValue = 0;
        break;
    }

    inv_AI_parameterPoolCursor = p_stParam;
    return p_stTree;
}

s32 fn_bPerso1Point1IntersectPerso2Point2(HIE_tdstSuperObject_* hPerso1, MTH3D_tdstVector_* p_stVertex1,
                  HIE_tdstSuperObject_* hPerso2, MTH3D_tdstVector_* p_stVertex2)
{
    HIE_tdstSuperObject_* sector = GAM_fn_hGetCurrentSector(hPerso1);
    HIE_tdstSuperObject_* otherSector;
    SECT_tdstSector_* sectorData;
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    MTH3D_tdstVector_ segment;
    s32 count;

    if (sector == NULL)
        return 0;
    otherSector = GAM_fn_hGetCurrentSector(hPerso2);
    if (otherSector == NULL)
        return 0;
    inv_MTH3D_fn_vSubXYZ(&segment, p_stVertex2, p_stVertex1);
    if (HIE_bDetectIntersectSegmentWithSuperObject(p_stVertex1, &segment, sector, 0))
        return 1;
    if (sector != otherSector &&
        HIE_bDetectIntersectSegmentWithSuperObject(p_stVertex1, &segment, otherSector, 0))
        return 1;
    sectorData = sector->hLinkedObject;
    count = sectorData->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt;
    neighbor = sectorData->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    while (count > 0) {
        if (neighbor->hPointerOfSectorSO != otherSector &&
            HIE_bDetectIntersectSegmentWithSuperObject(p_stVertex1, &segment, neighbor->hPointerOfSectorSO, 0))
            return 1;
        count--;
        neighbor++;
    }
    return 0;
}

void* fn_p_stSectorCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                             s32* p_lValue)
{
    MTH3D_tdstVector_ center0;
    MTH3D_tdstVector_ center1;
    HIE_tdstSuperObject_* saved;
    tdstGetSetParam_* scratch;
    void* other;
    u16 op = AI_NODE_ID(p_stTree);

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    if (op == eCond_SeePerso) {
        void* base = saved;
        scratch = inv_AI_parameterPoolCursor;
        inv_AI_parameterPoolCursor = scratch + 1;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        other = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        fn_vGetCenterOfBoundingVolume(base, &center0);
        fn_vGetCenterOfBoundingVolume(other, &center1);
        *p_lValue = !fn_bPerso1Point1IntersectPerso2Point2(base, &center0, other, &center1);
        inv_AI_parameterPoolCursor = scratch;
    } else {
        *p_lValue = 0;
    }
    return p_stTree;
}

void* fn_p_stLightCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                            s32* p_lValue)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    struct tdstMSLight_* h_MSLight;
    s32 lValue = 0;
    u16 op = AI_NODE_ID(p_stTree);

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_SuperObjPerso = 0;
    if (hNewSuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        p_SuperObjPerso = hNewSuperObjPerso->hLinkedObject;
    }
    h_MSLight = ((tdstEngineObject_*) p_SuperObjPerso)->h_MSLight;
    if (h_MSLight != 0) {
        switch (op) {
        case eCond_IsPersoLightOn:
#ifdef PC_PORT
            lValue = fn_ucMSLightGetOnOff(p_SuperObjPerso);
#else
            lValue = fn_ucMSLightGetOnOff(h_MSLight);
#endif
            break;
        case eCond_IsPersoLightPulseOn:
#ifdef PC_PORT
            lValue = fn_bMSLightGetPulse(p_SuperObjPerso);
#else
            lValue = fn_bMSLightGetPulse(h_MSLight);
#endif
            break;
        case eCond_IsPersoLightGyroPhareOn:
#ifdef PC_PORT
            lValue = fn_bMSLightGetGiroPhare(p_SuperObjPerso);
#else
            lValue = fn_bMSLightGetGiroPhare(h_MSLight);
#endif
            break;
        default:
            break;
        }
    }
    *p_lValue = lValue;
    return p_stTree;
}

void* fn_p_st_ActivationCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstEngineObject_* p_stTree,
                                  s32* p_lValue)
{
    HIE_tdstSuperObject_* saved;
    void* p_stNext = p_stTree;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    p_SuperObjPerso = saved;
    p_stTree = 0;
    if (p_SuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv)
        p_stTree = p_SuperObjPerso->hLinkedObject;
    p_SuperObjPerso = p_stTree->h_StandardGame;
    *p_lValue = (((gu_tdstStandardGame*) p_SuperObjPerso)->ucMiscFlags >> 1) & 1;
    return p_stNext;
}

void* fn_p_st_HandledCondition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                               s32* p_lValue)
{
    tdstGetSetParam_* scratch = inv_AI_parameterPoolCursor;
    void* p_stNext;
    s32 value;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    value = ((tdstEngineObject_*) scratch->u.pvValue)->h_StandardGame->ulLastTrame;
    *p_lValue = (value == inv_fn_ulGetEngineTrameNumber());
    inv_AI_parameterPoolCursor = scratch;
    return p_stNext;
}

tdstNodeInterpret_* fn_p_st_UserEvent_IsSet(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree, s32* _p_lValue)
{
    tdstNodeInterpret_* p_stNext;
    u8 ucDsgVarIndex;
    AI_tdstMind_* lMind;
    u8 ucEventIndex;
    tdstGetSetParam_* p_stParam;
    s32 op;
    MTH3D_tdstVector_ stEventVector;
    HIE_tdstSuperObject_* hPerso;
    HIE_tdstSuperObject_* p_stPerso;
    s32* p_lResult;
    s32 lFlag;

    p_lResult = _p_lValue;
    p_stPerso = p_SuperObjPerso;
    p_stNext = p_stTree;
    op = (s32) AI_NODE_ID(p_stTree);

    switch (op) {
    case eCond_UserEvent_IsSet:
    case eCond_UserEvent_IsSet2: {
        p_stParam = (tdstGetSetParam_*) inv_AI_parameterPoolCursor;
        inv_AI_parameterPoolCursor = (tdstGetSetParam_*) p_stParam + 1;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
        ucEventIndex = *(u8*) p_stParam;
        if (op == eCond_UserEvent_IsSet2) {
            if (p_stNext->eType == E_ti_Operator) {
                p_stNext = fn_p_stOperatorGetPerso(p_stPerso, p_stNext + 1, &hPerso);
            } else {
                hPerso = p_stPerso;
            }
            ucDsgVarIndex = (u8) p_stNext->param;
            p_stNext++;
            lMind = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) hPerso)
                                            ->hLinkedObject)
                            ->h_Brain
                     ->p_stMind;
        }
        break;
    }
    default:
        goto fail;
    }

    if (ucEventIndex < 8) {
        lFlag = (s32) p_stPerso->hLinkedObject;
        lFlag = fn_b3dDataGetUserEventFlag(((tdstEngineObject_*) lFlag)->h_3dData, ucEventIndex, &stEventVector);
        *p_lResult = lFlag;
        if (lFlag != 0) {
            if (op == eCond_UserEvent_IsSet2) {
                p_stParam->u.stVector = stEventVector;
                p_stParam->ucType = E_vt_Vector;
                fn_ucSetDsgVar(ucDsgVarIndex, 0, lMind, p_stParam);
            }
        }
    } else {
        *p_lResult = 0;
    }
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) p_stParam;
    goto done;
fail:
    *p_lResult = 0;
done:
    return p_stNext;
}

void* fn_p_stAlw_IsMine(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                        s32* p_lValue)
{
    HIE_tdstSuperObject_* saved;
    tdstGetSetParam_* scratch;
    void* object;
    void* linkedObject;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    object = scratch->u.pvValue;
    *p_lValue = (object != 0 && (linkedObject = ((tdstEngineObject_*) object)->h_StandardGame) != 0)
                    ? fn_ucIsAlwaysGeneratedByMe(
                          ((gu_tdstStandardGame*) linkedObject)->h_SuperObject, saved)
                    : 0;
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stZDMCollideWithObstacle(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                    s32* p_lValue)
{
    tdstGetSetParam_* scratch = inv_AI_parameterPoolCursor;
    DNM_stReport_* obj;
    u16 op = AI_NODE_ID(stream);
    inv_AI_parameterPoolCursor = scratch + 1;
    stream = fn_p_stEvalTree(superObject, stream, scratch);
    obj = fn_pstGetDNMReport(superObject);
    if (obj == 0) {
        *p_lValue = 0;
    } else {
        {
            u32 flags = obj->m_ulCurrSurfaceState;
            u16 value = scratch->u.uwValue;
            *p_lValue = 0;
            switch (op) {
            case eCond_IsZDMCollideWithObstacle:
                if (flags != 0)
                    *p_lValue = fn_bTestTypeOfGameMaterial(obj->m_stObstacle.hCollidedMaterial, value);
                break;
            case eCond_IsZDMCollideWithWall:
                if (flags & 4)
                    *p_lValue = fn_bTestTypeOfGameMaterial(obj->m_stWall.hCollidedMaterial, value);
                break;
            case eCond_IsZDMCollideWithGround:
                if (flags & 1)
                    *p_lValue = fn_bTestTypeOfGameMaterial(obj->m_stGround.hCollidedMaterial, value);
                break;
            case eCond_IsZDMCollideWithCeiling:
                if (flags & 0x10)
                    *p_lValue = fn_bTestTypeOfGameMaterial(obj->m_stCeil.hCollidedMaterial, value);
                break;
            }
        }
    }
    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* fn_p_stCustomBitsCond(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                            s32* p_lValue)
{
    HIE_tdstSuperObject_* saved;
    tdstGetSetParam_* scratch;
    u16 op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &saved);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    stream = fn_p_stEvalTree(superObject, stream, scratch);
    switch (op) {
    case eCond_IsCustomBitSet: {
        s32 bit = (s16) (scratch->u.uwValue - 1);
        if (bit < 32) {
            u32 mask = 1U << bit;
            tdstEngineObject_* perso = saved->hLinkedObject;
            *p_lValue = (perso->h_StandardGame->ulCustomBits & mask) != 0;
        } else {
            u32 mask = 1U << bit;
            tdstEngineObject_* perso = saved->hLinkedObject;
            *p_lValue = (perso->h_StandardGame->ulAICustomBits & mask) != 0;
        }
        break;
    }
    case eCond_IsCurrentStateCustomBitSet: {
        u32 mask = scratch->u.ulValue;
        tdstEngineObject_* perso = saved->hLinkedObject;
        tdstState_* state = perso->h_3dData->h_CurrentState;
        *p_lValue = state != NULL && (state->ucCustomBits & mask) != 0;
        break;
    }
    default:
        *p_lValue = 0;
        break;
    }
    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* fn_p_stCollisionWP(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                         s32* p_lValue)
{
    tdstGetSetParam_* scratch;
    void* parsed;
    tdstEngineObject_* first;
    WP_tdstWayPoint_* node;
    void* result;
    HIE_tdstSuperObject_* position;
    POS_tdstMatrix_* vector;
    f32 radius;
    f32 length_squared;
    MTH3D_tdstVector_ values;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    parsed = fn_p_stEvalTree(superObject, stream, scratch);
    first = scratch->u.pvValue;
    position = first->h_StandardGame->h_SuperObject;
    result = fn_p_stEvalTree(superObject, parsed, scratch);
    node = scratch->u.pvValue;
    if (position == 0) {
        *p_lValue = 0;
    } else if (node == 0) {
        *p_lValue = 0;
    } else {
        if (node->m_hSuperObject != 0) {
            WP_fnv_WayPoint_ComputeLocation(node, &values);
            vector = position->gu_hGlobalMatrix;
            values.xX -= vector->stMatrix.stCol_3.xX;
            vector = position->gu_hGlobalMatrix;
            values.xY -= vector->stMatrix.stCol_3.xY;
            vector = position->gu_hGlobalMatrix;
            values.xZ -= vector->stMatrix.stCol_3.xZ;
        } else {
            values.xX = node->m_stVertex.xX - position->gu_hGlobalMatrix->stMatrix.stCol_3.xX;
            values.xY = node->m_stVertex.xY - position->gu_hGlobalMatrix->stMatrix.stCol_3.xY;
            values.xZ = node->m_stVertex.xZ - position->gu_hGlobalMatrix->stMatrix.stCol_3.xZ;
        }
        radius = node->m_xRadius;
        length_squared = MTH_fn_xSqrAddSqrAddSqrOpt2(values.xX, values.xY, values.xZ);
        if (0.0f < length_squared - MTH_fn_xSqrOpt1(radius)) {
            *p_lValue = 0;
        } else {
            *p_lValue = 1;
        }
    }
    inv_AI_parameterPoolCursor = scratch;
    return result;
}

void* fn_p_stNullVector(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                        s32* p_lValue)
{
    void* scratch;
    f32* vector;
    void* result;
    s32 value;
    f32 epsilon;

    scratch = (void*) inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch + 1;
    result = fn_p_stEvalTree(superObject, stream, scratch);
    vector = scratch;
    epsilon = 1e-05f;
    value = 0;
    if (inv_MTH_fn_xAbsoluteTied(vector[0]) < epsilon &&
        inv_MTH_fn_xAbsoluteTied(vector[1]) < epsilon &&
        inv_MTH_fn_xAbsoluteTied(vector[2]) < epsilon) {
        value = 1;
    }
    *p_lValue = value;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch;
    return result;
}

void* fn_p_stMechanicOption(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                            s32* p_lValue)
{
    typedef struct ConditionVectorWords {
        s32 x;
        s32 y;
        s32 z;
    } ConditionVectorWords;

    HIE_tdstSuperObject_* saved;
    HIE_tdstSuperObject_* perso;
    DNM_stDynamics* mechanic;
    s32 result;
    s32 value;
    u16 op;
    u8* scratch;
    u8* nextScratch;
    tdstEngineObject_* p;
    tdstBrain_* q;
    AI_tdstMind_* dsg;
    s32 nodeType;
    u8 id;

    op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &saved);
    mechanic =
        ((tdstEngineObject_*) saved->hLinkedObject)->h_Dynam->p_stDynamics;

    switch (op) {
    case eCond_IsMechanicAnimation:
        result = mechanic->m_tdstDynamicsBaseBlock.ulFlags & 1;
        break;
    case eCond_IsMechanicCollide:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 1;

        result &= 1;
        break;
    case eCond_IsMechanicGravity:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 2;

        result &= 1;
        break;
    case eCond_IsMechanicTilt:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 3;

        result &= 1;
        break;
    case eCond_IsMechanicGi:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 4;

        result &= 1;
        break;
    case eCond_IsMechanicClimb:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 6;

        result &= 1;
        break;
    case eCond_IsMechanicOnGround:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 5;

        result &= 1;
        break;
    case eCond_IsMechanicSpider:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 8;

        result &= 1;
        break;
    case eCond_IsMechanicShoot:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 9;

        result &= 1;
        break;
    case eCond_IsMechanicSwim:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 9;

        result &= 1;
        break;
    case eCond_IsMechanicNeverFall:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 21;

        result &= 1;
        break;
    case eCond_IsMechanicCollisionControl:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 7;

        result &= 1;
        break;
    case eCond_IsMechanicKeepSpeedZ:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 8;

        result &= 1;
        break;
    case eCond_IsMechanicSpeedLimit:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 9;

        result &= 1;
        break;
    case eCond_IsMechanicInertia:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 10;

        result &= 1;
        break;
    case eCond_IsMechanicStream:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 11;

        result &= 1;
        break;
    case eCond_IsMechanicStickOnPlatform:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 12;

        result &= 1;
        break;
    case eCond_IsMechanicPatformCrash:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 5;

        result &= 1;
        break;
    case eCond_CanFall:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 6;

        result &= 1;
        if (result != 0) {
            if (stream->eType == E_ti_Operator) {
                stream = fn_p_stOperatorGetPerso(superObject, stream + 1, &perso);
            } else {
                perso = superObject;
            }
            nodeType = E_vt_Vector;
            scratch = (u8*) inv_AI_parameterPoolCursor;
            nextScratch = scratch + 0x10;
            p = perso->hLinkedObject;
            id = (u8) stream->param;
            q = p->h_Brain;
            stream++;
            dsg = q->p_stMind;
            inv_AI_parameterPoolCursor = (tdstGetSetParam_*) nextScratch;
            *(ConditionVectorWords*) scratch = *(ConditionVectorWords*) &mechanic->m_tdstDynamicsComplexBlock.m_stFallTranslation;
            scratch[0xC] = nodeType;
            fn_ucSetDsgVar(id, 0, dsg, scratch);
            inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch;
        } else {
            stream = fn_p_stSkipThisArgument(stream);
        }
        break;
    case eCond_IsGiBlock:
    case eCond_IsMechanicBlock: {
        s32 flags = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;
        mechanic->m_tdstDynamicsBaseBlock.ulEndFlags = flags & 0xFFFEFFFF;
        result = flags >> 16;
        result &= 1;
        break;
    }
    case eCond_IsMechanicScale:
        value = mechanic->m_tdstDynamicsBaseBlock.ulFlags;

        result = value >> 13;

        result &= 1;
        break;
    case eCond_IsMechanicExec: {
        s32 flags = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;
        mechanic->m_tdstDynamicsBaseBlock.ulEndFlags = flags & 0xFFFFDFFF;
        result = flags >> 13;
        result &= 1;
        break;
    }
    case eCond_IsMechanicCrash:
        value = mechanic->m_tdstDynamicsBaseBlock.ulEndFlags;

        result = value >> 19;

        result &= 1;
        break;
    default:
        result = 0;
        break;
    }

    *p_lValue = result != 0;
    return stream;
}

void* fn_p_stCheckAnimEnd(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                          s32* p_lValue)
{
    HIE_tdstSuperObject_* saved;
    u16 op = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &saved);
    switch (op) {
    case eCond_CheckAnimEnd:
        *p_lValue = PLA_fn_bTestIfEndOfAnim(saved);
        break;
    case eCond_CheckActionEnd:
        *p_lValue = PLA_fn_bTestIfEndOfAction(saved);
        break;
    case eCond_IsInAction: {
        tdstGetSetParam_* scratch = inv_AI_parameterPoolCursor;
        inv_AI_parameterPoolCursor = scratch + 1;
        stream = fn_p_stEvalTree(superObject, stream, scratch);
        *p_lValue = PLA_fn_bTestCurrentAction(saved, scratch->u.tdxstHandleToState);
        inv_AI_parameterPoolCursor = scratch;
        break;
    }
    default:
        *p_lValue = 0;
        break;
    }
    return stream;
}

void* fn_p_stHierarchySon(HIE_tdstSuperObject_* inputContext, tdstNodeInterpret_* stream,
                          s32* p_lValue)
{
    HIE_tdstSuperObject_* hPerso;
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* target;
    tdstDynam_* hier;
    DNM_stDynamics* data;
    HIE_tdstSuperObject_* cur;
    void* context = inputContext;

    fn_vGetUltraOperatorPerso(context, &hPerso);
    *p_lValue = 0;
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    stream = fn_p_stEvalTree(context, stream, scratch);

    target = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
    hier = ((tdstEngineObject_*) hPerso->hLinkedObject)->h_Dynam;
    inv_AI_parameterPoolCursor = scratch;

    if (hier != 0) {
        data = hier->p_stDynamics;
        if ((data->m_tdstDynamicsBaseBlock.ulEndFlags & 4) != 0) {
            if (data->m_tdstDynamicsComplexBlock.m_hPlatform == target)
                *p_lValue = 1;
            return stream;
        }
    }

    cur = target->hFirstElementSta;
    if (cur != 0) {
        do {
            if (cur == hPerso) {
                *p_lValue = 1;
                return stream;
            }
            cur = cur->hNextBrotherSta;
        } while (cur != 0);
    }

    return stream;
}

tdstNodeInterpret_* fn_p_stCheckCapabilities(HIE_tdstSuperObject_* superObject,
                                             tdstNodeInterpret_* stream, s32* p_lValue)
{
    HIE_tdstSuperObject_* hPerso;
    tdstGetSetParam_* scratch;
    void* p_stNext;
    s32 capabilities = 0;
    s32 mask = 0;
    u16 op = AI_NODE_ID(stream);

    fn_vGetUltraOperatorPerso(superObject, &hPerso);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    *p_lValue = 0;

    switch (op) {
    case eCond_HasTheCapability:
    case eCond_HasOneOfTheCapabilities:
        p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
        capabilities = scratch->u.lValue;
        stream = p_stNext;
        p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
        stream = p_stNext;
        mask = scratch->u.lValue;
        break;

    case eCond_HasTheCapabilityNumber:
        p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
        capabilities = scratch->u.lValue;
        stream = p_stNext;
        p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
        stream = p_stNext;
        mask = 1 << scratch->u.ucValue;
        break;

    case eCond_PersoHasTheCapability:
    case eCond_PersoHasOneOfTheCapabilities: {
        void* obj = 0;
        if (hPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv)
            obj = hPerso->hLinkedObject;
        capabilities = fn_ulGetCapabilities(obj);
    }
        p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
        stream = p_stNext;
        mask = scratch->u.lValue;
        break;

    case eCond_PersoHasTheCapabilityNumber: {
        void* obj = 0;
        if (hPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv)
            obj = hPerso->hLinkedObject;
        capabilities = fn_ulGetCapabilities(obj);
    }
        p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
        stream = p_stNext;
        mask = 1 << scratch->u.ucValue;
        break;
    }

    if (op == eCond_HasOneOfTheCapabilities || op == eCond_PersoHasOneOfTheCapabilities)
        *p_lValue = (capabilities & mask) != 0;
    else
        *p_lValue = (capabilities & mask) == mask;

    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* fn_p_stIsPersoActive(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                           s32* p_lValue)
{
    tdstGetSetParam_* scratch = inv_AI_parameterPoolCursor;
    void* p_stNext;
    gu_tdstStandardGame* p;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stNext = fn_p_stEvalTree(superObject, stream, scratch);
    p = ((tdstEngineObject_*) scratch->u.pvValue)->h_StandardGame;
    if (p != 0)
        *p_lValue = inv_fn_bIsSuperObjectInDynamicWorld(p->h_SuperObject);
    else
        *p_lValue = 0;
    inv_AI_parameterPoolCursor = scratch;
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stIsCollisionFlag(HIE_tdstSuperObject_* p_SuperObjPerso,
                                           tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    HIE_tdstSuperObject_* perso;
    u16 op;
    CollisionSet* collisionSet;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    collisionSet = ((tdstEngineObject_*) perso->hLinkedObject)->h_CollSet;
    switch (op) {
    case eCond_NEstPasEnCollisionAvecMap:
        *p_lValue = collisionSet->ucCollisionFlag & 1;
        break;
    case eCond_NEstPasEnCollisionAvecProjectile:
        *p_lValue = collisionSet->ucCollisionFlag & 2;
        break;
    case eCond_NEstPasEnCollisionAvecSecondCharact:
        *p_lValue = collisionSet->ucCollisionFlag & 4;
        break;
    case eCond_NEstPasEnCollisionAvecMainCharact:
        *p_lValue = collisionSet->ucCollisionFlag & 8;
        break;
    case eCond_NEstPasEnCollisionAvecAutresSecteurs:
        *p_lValue = collisionSet->ucCollisionFlag & 0x20;
        break;
    }
    return p_stTree;
}

void* fn_p_stIsOf(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream, s32* p_lValue)
{
    HIE_tdstSuperObject_* hPerso;
    tdstGetSetParam_* scratch;
    u16 type;
    tdstEngineObject_* p_stEngineObject;
    tdstBrain_* p_stBrain;
    u32 ulValue;
    u32 ulModel;
    u32 bResult;

    type = AI_NODE_ID(stream);
    fn_vGetUltraOperatorPerso(superObject, &hPerso);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    stream = fn_p_stEvalTree(superObject, stream, scratch);

    switch (type) {
    case eCond_IsInFamily:
        *p_lValue =
            ((u32) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) hPerso)->hLinkedObject)->h_3dData->h_Family ^
             scratch->u.ulValue) == 0;
        break;
    case eCond_IsInModel:
        p_stEngineObject = ((HIE_tdstSuperObject_*) hPerso)->hLinkedObject;
        p_stBrain = p_stEngineObject->h_Brain;
        bResult = 0;
        if (p_stBrain != 0) {
            ulModel = (u32) p_stBrain->p_stMind->p_stAIModel;
            ulValue = scratch->u.ulValue;
            ulModel = ulModel ^ ulValue;
            bResult = ulModel == 0;
        }
        *p_lValue = bResult;
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* fn_p_stAJoypadIsConnected(void* context, void* stream)
{
    (void) context;
    return stream;
}

void* fn_p_stAKeyJustPressed(tdstNodeInterpret_* context, void* stream)
{
    (void) context;
    return stream;
}

void* fn_p_stAButtonPadJustPressed(void* superObject, void* stream, s32* out)
{
    *out = IPT_fn_bAButtonPadJustPressed();
    return stream;
}

tdstNodeInterpret_* fn_p_stDemoCondition(HIE_tdstSuperObject_* superObject,
                                         tdstNodeInterpret_* stream, s32* outParam)
{
    *outParam = g_stEngineStructure.xDemoMode;
    return stream;
}

tdstNodeInterpret_* fn_p_stIsInStereoMode(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    *p_lValue = SND_fn_bIsMusicPlayingSound(p_SuperObjPerso);
    return p_stTree;
}

void* func_00209570(void* context, void* stream, s32* out)
{
    *out = SND_fn_bGetStereoSound(context);
    return stream;
}

void* func_002095A8(void* superObject, void* stream, s32* out)
{
#ifdef PC_PORT
    tdstGetSetParam_ stParam;
    void* p_stNext = fn_p_stEvalTree(superObject, stream, &stParam);
    s32* p = stParam.u.pvValue;
#else
    s32* p;
    void* p_stNext = fn_p_stEvalTree(superObject, stream, &p);
#endif
    *out = p != 0 && *p != 0;
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stTooFarCondition(HIE_tdstSuperObject_* p_SuperObjPerso,
                                           tdstNodeInterpret_* stream, s32* p_lValue)
{
    HIE_tdstSuperObject_* perso;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    if (perso == g_stEngineStructure.activeSuperObject) {
        *p_lValue = 0;
    } else {
        gu_tdstStandardGame* standardGame;
        standardGame = ((tdstEngineObject_*) perso->hLinkedObject)->h_StandardGame;
        if (standardGame != 0) {
            if (standardGame->ulCustomBits & 0x4C0000)
                *p_lValue = standardGame->ucMiscFlags >> 7;
            else
                *p_lValue = 0;
        }
    }
    return stream;
}

void* func_00209668(void* context, void* stream, s32* outParam)
{
    u16 op = AI_NODE_ID(stream);

    *outParam = 0;
    switch (op) {
    case 0x7B:
        *outParam = inv_SAV_fn_bIsSaveDeviceReady();
#ifdef PC_PORT
        if (*outParam != 0 && pc_mod_autosave_prompt((PcAddress) ((u8*) stream - 8))) {
            tdstNodeInterpret_* clockNode =
                (tdstNodeInterpret_*) pc_mod_autosave_clock((PcAddress) ((u8*) stream - 8));
            s32 saved;

            if (clockNode != NULL) {
                HIE_tdstSuperObject_* clock;
                tdstGetSetParam_ played;

                fn_p_stOperatorGetPerso(context, clockNode, &clock);
                fn_vOperatorGetDsgVar(30, 0, clock, &played);
                if (played.ucType == E_vt_Float) {
                    played.u.xValue = played.u.xValue / 1000.0f;
                } else {
                    played.u.lValue = played.u.lValue / 1000;
                }
                inv_SAV_fn_vSetCurrentGameDuration(played.u.ulValue);
            }
            saved = inv_SAV_fn_bProcessCurrentSaveRequest();

            pc_mod_autosave_done(saved);
            if (saved != 0) {
                *outParam = 0;
            }
        }
#endif
        break;
    case 0x7C:
        *outParam = inv_SAV_fn_bIsSaveDeviceFormatted();
        break;
    case 0x7D:
        *outParam = inv_SAV_fn_bCanSaveCurrentGame();
        break;
    default:
        return stream;
    }
    return stream;
}

s32 func_00209708(s32 superObject, s32 stream, s32* out)
{
    tdstGetSetParam_ param;
    s32 lPad;
    s32 lPadCount;
    s32 lPadIndex;
    s32 p_stNext;

    p_stNext = (s32) fn_p_stEvalTree(superObject, stream, &param);
    lPad = param.u.lValue;
    lPadIndex = lPad - 1;
    lPadCount = inv_IPT_fn_lGetPadWithButton8Pressed();
    if (lPadCount >= lPadIndex) {
        inv_IPT_fn_vIncrementInputFrameCounter();
        inv_IPT_fn_bSwapJoystickDevices(lPadIndex, lPadCount);
        *out = 1;
    } else {
        *out = 0;
    }
    return p_stNext;
}

tdstNodeInterpret_* func_00209788(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream, s32* out)
{
    tdstGetSetParam_ stPerso;
    tdstNodeInterpret_* p_stNode;
    tdstNodeInterpret_* p_stParent;

    p_stNode = stream;
    fn_vGetUltraOperatorPerso(superObject, &stPerso.u.hSuperObject);
    p_stParent = stream - 1;
    if (p_stNode->ucDepth > p_stParent->ucDepth) {
        do {
            p_stNode = fn_p_stSkipThisArgument(p_stNode);
        } while (p_stParent->ucDepth < p_stNode->ucDepth);
    }
    *out = 1;
    return p_stNode;
}
