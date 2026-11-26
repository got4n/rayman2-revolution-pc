#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"

#include "MTH/MTH.h"
#include "ps2/libc.h"
#include "clib/clib.h"

typedef struct WhileState {
    tdstNodeInterpret_* body;
    tdstNodeInterpret_* header;
    tdstNodeInterpret_* condition;
} WhileState;

extern s32 iNumWhile;
extern WhileState a_stWhileInfo[];

s32 strcasecmp(const char* first, const char* second);

tdstNodeInterpret_* fn_p_stGetFrameLength(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree,
                                                   tdstGetSetParam_* p_stValue)
{
    p_stValue->u.xValue = inv_fn_xGetEngineClockScale();
    p_stValue->ucType = E_vt_Float;
    return p_stTree;
}

void inv_fn_vClearDsgMemRecursive(HIE_tdstSuperObject_* root)
{
    tdstEngineObject_* object;
    tdstBrain_* brain;
    HIE_tdstSuperObject_* child;
    HIE_tdstSuperObject_* next;

    if (root->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        object = root->hLinkedObject;
        brain = object->h_Brain;
        if (brain != NULL) {
            fn_vReinitAIWithFlags(brain->p_stMind, OTI_WhenGameStart);
        }
    } else {
        child = root->hFirstElementSta;
        next = HIE_fn_hForceGetSuperObjectNextBrother(child);
        while (child != NULL) {
            inv_fn_vClearDsgMemRecursive(child);
            child = next;
            next = HIE_fn_hForceGetSuperObjectNextBrother(child);
        }
    }
}

tdstNodeInterpret_* inv_fn_p_stGetCurrentWorldLevelIndex(HIE_tdstSuperObject_* superObject,
                                                         tdstNodeInterpret_* stream,
                                                         tdstGetSetParam_* out)
{
    char name[0x40];

    if (AI_NODE_ID(stream) == 0x67) {
        strcpy(name, fn_p_szGetLevelName());
        if (strcasecmp(inv_fn_p_szGetLevelNumber(), "Default") != 0) {
            Strcat(name, "$");
            Strcat(name, inv_fn_p_szGetLevelNumber());
        }
        if (strcasecmp(fn_p_szGetFirstLevelName(), "Default") != 0) {
            Strcat(name, "#");
            Strcat(name, fn_p_szGetFirstLevelName());
        }
        out->u.lValue = inv_fn_ucFindLevelNameIndex(name);
        out->ucType = E_vt_Integer;
    }
    return stream;
}

tdstNodeInterpret_* fn_p_stNoActionKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_Action;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stMeKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    void* object = NULL;

    if (p_SuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        object = p_SuperObjPerso->hLinkedObject;
    }
    p_stValue->u.pvValue = object;
    p_stValue->ucType = E_vt_Perso;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stMainActorKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    s32 engineObjectType = 2;
    HIE_tdstSuperObject_* main = g_stEngineStructure.activeSuperObject;
    void* object = NULL;

    if (main->lTypeOfLinkedObject == engineObjectType) {
        object = main->hLinkedObject;
    }
    p_stValue->u.pvValue = object;
    p_stValue->ucType = E_vt_Perso;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNoSuperObjectKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_SuperObject;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNobodyKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_Perso;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNowhereKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_WayPoint;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stEmptyTextKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = -1;
    p_stValue->ucType = E_vt_Text;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stCapsNullKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_Caps;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNoGraphKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_Graph;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNoGMTKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_GameMaterial;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNoVMTKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_VisualMaterial;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stNoSoundEventKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_SoundEvent;
    return p_stTree;
}

tdstNodeInterpret_* func_001F1E58(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                  tdstGetSetParam_* out)
{
    out->u.lValue = 0;
    out->ucType = E_vt_Module;
    return stream;
}

tdstNodeInterpret_* fn_p_stNoInputKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_Button;
    return p_stTree;
}

#ifdef PC_PORT
tdstNodeInterpret_* fn_p_stThenKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    return fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, p_stTree, p_stValue);
}
#else
tdstNodeInterpret_* fn_p_stThenKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    return fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, p_stTree, p_stValue);
}
#endif

#ifdef PC_PORT
tdstNodeInterpret_* fn_p_stElseKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                       void* unused)
#else
tdstNodeInterpret_* fn_p_stElseKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
#endif
{
    tdstNodeInterpret_* header = p_stTree - 1;

    return header + header->uwNodeToSkip;
}

tdstNodeInterpret_* fn_p_stIfKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    tdstNodeInterpret_* header = p_stTree - 1;
    u32 negate = 0;
    u32 skipCondition;

    switch ((u16) header->param) {
    case 0:
        negate = 1;
        skipCondition = 0;
        break;
    case 2:
        negate = 1;
    case 3:
        skipCondition = gcGlobAleat & 1;
        break;
    case 4:
        negate = 1;
    case 5:
        skipCondition = gcGlobAleat & 3;
        break;
    case 6:
        negate = 1;
    case 7:
        skipCondition = gcGlobAleat & 7;
        break;
    case 8:
        negate = 1;
    case 9:
        skipCondition = gcGlobAleat & 15;
        break;
    default:
        skipCondition = 0;
        break;
    }
    if (skipCondition) {
        p_stValue->u.lValue = 0;
        p_stTree += p_stTree->uwNodeToSkip;
    } else {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (negate) {
            p_stValue->u.lValue = !p_stValue->u.lValue;
        }
    }
    if (!p_stValue->u.lValue) {
        p_stTree = header + header->uwNodeToSkip;
    } else {
        p_stTree++;
    }
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stWhileKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* condition = inv_AI_parameterPoolCursor;
    WhileState* state = &a_stWhileInfo[iNumWhile];
    tdstNodeInterpret_* header = p_stTree - 1;
    s32 result;

    inv_AI_parameterPoolCursor = condition + 1;
    state->condition = p_stTree;
    state->header = header;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, condition);
    result = condition->u.lValue;
    inv_AI_parameterPoolCursor = condition;
    if (result == 0) {
        return header + header->uwNodeToSkip;
    }
    state->body = p_stTree;
    iNumWhile++;
    return p_stTree;
}

#ifdef PC_PORT
tdstNodeInterpret_* fn_p_stBeginWhileKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                  tdstGetSetParam_* p_stValue)
{
    return fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, p_stTree, p_stValue);
}
#else
tdstNodeInterpret_* fn_p_stBeginWhileKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                       tdstGetSetParam_* p_stValue)
{
    return fn_p_stIntelligenceEvalTreeEngine(p_SuperObjPerso, p_stTree, p_stValue);
}
#endif

tdstNodeInterpret_* fn_p_stEndWhileKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso,
                                               tdstNodeInterpret_* p_stTree)
{
    WhileState* state;
    tdstGetSetParam_* condition;
    s32 result;
    s32 depth = iNumWhile;

    if (depth == 0) {
        return p_stTree;
    }
    condition = inv_AI_parameterPoolCursor;
    state = &a_stWhileInfo[depth - 1];
    inv_AI_parameterPoolCursor = condition + 1;
    fn_p_stEvalTree(p_SuperObjPerso, state->condition, condition);
    result = condition->u.lValue;
    inv_AI_parameterPoolCursor = condition;
    if (result != 0) {
        return state->body;
    }
    iNumWhile--;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stEngineGotoKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    tdstNodeInterpret_* next = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    s32 count;

    if (p_stValue->ucType == E_vt_Float) {
        count = inv_MTH_fn_lToIntegerOrdered(p_stValue->u.xValue);
    } else {
        count = p_stValue->u.lValue;
    }
    p_stValue->u.lValue = 0;
    p_stValue->ucType = E_vt_MetaActionReturn;
    return next + count;
}
